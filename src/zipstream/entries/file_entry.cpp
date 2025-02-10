#include "zipstream/entries/file_entry.hpp"
#include "zipstream/crc32sum.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>

namespace zipstream
{

file_entry::file_entry(std::string const & name, std::string const & path)
: m_name(name)
, m_path(path)
{

}

std::string const & file_entry::name() const
{
    return m_name;
}

uint32_t file_entry::size() const
{
    return std::filesystem::file_size(m_path);
}

std::optional<uint32_t> file_entry::crc32() const
{
    return std::nullopt;
}

size_t file_entry::read_at(size_t offset, char * buffer, size_t buffer_size)
{
    std::ifstream file(m_path);
    file.seekg(offset);
    file.read(buffer, buffer_size);
    auto const count = file.gcount();

    if (file.bad())
    {
        throw std::runtime_error("failed to read file");
    }

    return count;
}


}