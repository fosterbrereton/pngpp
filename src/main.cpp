/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

// stdc++
#include <iostream>
#include <numeric>
#include <random>

// tbb
#include <tbb/parallel_for_each.h>

// boost
#include <boost/program_options.hpp>

// application
#include <pngpp/files.hpp>
#include <pngpp/png.hpp>

/**************************************************************************************************/

using namespace pngpp;

/**************************************************************************************************/

path_t associated_filename(path_t src, const std::string& new_leaf) {
    // takes ("/path/to.png", "extra") and returns "/path/to_extra.png"
    std::string stem(src.stem().string());
    std::string extension(src.extension().string());

    src.remove_filename();

    std::string new_filename(stem + "_" + new_leaf + extension);

    return src / new_filename;
}

/**************************************************************************************************/

path_t derived_filename(path_t src, const std::string& new_leaf) {
    // takes ("/path/to.png", "extra") and returns "/path/extra_to.png"
    std::string leaf(src.leaf().string());

    src.remove_filename();

    std::string new_filename(new_leaf + "_" + leaf);

    return src / new_filename;
}

/**************************************************************************************************/

void dump_color_table(const image_t& image, const path_t& output) {
    color_table_t color_table(image.color_table());

    if (color_table.empty())
        return;

    color_table.resize(PNG_MAX_PALETTE_LENGTH);

    constexpr std::size_t swatch_size_k(32);
    const std::size_t     width(16 * swatch_size_k);
    const std::size_t     height(16 * swatch_size_k);
    const std::size_t     depth(8);
    const std::size_t     rowbytes(width);
    image_t               table(width, height, depth, rowbytes, PNG_COLOR_TYPE_PALETTE);
    auto                  first(table.data());

    for (std::size_t y(0); y < 16; ++y) {
        for (std::size_t i(0); i < swatch_size_k; ++i) {
            for (std::size_t x(0); x < 16; ++x) {
                for (std::size_t j(0); j < swatch_size_k; ++j) {
                    *first++ = 16 * y + x;
                }
            }
        }
    }

    table.set_color_table(color_table);

    save_png(table, associated_filename(output, "table"), save_options_t());
}

/**************************************************************************************************/

std::size_t verbose_save(const image_t& image,
                         const path_t&  path,
                         save_mode      mode = save_mode::max) {
    dump_color_table(image, path);

    save_options_t options;

    options._mode = mode;

    std::size_t size(save_png(image, path, options));

    std::cout << path.leaf().string() << ": " << size << '\n';

    return size;
}

/**************************************************************************************************/

typedef std::vector<std::size_t> indexed_histogram_t;

indexed_histogram_t indexed_histogram(const image_t& image) {
    indexed_histogram_t result(PNG_MAX_PALETTE_LENGTH, 0);

    for (auto entry : image)
        ++result[entry];

    return result;
}

/**************************************************************************************************/

typedef std::pair<std::size_t, std::uint8_t> indexed_histogram_pair_t;
typedef std::vector<indexed_histogram_pair_t> indexed_histogram_table_t;

indexed_histogram_table_t make_indexed_histogram_table(const image_t& image) {
    std::size_t               count(image.color_table().size());
    indexed_histogram_table_t result(count);
    indexed_histogram_t       image_hist(indexed_histogram(image));

    for (std::size_t i(0); i < count; ++i) {
        result[i] = indexed_histogram_pair_t(image_hist[i], i);
    }

    return result;
}

/**************************************************************************************************/

typedef std::vector<std::uint8_t> index_map_t;

image_t reindex_image(const image_t& image, const index_map_t& map) {
    image_t result(image);

    for (auto& entry : result)
        entry = map[entry];

    const auto&   src_table(image.color_table());
    color_table_t dst_table(src_table);
    std::size_t   count(image.color_table().size());

    for (std::size_t i(0); i < count; ++i)
        dst_table[map[i]] = src_table[i];

    result.set_color_table(dst_table);

    return result;
}

