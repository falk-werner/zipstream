#ifndef ZIPSTREAM_ENTRY_TYPE_HPP
#define ZIPSTREAM_ENTRY_TYPE_HPP

namespace zipstream
{

enum class entry_type
{
    directory,
    file_with_content,
    file_from_path
};

}

#endif
