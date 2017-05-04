/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

#ifndef PNGPP_RGBA_HPP__
#define PNGPP_RGBA_HPP__

// stdc++
#include <cstdint>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/
// Fixed point 0.8 bit arithmetic. This assumes your 8-bit value represents a
// floating point value of the range [0..1), and allows closed multiplication of
// two values. From "Three Wrongs Make a Right", James F. Blinn, IEEE Computer
// Graphics and Applications, Nov. 1995, Vol. 15, issue 6
inline constexpr std::uint8_t fixmul(std::uint8_t x, std::uint8_t y) {
    std::uint32_t i(x * y + 128);
    return static_cast<std::uint8_t>((i + (i >> 8)) >> 8);
}

/**************************************************************************************************/

template <typename T>
struct rgba {
    typedef T value_type;

    T _r;
    T _g;
    T _b;
    T _a;
};

/**************************************************************************************************/

template <typename T>
inline bool operator==(const rgba<T>& x, const rgba<T>& y) {
    return x._r == y._r && x._g == y._g && x._b == y._b && x._a == y._a;
}

template <typename T>
inline bool operator!=(const rgba<T>& x, const rgba<T>& y) {
    return !(x == y);
}

/**************************************************************************************************/

template <typename T>
inline rgba<T>& operator+=(rgba<T>& x, const rgba<T>& y) {
    x._r += y._r;
    x._g += y._g;
    x._b += y._b;
    x._a += y._a;

    return x;
}

template <typename T>
inline rgba<T>& operator-=(rgba<T>& x, const rgba<T>& y) {
    x._r -= y._r;
    x._g -= y._g;
    x._b -= y._b;
    x._a -= y._a;

    return x;
}

template <typename T>
inline rgba<T>& operator*=(rgba<T>& x, const rgba<T>& y) {
    x._r *= y._r;
    x._g *= y._g;
    x._b *= y._b;
    x._a *= y._a;

    return x;
}

template <typename T>
inline rgba<T>& operator/=(rgba<T>& x, const rgba<T>& y) {
    x._r /= y._r;
    x._g /= y._g;
    x._b /= y._b;
    x._a /= y._a;

    return x;
}

/**************************************************************************************************/

template <typename T>
inline rgba<T>& operator+=(rgba<T>& x, double y) {
    x._r = std::lround(x._r + y);
    x._g = std::lround(x._g + y);
    x._b = std::lround(x._b + y);
    x._a = std::lround(x._a + y);

    return x;
}

template <typename T>
inline rgba<T>& operator-=(rgba<T>& x, double y) {
    x._r = std::lround(x._r - y);
    x._g = std::lround(x._g - y);
    x._b = std::lround(x._b - y);
    x._a = std::lround(x._a - y);

    return x;
}

template <typename T>
inline rgba<T>& operator*=(rgba<T>& x, double y) {
    x._r = std::lround(x._r * y);
    x._g = std::lround(x._g * y);
    x._b = std::lround(x._b * y);
    x._a = std::lround(x._a * y);

    return x;
}

template <typename T>
inline rgba<T>& operator/=(rgba<T>& x, double y) {
    x._r = std::lround(x._r / y);
    x._g = std::lround(x._g / y);
    x._b = std::lround(x._b / y);
    x._a = std::lround(x._a / y);

    return x;
}

/**************************************************************************************************/

template <typename T, typename U>
inline rgba<T> operator+(rgba<T> x, const U& y) {
    x += y;
    return x;
}

template <typename T, typename U>
inline rgba<T> operator-(rgba<T> x, const U& y) {
    x -= y;
    return x;
}

template <typename T, typename U>
inline rgba<T> operator*(rgba<T> x, const U& y) {
    x *= y;
    return x;
}

template <typename T, typename U>
inline rgba<T> operator/(rgba<T> x, const U& y) {
    x /= y;
    return x;
}

/**************************************************************************************************/

template <typename T>
inline bool operator<(const rgba<T>& x, const rgba<T>& y) {
    if (x._r < y._r) {
        return true;
    } else if (y._r < x._r) {
        return false;
    }

    if (x._g < y._g) {
        return true;
    } else if (y._g < x._g) {
        return false;
    }

    if (x._b < y._b) {
        return true;
    } else if (y._b < x._b) {
        return false;
    }

    return x._a < y._a;
}

/**************************************************************************************************/

template <typename U, typename T>
inline U widen(const rgba<T>& c) {
    static_assert(sizeof(typename U::value_type) >= sizeof(T), "not a widen");

    return U{c._r, c._g, c._b, c._a};
}

/**************************************************************************************************/

template <typename U, typename T>
inline U shorten(const rgba<T>& c) {
    typedef typename U::value_type value_type;

    static_assert(sizeof(value_type) <= sizeof(T), "not a shorten");

    // this will break if the signedness differs between T and value_type.
    static const auto clip([](T x) {
        constexpr T tmin{std::numeric_limits<value_type>::min()};
        constexpr T tmax{std::numeric_limits<value_type>::max()};

        return static_cast<value_type>(std::max(tmin, std::min(x, tmax)));
    });

    return U{clip(c._r), clip(c._g), clip(c._b), clip(c._a)};
}

/**************************************************************************************************/

using rgba_t   = rgba<std::uint8_t>;
using rgba16_t = rgba<std::uint16_t>;
using rgba32_t = rgba<std::uint32_t>;
using rgba64_t = rgba<std::uint64_t>; // for accumulators, etc.

/**************************************************************************************************/
#if 0
template <typename T>
png_color make_png_color(const rgba<T>& src) {
    rgba<png_byte> x(shorten<rgba<png_byte>>(src));

    return {x._r, x._g, x._b};
}
#endif
/**************************************************************************************************/

inline rgba_t premultiply(rgba_t c) {
    c._r = fixmul(c._r, c._a);
    c._g = fixmul(c._g, c._a);
    c._b = fixmul(c._b, c._a);

    return c;
}

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/

#endif // PNGPP_RGBA_HPP__

/**************************************************************************************************/
