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
    write_local_header,
    write_local_data,
    write_central_header,
    write_central_end,
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
