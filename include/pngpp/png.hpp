/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

#ifndef PNGPP_PNG_HPP__
#define PNGPP_PNG_HPP__

// stdc++
#include <fstream>

// libpng
#include <png.h>

// zlib
#include <zlib.h>

// application
#include <pngpp/image.hpp>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

image_t read_png(const std::string& path);

/**************************************************************************************************/

enum class write_mode {
    one,
    mid,
    max
};

struct write_options_t {
    write_mode _mode{write_mode::one};
    int        _one_z_compression{Z_BEST_COMPRESSION};
    int        _one_z_strategy{Z_FILTERED};
    int        _one_png_filter{PNG_ALL_FILTERS};
};

void write_png(const image_t&         image,
               const std::string&     path,
               const write_options_t& options);

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/

#endif // PNGPP_PNG_HPP__

/**************************************************************************************************/
