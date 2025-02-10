#ifndef ZIPSTREAM_ENTRY_I_HPP
#define ZIPSTREAM_ENTRY_I_HPP

#include <string>
#include <cstddef>
#include <cinttypes>
#include <optional>

namespace zipstream
{

class entry_i
{
public:
    virtual ~entry_i() = default;
    virtual std::string const & name() const = 0;
    virtual uint32_t size() const = 0;
    virtual std::optional<uint32_t> crc32() const = 0;
    virtual size_t read_at(size_t offset, char * buffer, size_t buffer_size) = 0;
};

}

#endif
