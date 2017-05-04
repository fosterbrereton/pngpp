/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

// identity
#include <pngpp/image.hpp>

// tbb
#include <tbb/parallel_for.h>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

image_t premultiply(image_t image) {
    if (image.bpp() == 4) {
        auto area{image.area()};
        auto base{image.data()};

        tbb::parallel_for<decltype(area)>(0, area, 1, [_base = base](auto i) {
            auto p{_base + i * 4};
            auto a{p[3]};

            if (a != 255) {
                p[0] = fixmul(p[0], a);
                p[1] = fixmul(p[1], a);
                p[2] = fixmul(p[2], a);
            }
        });
    }

    return image;
}

/**************************************************************************************************/

image_t unpremultiply(image_t image) {
    // there be rounding error dragons here.
    if (image.bpp() == 4) {
        auto area{image.area()};
        auto base{image.data()};

        tbb::parallel_for<decltype(area)>(0, area, 1, [_base = base](auto i) {
            auto p{_base + i * 4};
            auto a{p[3]};

            if (a != 255) {
                p[0] = fixdiv(p[0], a);
                p[1] = fixdiv(p[1], a);
                p[2] = fixdiv(p[2], a);
            }
        });
    }

    return image;
}

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/
