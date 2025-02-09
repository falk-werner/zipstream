#ifndef ZIPSTREAM_BUFFER_HPP
#define ZIPSTREAM_BUFFER_HPP

#include <cinttypes>
#include <cstddef>
#include <string>

namespace zipstream
{


class buffer
{
    buffer(buffer const &) = delete;
    buffer operator=(buffer const &) = delete;
    buffer(buffer &&) = delete;
    buffer operator=(buffer &&) = delete;
public:
    explicit buffer(size_t capacity);
    ~buffer();

    void write_u16(uint16_t value);
    void write_u32(uint32_t value);
    void write_str(std::string const & value);
    size_t write_position() const;

    bool empty() const;
    size_t read(char * buffer, size_t size);
    
    void reset();

private:
    uint8_t * data;
    size_t cap;
    size_t write_pos;
    size_t read_pos;
};

}

#endif
