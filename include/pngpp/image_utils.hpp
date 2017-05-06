/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

#ifndef PNGPP_IMAGE_UTILS_HPP__
#define PNGPP_IMAGE_UTILS_HPP__

/**************************************************************************************************/

// application
#include <pngpp/async.hpp>
#include <pngpp/files.hpp>
#include <pngpp/image.hpp>
#include <pngpp/png.hpp>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/
// Saves the image's color table (if present.)
future<void> dump_color_table(const image_t& image, path_t output);

/**************************************************************************************************/
// Saves the image (and, if present, its color table.)
future<void> dump_image(image_t   image,
                        path_t    path,
                        save_mode mode = save_mode::max);

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/

#endif // PNGPP_IMAGE_UTILS_HPP__

/**************************************************************************************************/
