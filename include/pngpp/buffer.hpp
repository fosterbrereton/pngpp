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
    std::size_t _capacity;

public:
    typedef std::uint8_t value_type;

    buffer_t() : _capacity(0) {}

    explicit buffer_t(std::size_t size) : _buffer(std::malloc(size)), _capacity(size) {}

    buffer_t(const buffer_t& rhs) : buffer_t(rhs._capacity) {
        if (_capacity)
            std::memcpy(data(), rhs.data(), _capacity);
    }

    buffer_t(buffer_t&& rhs)
        : _buffer(std::move(rhs._buffer)), _capacity(std::move(rhs._capacity)) {
        rhs._capacity = 0;
    }

    buffer_t& operator=(buffer_t rhs) {
        _buffer   = std::move(rhs._buffer);
        _capacity = std::move(rhs._capacity);
        return *this;
    }

    bool empty() const {
        return _capacity == 0;
    }
    std::size_t capacity() const {
        return _capacity;
    }

    // returns true iff reallocation happened
    bool resize(std::size_t cap) {
        if (cap <= _capacity) {
            return false;
        }

        std::size_t grow_cap = static_cast<std::size_t>(std::ceil(_capacity * 1.4));
        std::size_t new_cap  = std::max(cap, grow_cap);
        buffer_t    result(new_cap);

        if (_capacity)
            std::memcpy(result.data(), data(), _capacity);

        std::swap(*this, result);

        return true;
    }

    value_type* data() {
        return reinterpret_cast<value_type*>(_buffer.get());
    }
    const value_type* data() const {
        return reinterpret_cast<const value_type*>(_buffer.get());
    }

    value_type& operator[](std::size_t index) {
        return *(data() + index);
    }
    const value_type& operator[](std::size_t index) const {
        return *(data() + index);
    }

    value_type* begin() {
        return data();
    }
    const value_type* begin() const {
        return data();
    }

    value_type* end() {
        return data() + capacity();
    }
    const value_type* end() const {
        return data() + capacity();
    }
};

/**************************************************************************************************/

class bufferstream_t {
    buffer_t    _buffer;
    std::size_t _pos{0};

public:
    typedef buffer_t::value_type value_type;

    void write(void* buffer, std::size_t size) {
        std::size_t needed(_pos + size);

        if (needed > capacity())
            _buffer.resize(needed);

        std::memcpy(end(), buffer, size);

        _pos += size;
    }

    bool empty() const {
        return _pos == 0;
    }
    std::size_t size() const {
        return _pos;
    }
    std::size_t capacity() const {
        return _buffer.capacity();
    }

    value_type* data() {
        return _buffer.data();
    }
    const value_type* data() const {
        return _buffer.data();
    }

    value_type* begin() {
        return _buffer.begin();
    }
    const value_type* begin() const {
        return _buffer.begin();
    }

    value_type* end() {
        return data() + _pos;
    }
    const value_type* end() const {
        return data() + _pos;
    }
};

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/

#endif // PNGPP_BUFFER_HPP__

/**************************************************************************************************/
