/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

#ifndef PNGPP_BUFFER_HPP__
#define PNGPP_BUFFER_HPP__

/**************************************************************************************************/

// stdc++
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

class buffer_t {
    struct deleter_t {
        void operator()(void* x) const {
            std::free(x);
        }
    };

    std::unique_ptr<void, deleter_t> _buffer;
    std::size_t _size{0};

public:
    typedef std::uint8_t value_type;

    buffer_t() = default;

    explicit buffer_t(std::size_t size) : _buffer(std::malloc(size)), _size(size) {}

    buffer_t(const buffer_t& rhs) : buffer_t(rhs._size) {
        if (_size)
            std::memcpy(data(), rhs.data(), _size);
    }

    buffer_t(buffer_t&& rhs) : _buffer(std::move(rhs._buffer)), _size(std::move(rhs._size)) {
        rhs._size = 0;
    }

    buffer_t& operator=(buffer_t rhs) {
        _buffer = std::move(rhs._buffer);
        _size   = std::move(rhs._size);
        return *this;
    }

    bool empty() const {
        return _size == 0;
    }
    std::size_t size() const {
        return _size;
    }

    value_type* data() {
        return reinterpret_cast<value_type*>(_buffer.get());
    }
    const value_type* data() const {
        return reinterpret_cast<const value_type*>(_buffer.get());
    }
    auto operator[](std::size_t index) {
        return *(data() + index);
    }
    auto operator[](std::size_t index) const {
        return *(data() + index);
    }
    auto begin() {
        return data();
    }
    auto begin() const {
        return data();
    }
    auto end() {
        return data() + size();
    }
    auto end() const {
        return data() + size();
    }

    // returns true iff reallocation happened
    bool resize(std::size_t size) {
        if (size <= _size) {
            return false;
        }

        std::size_t grow_size = static_cast<std::size_t>(std::ceil(_size * 1.4));
        std::size_t new_size  = std::max(size, grow_size);
        buffer_t    result(new_size);

        if (_size)
            std::memcpy(result.data(), data(), _size);

        std::swap(*this, result);

        return true;
    }
};

/**************************************************************************************************/

class bufferstream_t {
    buffer_t    _buffer;
    std::size_t _pos{0};

public:
    typedef buffer_t::value_type value_type;

    auto empty() const {
        return _pos == 0;
    }
    auto size() const {
        return _pos;
    }
    auto capacity() const {
        return _buffer.size();
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
        return data() + _pos;
    }
    auto end() const {
        return data() + _pos;
    }

    buffer_t move_buffer() {
        _pos = 0;
        return std::move(_buffer);
    }

    void write(void* buffer, std::size_t size) {
        std::size_t needed(_pos + size);

        if (needed > capacity())
            _buffer.resize(needed);

        std::memcpy(end(), buffer, size);

        _pos += size;
    }
};

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/

#endif // PNGPP_BUFFER_HPP__

/**************************************************************************************************/
