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
    : _input(path.c_str(), std::ios_base::in | std::ios_base::binary),
      _png_struct(png_create_read_struct(
          PNG_LIBPNG_VER_STRING, nullptr, &png_reader_t::fail, &png_reader_t::warn)),
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
    image_t                result(width, height, depth, rowbytes, color_type);
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
#if 0
#pragma mark -
#endif
/**************************************************************************************************/

class png_writer_t {
    std::ofstream _output;

    static void flush_thunk(png_structp png) {}
    static void write_thunk(png_structp png, png_bytep buffer, png_size_t size);
    void write(png_bytep buffer, png_size_t size);

    static void fail(png_structp, png_const_charp);
    static void warn(png_structp, png_const_charp);

public:
    explicit png_writer_t(const std::string& path);
    ~png_writer_t();

    void write(const image_t& image, const write_options_t& options);
};

/**************************************************************************************************/

png_writer_t::png_writer_t(const std::string& path)
    : _output(path.c_str(), std::ios_base::out | std::ios_base::binary) {}

/**************************************************************************************************/

png_writer_t::~png_writer_t() {}

/**************************************************************************************************/

void png_writer_t::fail(png_structp, png_const_charp message) {
#ifndef NDEBUG
    std::cerr << message << '\n';
#endif

    throw std::runtime_error(message);
}

/**************************************************************************************************/

void png_writer_t::warn(png_structp, png_const_charp message) {
#ifndef NDEBUG
    std::cerr << message << '\n';
#endif
}

/**************************************************************************************************/

void png_writer_t::write(png_bytep buffer, png_size_t size) {
    _output.write(reinterpret_cast<char*>(buffer), size);
}

/**************************************************************************************************/

void png_writer_t::write_thunk(png_structp png, png_bytep buffer, png_size_t size) {
    if (!png || !buffer)
        png_error(png, "invalid pointer");

    bufferstream_t* writer(static_cast<bufferstream_t*>(png_get_io_ptr(png)));

    if (!writer)
        png_error(png, "invalid pointer");

    writer->write(buffer, size);
}

/**************************************************************************************************/

void png_writer_t::write(const image_t& image, const write_options_t& options) {
    if (!_output)
        png_error(nullptr, "file could not be opened for write");

    bufferstream_t stream;

    png_structp png_struct = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, nullptr, &png_writer_t::fail, &png_writer_t::warn);

    if (!png_struct)
        png_error(png_struct, "png_create_write_struct failed");

    png_infop png_info = png_create_info_struct(png_struct);

    if (!png_info)
        png_error(png_struct, "png_create_info_struct failed");

    png_set_write_fn(png_struct, &stream, &png_writer_t::write_thunk, &png_writer_t::flush_thunk);

    png_set_compression_buffer_size(png_struct, 1024 * 1024); // 1MB compression buffer

    png_set_compression_level(png_struct, options._one_z_compression);
    png_set_compression_mem_level(png_struct, MAX_MEM_LEVEL);
    png_set_compression_strategy(png_struct, options._one_z_strategy);
    png_set_compression_window_bits(png_struct, 15);
    png_set_compression_method(png_struct, Z_DEFLATED);

    png_set_filter(png_struct, 0, options._one_png_filter);
    png_set_packing(png_struct);
    png_set_benign_errors(png_struct, 1);

    png_set_IHDR(png_struct,
                 png_info,
                 image.width(),
                 image.height(),
                 image.depth(),
                 image.color_type(),
                 false, // interlacing
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    const auto&            color_table(image.get_color_table());
    std::vector<png_byte*> rows(
        buffer_rows(const_cast<png_byte*>(image.data()), image.height(), image.rowbytes()));

    if (!color_table.empty()) {
        png_set_PLTE(
            png_struct, png_info, color_table.data(), static_cast<int>(color_table.size()));
    }

    png_write_info(png_struct, png_info);

    png_write_image(png_struct, const_cast<png_bytepp>(rows.data()));

    png_write_end(png_struct, png_info);

    png_destroy_write_struct(&png_struct, &png_info);

    _output.write(reinterpret_cast<const char*>(stream.data()), stream.size());
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

void write_png(const image_t& image, const std::string& path, const write_options_t& options) {
    png_writer_t writer(path);

    writer.write(image, options);
}

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/