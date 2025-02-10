#include <zipstream/zipstream.hpp>

int main(int argc, char* argv[])
{
    zipstream::builder builder;
    builder.add_file_with_content("foo.txt", "foo");
    builder.add_directory("a/");
    builder.add_file_with_content("a/foo.txt", "foo");
    builder.add_file_from_path("bar.txt", "CMakeLists.txt");

    auto stream = builder.build();
    stream->write_to_file("foo.zip");
    
    return 0;
}