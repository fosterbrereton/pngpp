/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

#ifndef PNGPP_IMAGE_HPP__
#define PNGPP_IMAGE_HPP__

/**************************************************************************************************/

// stdc++
#include <vector>
#include <string>

// libpng
#include <png.h>

// application
#include <pngpp/buffer.hpp>
#include <pngpp/rgba.hpp>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

typedef std::vector<rgba_t> color_table_t;

/**************************************************************************************************/

class image_t {
    std::size_t   _width{0};
    std::size_t   _height{0};
    std::size_t   _depth{0};
    std::size_t   _rowbytes{0};
    int           _color_type{0};
    buffer_t      _buffer;
    color_table_t _color_table;

    friend bool operator==(const image_t& x, const image_t& y);

public:
    image_t() = default;

    image_t(std::size_t width,
            std::size_t height,
            std::size_t depth,
            std::size_t rowbytes,
            int         color_type)
        : _width(width), _height(height), _depth(depth), _rowbytes(rowbytes),
          _color_type(color_type), _buffer(rowbytes * _height) {
        if (_depth != 8)
            throw std::runtime_error("depth " + std::to_string(_depth) + " not supported.");
    }

    auto data() {
        return _buffer.data();
    }
    auto data() const {
        return _buffer.data();
    }
    auto begin() {
        return _buffer.begin();
    }
    auto begin() const {
        return _buffer.begin();
    }
    auto end() {
        return _buffer.end();
    }
    auto end() const {
        return _buffer.end();
    }

    auto width() const {
        return _width;
    }
    auto height() const {
        return _height;
    }
    auto depth() const {
        return _depth;
    }
    auto rowbytes() const {
        return _rowbytes;
    }
    auto color_type() const {
        return _color_type;
    }
    const auto& color_table() const {
        return _color_table;
    }

    auto area() const {
        return width() * height();
    }

    auto bpp() const {
        return rowbytes() / width();
    }

    void set_color_table(color_table_t color_table) {
        _color_table = std::move(color_table);
    }

    template <typename T>
    rgba<T> pixel(std::size_t index) const {
        auto bp{bpp()};
        auto offset(index * bp);
        auto base(data() + offset);

        // this needs to be revisited for non-truecolor PNGs.
        rgba<std::uint8_t> base_pixel{base[0],
                                      base[1],
                                      base[2],
                                      static_cast<std::uint8_t>(bp == 4 ? base[0] : 255)};
        return widen<rgba<T>>(base_pixel);
    }

    template <typename T>
    rgba<T> pixel(std::size_t x, std::size_t y) const {
        return pixel<T>(y * _width + x);
    }
};

/**************************************************************************************************/

inline bool operator==(const image_t& x, const image_t& y) {
    return x._width == y._width && x._height == y._height && x._depth == y._depth &&
           x._rowbytes == y._rowbytes && x._color_type == y._color_type && x._buffer == y._buffer &&
           x._color_table == y._color_table;
}

inline bool operator!=(const image_t& x, const image_t& y) {
    return !(x == y);
}

/**************************************************************************************************/
// if the image includes an alpha channel, it is premultiplied into it
image_t premultiply(image_t image);

// if the image includes an alpha channel, it is unpremultiplied from it
image_t unpremultiply(image_t image);

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/

inline bool operator==(const png_color& x, const png_color& y) {
    return x.red == y.red && x.green == y.green && x.blue == y.blue;
}
inline bool operator!=(const png_color& x, const png_color& y) {
    return !(x == y);
}

inline bool operator<(const png_color& x, const png_color& y) {
    if (x.red < y.red) {
        return true;
    } else if (y.red < x.red) {
        return false;
    }

    if (x.green < y.green) {
        return true;
    } else if (y.green < x.green) {
        return false;
    }

    return x.blue < y.blue;
}

/**************************************************************************************************/

#endif // PNGPP_IMAGE_HPP__

/**************************************************************************************************/
