#ifndef ZIPSTREAM_STREAM_I_HPP
#define ZIPSTREAM_STREAM_I_HPP

#include <string>

namespace zipstream
{

class stream_i
{
public:
    virtual ~stream_i() = default;
    virtual void write_to_file(std::string const & path) = 0;
    virtual size_t read(char * buffer, size_t buffer_size) = 0;
    virtual void skip(size_t count) = 0;
    virtual void reset() = 0;
};

}

#endif
