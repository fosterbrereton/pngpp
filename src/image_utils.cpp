/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

// identity
#include <pngpp/image_utils.hpp>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

future<void> dump_color_table(const image_t& image, path_t output) {
    const auto& color_table = image.color_table();

    if (color_table.empty())
        return make_ready_future();

    return async([_color_table = image.color_table(), _output = std::move(output)]() {
        constexpr std::size_t swatch_size_k(32);

        const std::size_t count(_color_table.size());
        const std::size_t dim(std::ceil(std::sqrt(count)));
        const std::size_t width(dim * swatch_size_k);
        const std::size_t height(dim * swatch_size_k);
        const std::size_t depth(8);
        const std::size_t rowbytes(width * 4);
        image_t           table(width, height, depth, rowbytes, PNG_COLOR_TYPE_RGB_ALPHA);
        auto              first(table.data());

        for (std::size_t y(0); y < dim; ++y) {
            for (std::size_t i(0); i < swatch_size_k; ++i) {
                for (std::size_t x(0); x < dim; ++x) {
                    std::size_t   index(dim * y + x);
                    bool          valid(index < count);
                    const rgba_t& entry(valid ? _color_table[index] : rgba_t());
                    for (std::size_t j(0); j < swatch_size_k; ++j) {
                        *first++ = entry._r;
                        *first++ = entry._g;
                        *first++ = entry._b;
                        *first++ = i > j ? entry._a : valid ? 255 : 0;
                    }
                }
            }
        }

        save_png(table, _output, save_options_t()).get();
    });
}

/**************************************************************************************************/

future<void> dump_image(image_t   image,
                        path_t    output,
                        save_mode mode) {
    return async([_image = std::move(image), _output = std::move(output), _mode = mode]() {
        dump_color_table(_image, associated_filename(_output, "table"));

        save_options_t options;

        options._mode = _mode;

        save_png(_image.premultiplied() ? unpremultiply(_image) : _image, _output, options).get();
    });
}

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/
