#include "zipstream/builder.hpp"
#include "zipstream/entry.hpp"
#include "zipstream/stream.hpp"

#include "zipstream/entries/dir_entry.hpp"
#include "zipstream/entries/static_file_entry.hpp"
#include "zipstream/entries/file_entry.hpp"

#include <vector>
#include <filesystem>

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
    e.inner_entry = std::make_unique<dir_entry>(name);
    d->entries.emplace_back(std::move(e));

    return *this;
}

builder& builder::add_file_with_content(std::string const & name, std::string const & content)
{
    entry e;
    e.inner_entry = std::make_unique<static_file_entry>(name, content);
    d->entries.emplace_back(std::move(e));

    return *this;
}

builder& builder::add_file_from_path(std::string const & name, std::string const & path)
{
    entry e;
    e.inner_entry = std::make_unique<file_entry>(name, path);
    d->entries.emplace_back(std::move(e));

    return *this;
}

std::unique_ptr<stream_i> builder::build()
{
    return std::unique_ptr<stream_i>(new stream(std::move(d->entries)));
}



}