# zipstream

C++ library to create ZIP archives on the fly.

ZIP files can be created in a single pass without storing them on disk. This project contains a C++ library that to provide a stream oriented pull API to create zip archives.

## Usage

```C++
#include <zipstream>

int main(int argc, char * argv[])
{

    zipstream::builder builder;

    builder.add_file_with_content("foo.txt", "foo");
    builder.add_file_from_path("bar.txt" "path/to/bar.txt");
 
    auto stream = builder.build();

    constexpr size_t const buffer_size = 10 * 1024;
    char buffer[buffer_size];
    size_t count = stream.read(buffer, buffer_size);
    while (count > 0)
    {
        // consume chunk of zip archive
        count = stream.read(buffer, buffer_size);
    }
}
```

## Builder API

| Method | Arguments | Description |
| ------ | --------- | ----------- |
| add_directory | name: str | Adds a directory to the archive |
| add_file_with_content | name: str, contents: str | Add a static file with the given name and contents |
| add_file_from_path | name: str, path: str | Adds the file specifed by path with the given name |

### Notice

Any file referenced by the builder must not be changed on the filesystem
until the archive is completely read. Changes of files are not really
tracked by the library an result either in exceptions (if the file is
deleted or truncated) or in corrupt archives (if the file is modified).

## Missing Features

- install library using `cmake install`
- use correct file and directory attributes
- use corrent file date and time
- compress files using deflate
- create zip64 archives
- add more unit tests
- add options to opt out creating unit tests


## References

- [ZIP file format specification](https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT)
- [Description of external file attributes (Unix)](https://unix.stackexchange.com/questions/14705/the-zip-formats-external-file-attribute)
- [CRC32](https://crccalc.com/?crc=42&method=CRC-32/ISO-HDLC)
