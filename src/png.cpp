/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

// identity
#include <pngpp/png.hpp>

// stdc++
#include <iostream>
#include <vector>

// application
// #include <pngpp/files.hpp>

/**************************************************************************************************/

using namespace pngpp;

/**************************************************************************************************/

namespace {

/**************************************************************************************************/

std::vector<png_byte*> buffer_rows(png_byte* p, std::size_t height, std::size_t rowbytes) {
    std::vector<png_byte*> result(height);

    for (auto& row : result) {
        row = p;
        p += rowbytes;
    }

    return result;
}

/**************************************************************************************************/

class png_reader_t {
    std::ifstream _input;
    png_structp   _png_struct{nullptr};
    png_infop     _png_info{nullptr};
    png_infop     _png_end_info{nullptr};

    static void read_thunk(png_structp png, png_bytep buffer, png_size_t size);
    void read(png_bytep buffer, png_size_t size);

    static void fail(png_structp, png_const_charp);
    static void warn(png_structp, png_const_charp);

public:
    explicit png_reader_t(const std::string& path);
    ~png_reader_t();

    image_t read();

    bool has_chunk(png_uint_32 flag) const;
};

/**************************************************************************************************/

png_reader_t::png_reader_t(const std::string& path)
    : _input(path.c_str()),
      _png_struct(png_create_read_struct(
          PNG_LIBPNG_VER_STRING, this, &png_reader_t::fail, &png_reader_t::warn)),
      _png_info(png_create_info_struct(_png_struct)),
      _png_end_info(png_create_info_struct(_png_struct)) {
    if (!_input)
        png_error(_png_struct, "file could not be opened for read");

    if (!_png_struct)
        png_error(_png_struct, "png_create_read_struct failed");

    if (!_png_info)
        png_error(_png_struct, "png_create_info_struct failed");

    if (!_png_end_info)
        png_error(_png_struct, "png_create_info_struct failed");

    png_set_read_fn(_png_struct, this, &png_reader_t::read_thunk);
    png_set_crc_action(_png_struct, PNG_CRC_WARN_USE, PNG_CRC_WARN_USE);
}

/**************************************************************************************************/

image_t png_reader_t::read() {
    png_read_info(_png_struct, _png_info);

    png_uint_32 width(png_get_image_width(_png_struct, _png_info));
    png_uint_32 height(png_get_image_height(_png_struct, _png_info));
    png_byte    depth(png_get_bit_depth(_png_struct, _png_info));
    png_byte    color_type(png_get_color_type(_png_struct, _png_info));
    bool        has_alpha_channel(color_type & PNG_COLOR_MASK_ALPHA);
    bool        has_alpha(has_alpha_channel || has_chunk(PNG_INFO_tRNS));

    png_set_interlace_handling(_png_struct);

    if (depth > 8) {
        png_set_swap(_png_struct); // litte endian representation for channel data > 8bpp
    }

    if (has_alpha) {
        png_set_palette_to_rgb(_png_struct);
        png_set_tRNS_to_alpha(_png_struct);
    }

    png_read_update_info(_png_struct, _png_info);

    png_size_t             rowbytes(png_get_rowbytes(_png_struct, _png_info));
    image_t                result(width, height, depth, rowbytes);
    std::vector<png_byte*> rows(buffer_rows(result.data(), height, rowbytes));

    png_read_image(_png_struct, &rows[0]);
    png_read_end(_png_struct, _png_end_info);

    if (has_chunk(PNG_INFO_PLTE)) {
        png_colorp table{0};
        int        count{0};

        png_get_PLTE(_png_struct, _png_info, &table, &count);

        result.set_color_table(std::vector<png_color>(&table[0], &table[count]));
    }

    return result;
}

/**************************************************************************************************/

png_reader_t::~png_reader_t() {
    png_destroy_read_struct(&_png_struct, &_png_info, &_png_end_info);
}

/**************************************************************************************************/

void png_reader_t::fail(png_structp, png_const_charp message) {
#ifndef NDEBUG
    std::cerr << message << '\n';
#endif

    throw std::runtime_error(message);
}

/**************************************************************************************************/

void png_reader_t::warn(png_structp, png_const_charp message) {
#ifndef NDEBUG
    std::cerr << message << '\n';
#endif
}

/**************************************************************************************************/

void png_reader_t::read(png_bytep buffer, png_size_t size) {
    _input.read(reinterpret_cast<char*>(buffer), size);
}

/**************************************************************************************************/

void png_reader_t::read_thunk(png_structp png, png_bytep buffer, png_size_t size) {
    if (!png || !buffer)
        png_error(png, "invalid pointer");

    void* reader = png_get_io_ptr(png);

    if (!reader)
        png_error(png, "invalid pointer");

    static_cast<png_reader_t*>(reader)->read(buffer, size);
}

/**************************************************************************************************/

bool png_reader_t::has_chunk(png_uint_32 flag) const {
    return png_get_valid(_png_struct, _png_info, flag) == flag;
}

/**************************************************************************************************/

} // namespace

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

image_t read_png(const std::string& path) {
    png_reader_t reader(path);

    return reader.read();
}

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/
