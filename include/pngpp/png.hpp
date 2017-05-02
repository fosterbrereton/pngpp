/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

#ifndef PNGPP_PNG_HPP__
#define PNGPP_PNG_HPP__

// stdc++
#include <fstream>
#include <future>

// libpng
#include <png.h>

// zlib
#include <zlib.h>

// application
#include <pngpp/files.hpp>
#include <pngpp/image.hpp>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

image_t read_png(const path_t& path);

/**************************************************************************************************/
// "save" connotes "to disk" more than "write" does (which could also be going to memory).
enum class save_mode { one, mid, max };

struct save_options_t {
    save_mode _mode{save_mode::one};
    int       _one_z_compression{Z_BEST_COMPRESSION};
    int       _one_z_strategy{Z_FILTERED};
    int       _one_png_filter{PNG_ALL_FILTERS};
};

// returns the size of the saved file in bytes.
std::future<std::size_t> save_png(const image_t&        image,
                                  const path_t&         path,
                                  const save_options_t& options);

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/

#endif // PNGPP_PNG_HPP__

/**************************************************************************************************/
