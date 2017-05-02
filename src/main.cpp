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
#include <pngpp/rgba.hpp>

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

typedef std::map<rgba_t, std::size_t> true_histogram_t;

true_histogram_t true_histogram(const image_t& image) {
    true_histogram_t result;
    auto             bpp(image.bpp());
    auto             p(image.begin());
    auto             last(image.end());

    if (bpp == 3) {
        while (p != last) {
            rgba_t rgb{
                *p++, *p++, *p++, 255,
            };

            ++result[rgb];
        }
    } else if (bpp == 4) {
        while (p != last) {
            rgba_t rgb{
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

inline auto sq_distance(const rgba_t& x, const rgba_t& y) {
    return sq_distance(x._r, y._r) + sq_distance(x._g, y._g) + sq_distance(x._b, y._b) +
           sq_distance(x._a, y._a);
}

/**************************************************************************************************/

inline auto sq_distance(const png_color& x, const rgba_t& y) {
    return sq_distance(x.red, y._r) + sq_distance(x.green, y._g) + sq_distance(x.blue, y._b) +
           sq_distance(255, y._a);
}

/**************************************************************************************************/

inline auto sq_distance(const rgba_t& x, const png_color& y) {
    return sq_distance(y, x);
}

/**************************************************************************************************/

auto euclidean_distance(const rgba_t& x, const rgba_t& y) {
    return std::sqrt(sq_distance(x._r, y._r) + sq_distance(x._g, y._g) + sq_distance(x._b, y._b) +
                     sq_distance(x._a, y._a));
}

/**************************************************************************************************/

std::vector<std::int64_t> compute_sq_d(const std::vector<rgba_t>& colors,
                                       const std::vector<rgba_t>& seeds) {
    std::size_t               count(colors.size());
    std::vector<std::int64_t> values(count);

    tbb::parallel_for<std::size_t>(
        0, count, 1, [& _colors = colors, &_seeds = seeds, &_values = values](std::size_t i) {
            const auto&  color = _colors[i];
            std::int64_t d(std::numeric_limits<std::int64_t>::max());

            for (const auto& seed : _seeds) {
                d = std::min(d, sq_distance(color, seed));
            }

            _values[i] = d;
        });

    return values;
}

/**************************************************************************************************/

std::vector<rgba_t> k_means_pp(const std::vector<rgba_t>& v, std::size_t n) {
    std::random_device  rd;
    std::mt19937        gen(rd());
    std::vector<rgba_t> result;
    std::size_t         i(std::rand() % v.size());

    result.push_back(v[i]);

    while (result.size() < n) {
        auto                         d(compute_sq_d(v, result));
        std::discrete_distribution<> dist(d.begin(), d.end());
        std::size_t                  index(dist(gen));

        result.push_back(v[index]);
    }

    return result;
}

/**************************************************************************************************/

color_table_t make_color_table(const std::vector<rgba_t>& v) {
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

color_table_t make_grad_table() {
    // grayscale gradient color table from black to white. It might be better to
    // make this e.g., a green or yellow gradient instead, to ease visibility in
    // the dark range.
    std::size_t   count(PNG_MAX_PALETTE_LENGTH);
    color_table_t result(count);

    for (std::size_t i(0); i < count; ++i) {
        result[i].red   = i;
        result[i].green = i;
        result[i].blue  = i;
    }

    return result;
}

/**************************************************************************************************/

std::pair<std::size_t, double> quantize(const rgba_t& c, const color_table_t& table) {
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
    return quantize(rgba_t{c.red, c.green, c.blue, 255}, table);
}

/**************************************************************************************************/

typedef std::pair<image_t, image_t> quantization_t;

quantization_t quantize(const image_t& image, color_table_t color_table) {
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
            rgba_t color{
                *p++, *p++, *p++, *p++,
            };

            auto q(quantize(color, color_table));

            *dst++     = q.first;
            *err_dst++ = std::min<std::uint8_t>(std::lround(q.second), 255);
        }
    }

    result.set_color_table(std::move(color_table));
    error_result.set_color_table(make_grad_table());

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

void dump_quantization(const image_t& image, const image_t& error_image, const path_t& output) {
    path_t error_output(associated_filename(output, "error"));

    verbose_save(image, output, save_mode::one);
    save_png(error_image, error_output, save_options_t());
}

/**************************************************************************************************/

inline void dump_quantization(const quantization_t& q, const path_t& output) {
    dump_quantization(q.first, q.second, output);
}

/**************************************************************************************************/

struct centroid_cache_t {
    std::vector<rgba64_t>      _colors; // cumulative region color
    std::vector<std::uint64_t> _count;  // number of region members

    centroid_cache_t() = default;

    explicit centroid_cache_t(std::size_t count) : _colors(count), _count(count) {}

    std::size_t size() const {
        return _colors.size();
    }

    rgba64_t& centroid(std::size_t index) {
        return _colors[index];
    }
    const rgba64_t& centroid(std::size_t index) const {
        return _colors[index];
    }

    std::uint64_t& count(std::size_t index) {
        return _count[index];
    }
    std::uint64_t count(std::size_t index) const {
        return _count[index];
    }

    void add_member(std::size_t index, const rgba64_t& color) {
        ++count(index);

        centroid(index) += color;
    }

    void remove_member(std::size_t index, const rgba64_t& color) {
        --count(index);

        centroid(index) -= color;
    }

    void move_member(std::size_t src_index, std::size_t dst_index, const rgba64_t& color) {
        if (src_index == dst_index)
            return;

        remove_member(src_index, color);
        add_member(dst_index, color);
    }

    rgba_t color(std::size_t index) const {
        rgba_t result{0, 0, 0, 0};
        double c(count(index));

        if (c) {
            result = shorten<rgba_t>(centroid(index) / c);
        }

        return result;
    }

    color_table_t table() const {
        auto          count(size());
        color_table_t result;

        for (std::size_t i(0); i < count; ++i)
            result.push_back(make_png_color(color(i)));

        return result;
    }
};

struct round_state_t {
    round_state_t() = default;

    explicit round_state_t(std::size_t count) : _centroids(count) {}

    auto size() const {
        return _centroids.size();
    }

    color_table_t centroid_table() const {
        return _centroids.table();
    }

    std::uint64_t error() const {
        return std::accumulate(_image_error.begin(), _image_error.end(), std::uint64_t(0));
    }

    std::size_t      _r{0};        // iteration count
    image_t          _image;       // original image quantized with current color table
    image_t          _image_error; // rounded per-pixel quantization error
    centroid_cache_t _centroids;   // cache of cumulative centroid values
};

void dump_round(const round_state_t& round, const path_t& output) {
    std::cout << "r" << round._r << " error: " << round.error() << '\n';

    dump_quantization(round._image, // image contains this round's color table
                      round._image_error,
                      derived_filename(output, "_r" + std::to_string(round._r)));
}

round_state_t k_means_init_state(const image_t& original, color_table_t seed) {
    round_state_t result(seed.size());

    std::tie(result._image, result._image_error) = quantize(original, std::move(seed));

    auto bpp(original.bpp());
    auto p_index(result._image.begin());
    auto p(original.begin());
    auto last(original.end());

    while (p != last) {
        std::size_t index(*p_index++);
        rgba64_t    color{*p++, *p++, *p++, std::uint64_t(bpp == 4 ? *p++ : 255)};

        result._centroids.add_member(index, color);
    }

    return result;
}

void k_means_round(const image_t& original, const image_t& prev_image, round_state_t& state) {
    ++state._r;

    // requantize the original image with the updated centroid color table
    std::tie(state._image, state._image_error) = quantize(original, state.centroid_table());

    auto bpp(original.bpp());
    auto p_prior_index(prev_image.begin());
    auto p_index(state._image.begin());
    auto p(original.begin());
    auto last(original.end());

    while (p != last) {
        std::size_t prior_index(*p_prior_index++);
        std::size_t index(*p_index++);

        if (prior_index == index) {
            p += bpp;
            continue;
        }

        rgba64_t color{*p++, *p++, *p++, std::uint64_t(bpp == 4 ? *p++ : 255)};

        state._centroids.move_member(prior_index, index, color);
    }
}

/**************************************************************************************************/

color_table_t k_means(const image_t& image, color_table_t color_table, const path_t& output) {
    round_state_t round_state;
    std::uint64_t best_error(std::numeric_limits<std::uint64_t>::max());
    color_table_t best_table;
    bool          first{true};

    while (true) {
        image_t prev_image;

        if (first) {
            round_state = k_means_init_state(image, std::move(color_table));

            first = false;
        } else {
            prev_image = std::move(round_state._image);

            k_means_round(image, prev_image, round_state);
        }

        dump_round(round_state, output);

        std::uint64_t error(round_state.error());

        if (error < best_error) {
            best_table = round_state._image.color_table(); // copy
            best_error = error;

            // exact quantization found
            if (best_error == 0)
                break;
        }

        if (prev_image == round_state._image)
            break;
    };

    return best_table;
}

/**************************************************************************************************/

void truecolor_optimizations(const image_t& image, const path_t& output) {
    if (image.color_type() == PNG_COLOR_TYPE_PALETTE)
        return;

    true_histogram_t    histogram(true_histogram(image));
    std::vector<rgba_t> colors;

    for (const auto& color : histogram)
        colors.push_back(color.first);

    //auto tests = {2, 4, 8, 16, 32, 64, 128, 256};
    auto tests = {256};

    for (const auto& table_size : tests) {
        std::vector<rgba_t> seeds(k_means_pp(colors, table_size));
        color_table_t       color_table(make_color_table(seeds));
        auto                seed(quantize(image, color_table));

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

//    std::srand(std::time(nullptr));

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
