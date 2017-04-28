/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

// stdc++
#include <iostream>
#include <numeric>

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

void extract_color_table(const image_t& image, const path_t& output) {
    const auto& color_table(image.color_table());

    if (color_table.empty())
        return;

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
    extract_color_table(image, path);

    save_options_t options;

    options._mode = mode;

    std::size_t size(save_png(image, path, options));

    std::cout << path.leaf().string() << ": " << size << '\n';

    return size;
}

/**************************************************************************************************/

typedef std::vector<std::size_t> indexed_histogram_t;

indexed_histogram_t histogram(const image_t& image) {
    indexed_histogram_t result;

    if (image.color_type() == PNG_COLOR_TYPE_PALETTE) {
        result.resize(PNG_MAX_PALETTE_LENGTH, 0);

        for (auto entry : image)
            ++result[entry];
    }

    return result;
}

/**************************************************************************************************/

typedef std::pair<std::size_t, std::uint8_t> indexed_histogram_pair_t;
typedef std::vector<indexed_histogram_pair_t> indexed_histogram_table_t;

indexed_histogram_table_t make_indexed_histogram_table(const image_t& image) {
    indexed_histogram_table_t result;

    if (image.color_type() == PNG_COLOR_TYPE_PALETTE) {
        result.resize(PNG_MAX_PALETTE_LENGTH);

        indexed_histogram_t image_hist(histogram(image));

        for (std::size_t i(0); i < PNG_MAX_PALETTE_LENGTH; ++i) {
            result[i] = indexed_histogram_pair_t(image_hist[i], i);
        }
    }

    return result;
}

/**************************************************************************************************/

typedef std::vector<std::uint8_t> index_map_t;

index_map_t identity_index_map() {
    index_map_t result(PNG_MAX_PALETTE_LENGTH, 0);

    std::iota(result.begin(), result.end(), 0);

    return result;
}

image_t reindex_image(const image_t& image, const index_map_t& map) {
    if (image.color_type() != PNG_COLOR_TYPE_PALETTE)
        return image;

    image_t result(image);

    for (auto& entry : result)
        entry = map[entry];

    const auto&   src_table(image.color_table());
    color_table_t dst_table(src_table);

    for (std::size_t i(0); i < PNG_MAX_PALETTE_LENGTH; ++i)
        dst_table[map[i]] = src_table[i];

    result.set_color_table(dst_table);

    return result;
}

image_t reindex_image(const image_t& image, const indexed_histogram_table_t& table) {
    if (image.color_type() != PNG_COLOR_TYPE_PALETTE)
        return image;

    index_map_t sorted_map(PNG_MAX_PALETTE_LENGTH);

    for (std::size_t i(0); i < PNG_MAX_PALETTE_LENGTH; ++i) {
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

inline bool operator<(const png_color& x, const png_color& y) {
    if (x.green < y.green) {
        return true;
    } else if (y.green < x.green) {
        return false;
    }

    if (x.red < y.red) {
        return true;
    } else if (y.red < x.red) {
        return false;
    }

    return x.blue < y.blue;
}

/**************************************************************************************************/

void truecolor_optimizations(const image_t& image, const path_t& output) {
    if (image.color_type() == PNG_COLOR_TYPE_PALETTE)
        return;
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

    auto save_and_best([& _image = image, &_best_size = best_size, &_best_table = best_table ](
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
