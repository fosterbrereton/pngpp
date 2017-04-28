/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

// stdc++
#include <iostream>

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

void extract_color_table_image(const image_t& image, path_t& output) {
    const auto& color_table(image.color_table());

    if (color_table.empty())
        return;

    image_t table(16, 16, 8, 16, PNG_COLOR_TYPE_PALETTE);
    auto    first(table.data());

    for (std::size_t i(0); i < 256; ++i) {
        *first++ = static_cast<std::uint8_t>(i);
    }

    table.set_color_table(color_table);

    write_png(table, output.string() + "_table.png", write_options_t());
}

/**************************************************************************************************/

int main(int argc, char** argv) try {
    if (argc <= 1)
        throw std::runtime_error("Source file not specified");

    if (argc <= 2)
        throw std::runtime_error("Destination file not specified");

    path_t          input(canonical(argv[1]));
    path_t          output(argv[2]);
    image_t         image(read_png(input.string()));
    write_options_t options;

    //options._mode = write_mode::one;
    options._mode = write_mode::mid;
    //options._mode = write_mode::max;

    write_png(image, output.string(), options);
    extract_color_table_image(image, output);

    return 0;
} catch (const std::exception& error) {
    std::cerr << "Fatal error: " << error.what() << '\n';
    return 0;
} catch (...) {
    std::cerr << "Fatal error: unknown\n";
    return 0;
}
/**************************************************************************************************/