image_t reindex_image(const image_t& image, const indexed_histogram_table_t& table) {
    std::size_t count(image.color_table().size());
    index_map_t sorted_map(count);

    for (std::size_t i(0); i < count; ++i) {
        sorted_map[table[i].second] = i;
    }

    return reindex_image(image, sorted_map);
}

/**************************************************************************************************/

inline double grey(const png_color& c) {
    double r = c.red / 255.;
    double g = c.green / 255.;
    double b = c.blue / 255.;

    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

/**************************************************************************************************/

struct rgba {
    std::uint16_t _r;
    std::uint16_t _g;
    std::uint16_t _b;
    std::uint16_t _a;
};

struct rgba64 {
    std::uint64_t _r;
    std::uint64_t _g;
    std::uint64_t _b;
    std::uint64_t _a;
};

inline bool operator<(const rgba& x, const rgba& y) {
    if (x._r < y._r) {
        return true;
    } else if (y._r < x._r) {
        return false;
    }

    if (x._g < y._g) {
        return true;
    } else if (y._g < x._g) {
        return false;
    }

    if (x._b < y._b) {
        return true;
    } else if (y._b < x._b) {
        return false;
    }

    return x._a < y._a;
}

/**************************************************************************************************/

typedef std::map<rgba, std::size_t> true_histogram_t;

true_histogram_t true_histogram(const image_t& image) {
    true_histogram_t result;
    auto             bpp(image.bpp());
    auto             p(image.begin());
    auto             last(image.end());

    if (bpp == 3) {
        while (p != last) {
            rgba rgb{
                *p++, *p++, *p++, 0,
            };

            ++result[rgb];
        }
    } else if (bpp == 4) {
        while (p != last) {
            rgba rgb{
                *p++, *p++, *p++, *p++,
            };

            ++result[rgb];
        }
    }

    return result;
}

/**************************************************************************************************/

inline auto sq_distance(std::int64_t x, std::int64_t y) {
    std::int64_t diff(x - y);

    return diff * diff;
}

/**************************************************************************************************/

inline auto sq_distance(const rgba& x, const rgba& y) {
    return sq_distance(x._r, y._r) + sq_distance(x._g, y._g) + sq_distance(x._b, y._b) +
           sq_distance(x._a, y._a);
}

/**************************************************************************************************/

inline auto sq_distance(const png_color& x, const rgba& y) {
    return sq_distance(x.red, y._r) + sq_distance(x.green, y._g) + sq_distance(x.blue, y._b) +
           sq_distance(255, y._a);
}

/**************************************************************************************************/

inline auto sq_distance(const rgba& x, const png_color& y) {
    return sq_distance(y, x);
}

/**************************************************************************************************/

auto euclidean_distance(const rgba& x, const rgba& y) {
    return std::sqrt(sq_distance(x._r, y._r) + sq_distance(x._g, y._g) + sq_distance(x._b, y._b) +
                     sq_distance(x._a, y._a));
}

/**************************************************************************************************/

std::vector<std::int64_t> compute_d(const std::vector<rgba>& colors,
                                    const std::vector<rgba>& seeds) {
    std::vector<std::int64_t> integral_values;
    std::int64_t              integral_sum{0};

    for (const auto& color : colors) {
        std::int64_t d(std::numeric_limits<std::int64_t>::max());

        for (const auto& seed : seeds) {
            d = std::min(d, sq_distance(color, seed));
        }

        integral_sum += d;
        integral_values.push_back(d);
    }

    return integral_values;
}

/**************************************************************************************************/

std::vector<rgba> k_means_pp(const std::vector<rgba>& v, std::size_t n) {
    std::random_device rd;
    std::mt19937       gen(rd());
    std::vector<rgba>  result;
    std::size_t        i(std::rand() % v.size());

    result.push_back(v[i]);

    while (result.size() < n) {
        auto                         d(compute_d(v, result));
        std::discrete_distribution<> dist(d.begin(), d.end());
        std::size_t                  index(dist(gen));

        result.push_back(v[index]);
    }

    return result;
}

/**************************************************************************************************/

color_table_t make_color_table(const std::vector<rgba>& v) {
    std::size_t   count(std::min<std::size_t>(v.size(), PNG_MAX_PALETTE_LENGTH));
    color_table_t result(count);

    for (std::size_t i(0); i < count; ++i) {
        result[i].red   = v[i]._r;
        result[i].green = v[i]._g;
        result[i].blue  = v[i]._b;
    }

    return result;
}

/**************************************************************************************************/

std::pair<std::size_t, double> quantize(const rgba& c, const color_table_t& table) {
    std::size_t  index{0};
    std::int64_t min_error{std::numeric_limits<std::int64_t>::max()};
    std::size_t  count(table.size());

    for (std::size_t i(0); i < count; ++i) {
        std::int64_t error(sq_distance(c, table[i]));

        if (error >= min_error)
            continue;

        index     = i;
        min_error = error;
    }

    return std::make_pair(index, std::sqrt(min_error));
}

/**************************************************************************************************/

inline auto quantize(const png_color& c, const color_table_t& table) {
    return quantize(rgba{c.red, c.green, c.blue, 255}, table);
}

/**************************************************************************************************/

typedef std::pair<image_t, image_t> quantization_t;

quantization_t quantize(const image_t& image, const color_table_t& color_table) {
    image_t result(
        image.width(), image.height(), image.depth(), image.width(), PNG_COLOR_TYPE_PALETTE);
    image_t error_result(
        image.width(), image.height(), image.depth(), image.width(), PNG_COLOR_TYPE_PALETTE);
    auto bpp(image.bpp());
    auto p(image.begin());
    auto last(image.end());
    auto dst(result.begin());
    auto err_dst(error_result.begin());

    if (bpp == 3) {
        while (p != last) {
            png_color color{
                *p++, *p++, *p++,
            };

            auto q(quantize(color, color_table));

            *dst++     = q.first;
            *err_dst++ = std::min<std::uint8_t>(std::lround(q.second), 255);
        }
    } else if (bpp == 4) {
        while (p != last) {
            rgba color{
                *p++, *p++, *p++, *p++,
            };

            auto q(quantize(color, color_table));

            *dst++     = q.first;
            *err_dst++ = std::min<std::uint8_t>(std::lround(q.second), 255);
        }
    }

    color_table_t error_table(PNG_MAX_PALETTE_LENGTH);

    for (std::size_t i(0); i < PNG_MAX_PALETTE_LENGTH; ++i) {
        error_table[i].red   = i;
        error_table[i].green = i;
        error_table[i].blue  = i;
    }

    result.set_color_table(color_table);
    error_result.set_color_table(error_table);

    return std::make_pair(std::move(result), std::move(error_result));
}

/**************************************************************************************************/

void palette_optimizations(const image_t& image, const path_t& output) {
    if (image.color_type() != PNG_COLOR_TYPE_PALETTE)
        return;

    indexed_histogram_table_t hist_table(make_indexed_histogram_table(image));

    std::sort(hist_table.begin(), hist_table.end(), [& _image = image](auto& x, auto& y) {
        if (x.first < y.first) {
            return true;
        } else if (y.first < x.first) {
            return false;
        }

        const auto& color_table(_image.color_table());
        const auto& x_color(color_table[x.second]);
        const auto& y_color(color_table[y.second]);

#if 1
        return grey(x_color) < grey(y_color);
#else
        return x_color < y_color;
#endif
    });

    indexed_histogram_table_t best_table;
    std::size_t               best_size{std::numeric_limits<std::size_t>::max()};

    auto save_and_best([& _image = image, &_best_size = best_size, &_best_table = best_table](
        const indexed_histogram_table_t& table, const path_t& path) {
        std::size_t size = verbose_save(reindex_image(_image, table), path);

        if (size >= _best_size)
            return;

        _best_size  = size;
        _best_table = table;
    });

    save_and_best(hist_table, derived_filename(output, "sorted"));

    std::reverse(hist_table.begin(), hist_table.end());

    save_and_best(hist_table, derived_filename(output, "sorted_reverse"));
}

/**************************************************************************************************/

void dump_quantization(const quantization_t& q, const path_t& output) {
    path_t error_output(associated_filename(output, "error"));

    verbose_save(q.first, output, save_mode::one);
    save_png(q.second, error_output, save_options_t());
}

/**************************************************************************************************/

std::tuple<image_t, std::uint64_t, color_table_t> k_means_round(const image_t& image,
                                                                color_table_t  color_table,
                                                                const path_t&  output,
                                                                std::size_t    r) {
    auto result(quantize(image, color_table));

    dump_quantization(result, derived_filename(output, "_r" + std::to_string(r)));

    std::size_t              count(color_table.size());
    std::vector<rgba64>      centroid_sums(count);
    std::vector<std::size_t> centroid_counts(count);
    auto                     bpp(image.bpp());
    auto                     p_index(result.first.begin());
    auto                     p_error(result.second.begin());
    auto                     p(image.begin());
    auto                     last(image.end());
    std::uint64_t            error{0};

    while (p != last) {
        std::size_t index(*p_index++);
        auto&       centroid(centroid_sums[index]);

        centroid._r += *p++;
        centroid._g += *p++;
        centroid._b += *p++;

        if (bpp == 4)
            centroid._a += *p++;

        ++centroid_counts[index];

        error += *p_error++;
    }

    for (std::size_t i(0); i < count; ++i) {
        auto cc = centroid_counts[i];

        if (cc == 0)
            continue;

        const auto& cur_sum(centroid_sums[i]);
        auto&       cur_table(color_table[i]);

        cur_table.red   = cur_sum._r / cc;
        cur_table.green = cur_sum._g / cc;
        cur_table.blue  = cur_sum._b / cc;
        //cur_table.alpha = cur_sum._a / cc;
    }

    return std::make_tuple(std::move(result.first), error, std::move(color_table));
}

/**************************************************************************************************/

color_table_t k_means(const image_t& image, color_table_t color_table, const path_t& output) {
    image_t     last_assigned;
    std::size_t r{0};

    while (true) {
        auto round(k_means_round(image, color_table, output, ++r));

        if (std::get<0>(round) == last_assigned)
            break;

        std::cout << "error: " << std::get<1>(round) << '\n';

        color_table   = std::move(std::get<2>(round));
        last_assigned = std::move(std::get<0>(round));
    };

    return color_table;
}

/**************************************************************************************************/

void truecolor_optimizations(const image_t& image, const path_t& output) {
    if (image.color_type() == PNG_COLOR_TYPE_PALETTE)
        return;

    true_histogram_t  histogram(true_histogram(image));
    std::vector<rgba> colors;

    for (const auto& color : histogram)
        colors.push_back(color.first);

    //auto tests = {2, 4, 8, 16, 32, 64, 128, 256};
    auto tests = {32};

    for (const auto& table_size : tests) {
        std::vector<rgba> seeds(k_means_pp(colors, table_size));
        color_table_t     color_table(make_color_table(seeds));
        auto              seed(quantize(image, color_table));

        dump_quantization(seed, derived_filename(output, std::to_string(table_size) + "_seed"));

        color_table_t km_table(k_means(image, color_table, output));
        auto          km(quantize(image, km_table));

        dump_quantization(km, derived_filename(output, std::to_string(table_size) + "_km"));
    }
}

/**************************************************************************************************/

int main(int argc, char** argv) try {
    if (argc <= 1)
        throw std::runtime_error("Source file not specified");

    if (argc <= 2)
        throw std::runtime_error("Destination directory not specified");

    std::srand(std::time(nullptr));

    path_t        input(canonical(argv[1]));
    path_t        output(argv[2]);
    const image_t original(read_png(input.string()));

    if (!exists(output))
        create_directory(output);

    output = canonical(output) / input.leaf();

    verbose_save(original, output);

    truecolor_optimizations(original, output);

    palette_optimizations(original, output);

    return 0;
} catch (const std::exception& error) {
    std::cerr << "Fatal error: " << error.what() << '\n';
    return 0;
} catch (...) {
    std::cerr << "Fatal error: unknown\n";
    return 0;
}
/**************************************************************************************************/
