#ifndef ZIPSTREAM_ENTRY_HPP
#define ZIPSTREAM_ENTRY_HPP

#include <zipstream/entry_type.hpp>
#include <string>
#include <cinttypes>

namespace zipstream
{

struct entry
{
    entry_type type;
    std::string name;
    std::string value;
    uint32_t crc32;
    uint32_t size;
    uint32_t offset;
};

}

#endif
