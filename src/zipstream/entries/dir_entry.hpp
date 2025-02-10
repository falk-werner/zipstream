#ifndef ZIPSTREAM_ENTRIES_DIR_ENTRY_HPP
#define ZIPSTREAM_ENTRIES_DIR_ENTRY_HPP

#include "zipstream/entry_i.hpp"

namespace zipstream
{

class dir_entry: public entry_i
{
public:
    explicit dir_entry(std::string const & name);
    ~dir_entry() override = default;
    std::string const & name() const override;
    uint32_t size() const override;
    std::optional<uint32_t> crc32() const override;
    size_t read_at(size_t offset, char * buffer, size_t buffer_size) override;    
private:
    std::string const m_name;
};

}


#endif
