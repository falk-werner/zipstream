#include "zipstream/stream.hpp"
#include <zipstream/crc32sum.hpp>

#include <cstring>

#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <algorithm>

namespace zipstream
{

namespace
{

constexpr size_t const buffer_size = 100 * 1024;

uint32_t compute_checksum(entry const &e)
{
    switch (e.type)
    {
        case entry_type::directory:
            return 0;
        case entry_type::file_with_content:
            return crc32sum::from_string(e.value);
        case entry_type::file_from_path:
            return crc32sum::from_file(e.value);
        default:
            throw std::runtime_error("internal error: unknown entry type");
    }
}

uint32_t get_size(entry const &e)
{
    switch (e.type)
    {
        case entry_type::directory:
            return 0;
        case entry_type::file_with_content:
            return e.value.size();
        case entry_type::file_from_path:
            return std::filesystem::file_size(e.value);
        default:
            throw std::runtime_error("internal error: unknown entry type");
    }
}


}

stream::stream(std::vector<entry> && entries)
: m_entries(std::move(entries))
, m_buffer(buffer_size)
, m_pos(0)
, m_state(state::init)
, m_current_entry(0)
, m_data_pos(0)
{

}

void stream::write_to_file(std::string const & path)
{
    std::ofstream file(path, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    char buffer[buffer_size];

    reset();
    size_t count = read(buffer, buffer_size);
    while (count > 0)
    {
        file.write(buffer, count);
        count = read(buffer, buffer_size);
    }
}

size_t stream::read(char * buffer, size_t buffer_size)
{
    size_t pos = 0;
    while ((m_state != state::done) && (pos < buffer_size))
    {
        switch (m_state)
        {
            case state::init:
                process_init();
                break;
            case state::file_header:
                process_file_header(buffer, buffer_size, pos);
                break;
            case state::file_data:
                process_file_data(buffer, buffer_size, pos);
                break;
            case state::data_descriptor:
                process_data_descriptor(buffer, buffer_size, pos);
                break;
            case state::toc_entry:
                process_toc_entry(buffer, buffer_size, pos);
                break;
            case state::toc_end:
                process_toc_end(buffer, buffer_size, pos);
                break;
            case state::done:
                // fall-through
            default:
                throw std::runtime_error("invalid state");
        }
    }

    return pos;
}

void stream::skip(size_t count)
{
    char buffer[buffer_size];

    size_t remaining = count;
    while (remaining > 0)
    {
        size_t const chunk_size = std::min(remaining, buffer_size);
        size_t const bytes_read = read(buffer, chunk_size);
        if (bytes_read == 0)
        {
            // end of stream
            break;
        }

        remaining -= bytes_read;
    }

    throw std::runtime_error("not implemented");
}

void stream::reset()
{
    m_state = state::init;
    m_buffer.reset();
    m_current_entry = 0;
    m_data_pos = 0;
    m_toc_start = 0;
}

void stream::process_init()
{
    m_buffer.reset();
    m_pos = 0;
    m_state = state::file_header;
    m_current_entry = 0;
    m_data_pos = 0;
    m_toc_start = 0;
}

void stream::process_file_header(char * buffer, size_t buffer_size, size_t & pos)
{
    if (m_current_entry == m_entries.size())
    {
        m_buffer.reset();
        m_state = state::toc_entry;
        m_current_entry = 0;
        m_data_pos = 0;
        m_toc_start = m_pos;
        return;
    }

    if (m_buffer.empty())
    {
        auto & entry = m_entries.at(m_current_entry);
        entry.offset = m_pos;
        entry.crc32 = compute_checksum(entry);
        entry.size = get_size(entry);

        m_buffer.write_u32(0x04034b50);        // signatue
        m_buffer.write_u16(10);                // version needed (default=1.0)
        m_buffer.write_u16(0);                 // flags (none)
        m_buffer.write_u16(0);                 // compression method (store)
        m_buffer.write_u16(0);                 // ToDo: file time
        m_buffer.write_u16(0);                 // ToDo: file data
        m_buffer.write_u32(entry.crc32);       // crc32
        m_buffer.write_u32(entry.size);        // compressesd size
        m_buffer.write_u32(entry.size);        // uncompressed size
        m_buffer.write_u16(entry.name.size()); // filename length
        m_buffer.write_u16(0);                 // extra field length
        m_buffer.write_str(entry.name);        // filename                                        
    }

    size_t const count = m_buffer.read(&buffer[pos], buffer_size - pos);
    pos += count;
    m_pos += count;

    if (m_buffer.empty())
    {
        m_buffer.reset();
        m_state = state::file_data;
        m_data_pos = 0;
    }
}

void stream::process_file_data(char * buffer, size_t buffer_size, size_t & pos)
{
    auto & entry = m_entries.at(m_current_entry);
    switch (entry.type)
    {
        case entry_type::directory:
            // no_content -> next entry
            m_current_entry++;
            m_state = state::file_header;
            break;
        case entry_type::file_with_content:
            {
                size_t const remaining = entry.size - m_data_pos;
                size_t const count = std::min(remaining, buffer_size - pos);

                if (count > 0)
                {
                    memcpy(&buffer[pos], &entry.value.data()[m_data_pos], count);

                    pos += count;
                    m_pos += count;
                    m_data_pos += count;
                }

                if (entry.size == m_data_pos)
                {
                    m_current_entry++;
                    m_state = state::file_header;
                    m_data_pos = 0;
                }
            }
            break;
        case entry_type::file_from_path:
            {
                size_t const remaining = entry.size - m_data_pos;
                size_t const count = std::min(remaining, buffer_size - pos);

                if (count > 0)
                {
                    std::ifstream file(entry.name);
                    file.seekg(m_data_pos);
                    file.read(&buffer[pos], count);

                    pos += count;
                    m_pos += count;
                    m_data_pos += count;

                    if (m_data_pos != file.tellg())
                    {
                        throw std::runtime_error("failed to read file");
                    }
                }

                if (entry.size == m_data_pos)
                {
                    m_current_entry++;
                    m_state = state::file_header;
                    m_data_pos = 0;
                }
            }
            break;
        default:
            throw std::runtime_error("invalid entry type");
    }
}

void stream::process_data_descriptor(char * buffer, size_t buffer_size, size_t & pos)
{

}

void stream::process_toc_entry(char * buffer, size_t buffer_size, size_t & pos)
{
    if (m_current_entry >= m_entries.size())
    {
        m_buffer.reset();
        m_state = state::toc_end;
        m_current_entry = 0;
        return;                        
    }

    if (m_buffer.empty())
    {
        auto const & entry = m_entries.at(m_current_entry);
        m_buffer.write_u32(0x02014b50);        // central file header signature
        m_buffer.write_u16(0x031e);            // version made by (unix=3, 30 [same as zip utility])
        m_buffer.write_u16(10);                // version needed to extract (default=1.0)
        m_buffer.write_u16(0);                 // flags (none)
        m_buffer.write_u16(0);                 // compression method (store)
        m_buffer.write_u16(0);                 // ToDo: last mod file time
        m_buffer.write_u16(0);                 // ToDo: last mod file date
        m_buffer.write_u32(entry.crc32);       // crc32
        m_buffer.write_u32(entry.size);        // compressed size
        m_buffer.write_u32(entry.size);        // uncompressed size
        m_buffer.write_u16(entry.name.size()); // filename length
        m_buffer.write_u16(0);                 // entry length
        m_buffer.write_u16(0);                 // comment length
        m_buffer.write_u16(0);                 // disk number start
        m_buffer.write_u16(0);                 // internal attributes (none)
        m_buffer.write_u32(0x81b40000);        // ToDo: external attributes (reg file)
        m_buffer.write_u32(entry.offset);      // offset of local file header
        m_buffer.write_str(entry.name);                
    }

    size_t const count = m_buffer.read(&buffer[pos], buffer_size - pos);
    pos += count;
    m_pos += count;

    if (m_buffer.empty())
    {
        m_buffer.reset();
        m_current_entry++;
    }
}

void stream::process_toc_end(char * buffer, size_t buffer_size, size_t & pos)
{
    if (m_buffer.empty())
    {
        size_t const toc_end = m_pos;
        size_t const toc_size = toc_end - m_toc_start;
        m_buffer.write_u32(0x06054b50);         // end of central directory record signature
        m_buffer.write_u16(0);                  // number of this disk
        m_buffer.write_u16(0);                  // number of disk with start of eocd
        m_buffer.write_u16(m_entries.size());   // number of entries in this disk
        m_buffer.write_u16(m_entries.size());   // total number of entries
        m_buffer.write_u32(toc_size);           // size of central directory
        m_buffer.write_u32(m_toc_start);        // start of central directory
        m_buffer.write_u16(0);                  // comment length
    }

    size_t const count = m_buffer.read(&buffer[pos], buffer_size - pos);
    pos += count;
    m_pos += count;

    if (m_buffer.empty())
    {
        m_state = state::done;
    }
}


}