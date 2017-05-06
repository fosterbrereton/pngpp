/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

// identity
#include <pngpp/rgba.hpp>

// stdc++
#include <array>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

namespace {

/**************************************************************************************************/

auto itof_g_init() {
    std::array<double, 256> result;

    for (std::size_t i(0); i < 256; ++i)
        result[i] = i / 255.;

    return result;
}

const auto itof_g{itof_g_init()};

/**************************************************************************************************/

auto fixmul_g_init() {
    std::array<double, 65536> result;

    for (std::size_t y(0); y < 256; ++y) {
        auto fy{itof(y)};
        for (std::size_t x(0); x < 256; ++x) {
            auto fx{itof(x)};
            result[y * 256 + x] = ftoi(fx * fy);
        }
    }

    return result;
}

const auto fixmul_g{fixmul_g_init()};

/**************************************************************************************************/

auto fixdiv_g_init() {
    std::array<double, 65536> result;

    for (std::size_t y(0); y < 256; ++y) {
        auto fy{itof(y)};
        for (std::size_t x(0); x < 256; ++x) {
            result[y * 256 + x] = y <= x ? 255 : ftoi(itof(x) / fy);
        }
    }

    return result;
}

const auto fixdiv_g{fixdiv_g_init()};

/**************************************************************************************************/

} // namespace

/**************************************************************************************************/

std::uint8_t fixmul(std::uint8_t x, std::uint8_t y) {
    return fixmul_g[y * 256 + x];
}

std::uint8_t fixdiv(std::uint8_t x, std::uint8_t y) {
    return fixdiv_g[y * 256 + x];
}

/**************************************************************************************************/

double itof(std::uint8_t x) {
    return itof_g[x];
}

std::uint8_t ftoi(double x) {
    return x >= 1 ? 255 : x <= 0 ? 0 : std::lround(x * 255);
}

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/
