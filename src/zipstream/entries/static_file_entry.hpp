#ifndef ZIPSTREAM_ENTRIES_STATIC_FILE_ENTRY
#define ZIPSTREAM_ENTRIES_STATIC_FILE_ENTRY

#include "zipstream/entry_i.hpp"

namespace zipstream
{

class static_file_entry: public entry_i
{
public:
    static_file_entry(std::string const & name, std::string const & value);
    ~static_file_entry() override = default;
    std::string const & name() const override;
    uint32_t size() const override;
    std::optional<uint32_t> crc32() const override;
    size_t read_at(size_t offset, char * buffer, size_t buffer_size) override;
private:
    std::string const m_name;
    std::string const m_value;
};

}

#endif
