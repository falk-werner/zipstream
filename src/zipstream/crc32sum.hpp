#ifndef ZIPSTREAM_CRC32_HPP
#define ZIPSTREAM_CRC32_HPP

#include <zlib.h>

#include <string>
#include <cinttypes>

namespace zipstream
{

class crc32sum
{
public:
    crc32sum();
    ~crc32sum() = default;
    void update(char const * buffer, size_t buffer_size);
    uint32_t get_value() const;

    static uint32_t from_string(std::string const & value);
    static uint32_t from_file(std::string const & filename);
private:
    uint32_t value;
};

}

#endif
