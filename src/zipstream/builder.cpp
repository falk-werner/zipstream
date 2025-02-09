#include "zipstream/builder.hpp"
#include "zipstream/entry.hpp"
#include "zipstream/stream.hpp"

#include <vector>

namespace zipstream
{

class builder::detail
{
public:
    std::vector<entry> entries;
};


builder::builder()
: d(new detail())
{
}

builder::~builder()
{
    delete d;
}

builder::builder(builder && other)
{
    if (this != &other)
    {
        this->d = other.d;
        other.d = nullptr;
    }
}

builder& builder::operator=(builder && other)
{
    if (this != &other)
    {
        delete d;
        this->d = other.d;
        other.d = nullptr;
    }

    return *this;
}

builder& builder::add_directory(std::string const & name)
{
    entry e;
    e.type = entry_type::directory;
    e.name = name;

    d->entries.push_back(e);
    return *this;
}

builder& builder::add_file_with_content(std::string const & name, std::string const & content)
{
    entry e;
    e.type = entry_type::file_with_content;
    e.name = name;
    e.value = content;

    d->entries.push_back(e);
    return *this;
}

builder& builder::add_file_from_path(std::string const & name, std::string const & path)
{
    entry e;
    e.type = entry_type::file_from_path;
    e.name = name;
    e.value = path;

    d->entries.push_back(e);
    return *this;
}

std::unique_ptr<stream_i> builder::build()
{
    return std::unique_ptr<stream_i>(new stream(std::move(d->entries)));
}



}