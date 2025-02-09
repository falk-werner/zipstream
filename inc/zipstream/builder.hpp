#ifndef ZIPSTREAM_BUILDER_HPP
#define ZIPSTREAM_BUILDER_HPP

#include <zipstream/stream_i.hpp>

#include <string>
#include <memory>

namespace zipstream
{

class builder
{
    builder(builder const &) = delete;
    builder& operator=(builder const &) = delete;
public:
    builder();
    ~builder();
    builder(builder && other);
    builder& operator=(builder && other);
    builder& add_directory(std::string const & name);
    builder& add_file_with_content(std::string const & name, std::string const & content);
    builder& add_file_from_path(std::string const & name, std::string const & path);
    std::unique_ptr<stream_i> build();
private:
    class detail;
    detail *d;
};


}

#endif
