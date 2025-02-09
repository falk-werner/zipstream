#include <zipstream/zipstream.hpp>

int main(int argc, char* argv[])
{
    zipstream::builder builder;
    builder.add_file_with_content("foo.txt", "foo");

    auto stream = builder.build();
    stream->write_to_file("foo.zip");
    
    return 0;
}