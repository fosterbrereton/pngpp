/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

#ifndef PNGPP_IMAGE_HPP__
#define PNGPP_IMAGE_HPP__

/**************************************************************************************************/

// stdc++
#include <vector>

// libpng
#include <png.h>

// application
#include <pngpp/buffer.hpp>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

class image_t {
    std::size_t            _width{0};
    std::size_t            _height{0};
    std::size_t            _depth{0};
    std::size_t            _rowbytes{0};
    buffer_t               _buffer;
    std::vector<png_color> _color_table;

public:
    image_t() = default;

    image_t(std::size_t width, std::size_t height, std::size_t depth, std::size_t rowbytes)
        : _width(width), _height(height), _depth(depth), _rowbytes(rowbytes),
          _buffer(rowbytes * _height) {}

    std::uint8_t* data() {
        return _buffer.data();
    }
    const std::uint8_t* data() const {
        return _buffer.data();
    }

    std::size_t width() const {
        return _width;
    }
    std::size_t height() const {
        return _height;
    }
    std::uint8_t depth() const {
        return _depth;
    }

    void set_color_table(std::vector<png_color> color_table) {
        _color_table = std::move(color_table);
    }
};

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/

#endif // PNGPP_IMAGE_HPP__

/**************************************************************************************************/
