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

int main(int argc, char** argv) try {
    if (argc <= 1)
        throw std::runtime_error("Source file not specified");

    path_t path(canonical(argv[1]));

    std::cout << "Found `" << path.string() << "`\n";

    image_t image(read_png(path.string()));

    return 0;
} catch (const std::exception& error) {
    std::cerr << "Fatal error: " << error.what() << '\n';
    return 0;
} catch (...) {
    std::cerr << "Fatal error: unknown\n";
    return 0;
}
/**************************************************************************************************/
