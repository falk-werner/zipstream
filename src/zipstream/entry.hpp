#ifndef ZIPSTREAM_ENTRY_HPP
#define ZIPSTREAM_ENTRY_HPP

#include "zipstream/entry_i.hpp"
#include "zipstream/crc32sum.hpp"

#include <memory>

// #include <zipstream/entry_type.hpp>
// #include <string>
// #include <cinttypes>

namespace zipstream
{

struct entry
{
    entry()
    : offset(0)
    {

    }

    uint32_t offset;
    std::unique_ptr<entry_i> inner_entry;
    crc32sum computed_crc32;

    

    inline std::string const name() const
    {
        return inner_entry->name();
    }

    inline uint32_t crc32() const
    {
        return inner_entry->crc32().value_or(0);
    }

    inline uint32_t size() const
    {
        return inner_entry->size();
    }

    inline size_t read_at(size_t offset, char * buffer, size_t buffer_size)
    {
        return inner_entry->read_at(offset, buffer, buffer_size);
    }

    inline bool data_descriptor_needed() const
    {
        return !inner_entry->crc32().has_value();        
    }
};

}

#endif
