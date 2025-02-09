#include "zipstream/buffer.hpp"
#include <stdexcept>
#include <cstring>
#include <algorithm>

namespace zipstream
{

buffer::buffer(size_t capacity)
: data(new uint8_t[capacity])
, cap(capacity)
, read_pos(0)
, write_pos(0)
{

}

buffer::~buffer()
{
    delete[] data;
}

void buffer::write_u16(uint16_t value)
{
    if ((cap - write_pos) < 2)
    {
        throw std::runtime_error("buffer too small");
    }

    uint8_t const low = static_cast<uint8_t>(value & 0xff);
    uint8_t const high = static_cast<uint8_t>((value >> 8) & 0xff);

    data[write_pos++] = low;
    data[write_pos++] = high;
}

void buffer::write_u32(uint32_t value)
{
    if ((cap - write_pos) < 4)
    {
        throw std::runtime_error("buffer too small");
    }

    for(size_t i = 0; i < 4; i++)
    {
        data[write_pos++] = static_cast<uint8_t>(value & 0xff);
        value >>= 8;
    }
}

void buffer::write_str(std::string const & value)
{
    if (value.size() == 0) { return; }

    if ((cap - write_pos) < value.size())
    {
        throw std::runtime_error("buffer too small");
    }

    char * const target = reinterpret_cast<char*>(&data[write_pos]);
    memcpy(target, value.data(), value.size());
    write_pos += value.size();
}

size_t buffer::write_position() const
{
    return write_pos;
}

bool buffer::empty() const
{
    return (read_pos == write_pos);
}

size_t buffer::read(char * buffer, size_t size)
{
    size_t const available = write_pos - read_pos;
    size_t const count = std::min(available, size);

    char * const source = reinterpret_cast<char*>(&data[read_pos]);
    memcpy(buffer, source, count);

    read_pos += count;

    // automatically reset buffer if all data is read
    if (read_pos == write_pos)
    {
        reset();
    }

    return count;
}

void buffer::reset() {
    read_pos = 0;
    write_pos = 0;
}


}