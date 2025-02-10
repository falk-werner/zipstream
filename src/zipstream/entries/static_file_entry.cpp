#include "zipstream/entries/static_file_entry.hpp"
#include "zipstream/crc32sum.hpp"

#include <algorithm>
#include <cstring>

namespace zipstream
{

static_file_entry::static_file_entry(std::string const & name, std::string const & value)
: m_name(name)
, m_value(value)
{

}

std::string const & static_file_entry::name() const
{
    return m_name;
}

uint32_t static_file_entry::size() const
{
    return m_value.size();
}

std::optional<uint32_t> static_file_entry::crc32() const
{
    return crc32sum::from_string(m_value);
}


size_t static_file_entry::read_at(size_t offset, char * buffer, size_t buffer_size)
{
    if (offset >= m_value.size())
    {
        return 0;
    }

    size_t const available = m_value.size() - offset;
    size_t const count = std::min(available, buffer_size);

    memcpy(buffer, &m_value.data()[offset], count);

    return count;
}
 
}