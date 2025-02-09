#ifndef ZIPSTREAM_STREAM_HPP
#define ZIPSTREAM_STREAM_HPP

#include "zipstream/entry.hpp"
#include "zipstream/stream_i.hpp"
#include "zipstream/buffer.hpp"

#include <vector>

namespace zipstream
{

enum class state
{
    init,
    file_header,
    file_data,
    data_descriptor,
    toc_entry,
    toc_end,
    done
};

class stream: public stream_i
{
public:
    explicit stream(std::vector<entry> && entries);
    ~stream() override = default;
    void write_to_file(std::string const & path) override;
    size_t read(char * buffer, size_t buffer_size) override;
    void skip(size_t count) override;
    void reset() override;

private:
    void process_init();
    void process_file_header(char * buffer, size_t buffer_size, size_t & pos);
    void process_file_data(char * buffer, size_t buffer_size, size_t & pos);
    void process_data_descriptor(char * buffer, size_t buffer_size, size_t & pos);
    void process_toc_entry(char * buffer, size_t buffer_size, size_t & pos);
    void process_toc_end(char * buffer, size_t buffer_size, size_t & pos);

    std::vector<entry> m_entries;

    buffer m_buffer;
    size_t m_pos;
    state m_state;
    size_t m_current_entry;
    size_t m_data_pos;
    size_t m_toc_start;

};

}

#endif
