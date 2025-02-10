#include "zipstream/entries/dir_entry.hpp"

namespace zipstream
{

dir_entry::dir_entry(std::string const & name)
: m_name(name)
{

}

std::string const & dir_entry::name() const
{
    return m_name;
}

uint32_t dir_entry::size() const
{
    return 0;
}

std::optional<uint32_t> dir_entry::crc32() const
{
    return 0;
}

size_t dir_entry::read_at(size_t offset, char * buffer, size_t buffer_size)
{
    (void) offset;
    (void) buffer;
    (void) buffer_size;

    return 0;
}

}