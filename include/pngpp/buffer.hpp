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
    std::size_t _size;
    std::size_t _capacity;

public:
    typedef std::uint8_t value_type;

    buffer_t() : _size(0), _capacity(0) {}

    explicit buffer_t(std::size_t size)
        : _buffer(std::malloc(size)), _size(size), _capacity(size) {}

    buffer_t(const buffer_t& rhs) : buffer_t(rhs._size) {
        if (_size)
            std::memcpy(data(), rhs.data(), _size);
    }

    buffer_t(buffer_t&& rhs)
        : _buffer(std::move(rhs._buffer)), _size(std::move(rhs._size)),
          _capacity(std::move(rhs._capacity)) {
        rhs._size     = 0;
        rhs._capacity = 0;
    }

    buffer_t& operator=(buffer_t rhs) {
        _buffer   = std::move(rhs._buffer);
        _size     = std::move(rhs._size);
        _capacity = std::move(rhs._capacity);
        return *this;
    }

    bool empty() const {
        return _size == 0;
    }
    std::size_t size() const {
        return _size;
    }
    std::size_t capacity() const {
        return _capacity;
    }

    // returns true iff reallocation happened
    bool resize(std::size_t size) {
        if (size <= _capacity) {
            _size = size;
            return false;
        }

        std::size_t grow_cap = static_cast<std::size_t>(std::ceil(_capacity * 1.4));
        std::size_t new_cap  = std::max(size, grow_cap);

        buffer_t result(new_cap);

        if (_size)
            std::memcpy(result.data(), data(), _size);

        result.resize(size);

        std::swap(*this, result);

        return true;
    }

    // returns true iff reallocation happened
    bool shrink_to_fit() {
        if (_size == _capacity)
            return false;

        buffer_t result(_size);

        if (_size)
            std::memcpy(result.data(), data(), _size);

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
        return data() + size();
    }
    const value_type* end() const {
        return data() + size();
    }
};

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/

#endif // PNGPP_BUFFER_HPP__

/**************************************************************************************************/
