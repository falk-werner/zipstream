#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdexcept>

#include <cinttypes>

namespace
{

struct end_of_central_directory
{
    uint32_t signature;
    uint16_t disk_number;
    uint16_t disk_with_start_of_central_directory;
    uint16_t entries_on_this_disk;
    uint16_t total_entries;
    uint32_t size;
    uint32_t offset;
    std::string comment;
};

class mmap_file
{
    mmap_file(mmap_file const &) = delete;
    mmap_file& operator=(mmap_file const &) = delete;
public:
    mmap_file(std::string const & path)
    {
        fd = open(path.c_str(), O_RDONLY);
        if (fd < 0)
        {
            throw std::runtime_error("failed to open file");
        }

        struct stat info;
        int const rc = fstat(fd, &info);
        if (rc != 0)
        {
            close(fd);
            throw std::runtime_error("failed to stat file");
        }
        size = info.st_size;

        address = reinterpret_cast<uint8_t*>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
        if (address == ((void*) -1))
        {
            close(fd);
            throw std::runtime_error("failed to mmap file");
        }

    }

    ~mmap_file()
    {
        if (fd > 0)
        {
            munmap(address, size);
            close(fd);
        }
    }

    mmap_file(mmap_file && other)
    {
        if (this != &other)
        {
            this->fd = other.fd;
            this->address = other.address;
            this->size = other.size;

            other.fd = -1;
            other.address = nullptr;
            other.size = 0;
        }
    }

    mmap_file& operator=(mmap_file && other)
    {
        if (this != &other)
        {
                if (fd > 0)
                {
                    munmap(address, size);
                    close(fd);
                }
            
            this->fd = other.fd;
            this->address = other.address;
            this->size = other.size;

            other.fd = -1;
            other.address = nullptr;
            other.size = 0;
        }

        return *this;
    }

    uint8_t const * get_address() const
    {
        return address;
    }

    uint32_t read_u32(size_t offset) const
    {
        if ((offset + 4) > size)
        {
            throw std::runtime_error("read out of bounds");
        }

        uint32_t value = 0;
        for(size_t i = 0; i < 4; i++)
        {
            uint8_t c = address[offset + 3 - i];
            value <<= 8;
            value |= c;
        }

        return value;
    }

    uint16_t read_u16(size_t offset) const
    {
        if ((offset + 2) > size)
        {
            throw std::runtime_error("read out of bounds");
        }

        uint8_t const low = address[offset];
        uint8_t const high = address[offset + 1];

        return (high << 8) | low;
    }

    std::string read_string(size_t offset, size_t length)
    {
        if ((offset + length) > size)
        {
            throw std::runtime_error("read out of bounds");
        }

        return std::string(reinterpret_cast<char const*>(&address[offset]), length);
    }

    size_t get_size() const
    {
        return size;
    }

private:
    int fd;
    size_t size;
    uint8_t * address;
};

size_t find_end_of_central_directory(mmap_file& zip)
{
    uint8_t const * file = zip.get_address();
    for(size_t i = zip.get_size() - 22; i >= 4; i--)
    {
        uint32_t const signature = zip.read_u32(i);
        if (signature == 0x06054b50)
        {
            return i;
        }
    }

    throw std::runtime_error("invalid file format: end of central directory not found");
}

void read_end_of_central_directory(mmap_file& zip, size_t offset, end_of_central_directory & eocd)
{
    eocd.signature = zip.read_u32(offset);
    eocd.disk_number = zip.read_u16(offset + 4);
    eocd.disk_with_start_of_central_directory = zip.read_u16(offset + 6);
    eocd.entries_on_this_disk = zip.read_u16(offset + 8);
    eocd.total_entries = zip.read_u16(offset + 10);
    eocd.size = zip.read_u32(offset +12);
    eocd.offset = zip.read_u32(offset + 16);
    
    uint16_t const comment_length = zip.read_u16(offset + 20);
    if (comment_length == 0)
    {
        eocd.comment = "";
    }
    else
    {
        eocd.comment = zip.read_string(offset + 22, comment_length);
    }
}

// central file header

constexpr uint32_t const cfh_signature = 0x02014b50;
constexpr size_t const cfh_signature_offset = 0;
constexpr size_t const cfh_version_made_by_offset = 4;
constexpr size_t const cfh_version_needed_offset = 6;
constexpr size_t const cfh_flags_offset = 8;
constexpr size_t const cfh_compression_method_offset = 10;
constexpr size_t const cfh_last_mod_time_offset = 12;
constexpr size_t const cfh_last_mod_date_offset = 14;
constexpr size_t const cfh_checksum_offset = 16;
constexpr size_t const cfh_compressed_size_offset = 20;
constexpr size_t const cfh_uncompressed_size_offset = 24;
constexpr size_t const cfh_filename_length_offset = 28;
constexpr size_t const cfh_extra_field_length_offset = 30;
constexpr size_t const cfh_comment_offset = 32;
constexpr size_t const cfh_start_disk_offset = 34;
constexpr size_t const cfh_internal_attributes_offset = 36;
constexpr size_t const cfh_external_attributes_offset = 38;
constexpr size_t const cfh_local_header_offset_offset = 42;
constexpr size_t const cfh_filename_offset = 46;
constexpr size_t const cfh_static_size = 46;

struct central_file_header
{
    uint32_t signature;
    uint16_t version_made_by;
    uint16_t version_needed_to_extract;
    uint16_t general_purpose_flag;
    uint16_t compression_method;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint32_t checksum;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t disk_number_start;
    uint16_t internal_file_attributes;
    uint32_t external_file_attributes;
    uint32_t offset_of_local_header;

    std::string filename;
    std::string extra;
    std::string comment;
    size_t size;

    void parse(mmap_file & zip, size_t offset)
    {
        signature = zip.read_u32(offset + cfh_signature_offset);
        if (signature != cfh_signature)
        {
            throw std::runtime_error("invalid central file header signature");
        }

        version_made_by = zip.read_u16(offset + cfh_version_made_by_offset);
        version_needed_to_extract = zip.read_u16(offset + cfh_version_needed_offset);
        general_purpose_flag = zip.read_u16(offset + cfh_flags_offset);
        compression_method = zip.read_u16(offset + cfh_compression_method_offset);
        last_mod_time = zip.read_u16(offset + cfh_last_mod_time_offset);
        last_mod_date = zip.read_u16(offset + cfh_last_mod_date_offset);
        checksum = zip.read_u32(offset + cfh_checksum_offset);
        compressed_size = zip.read_u32(offset + cfh_compressed_size_offset);
        uncompressed_size = zip.read_u32(offset + cfh_uncompressed_size_offset);
        uint16_t const filename_length = zip.read_u16(offset + cfh_filename_length_offset);
        uint16_t const extra_length = zip.read_u16(offset + cfh_extra_field_length_offset);
        uint16_t const comment_length = zip.read_u16(offset + cfh_comment_offset);
        internal_file_attributes = zip.read_u16(offset + cfh_internal_attributes_offset);
        external_file_attributes = zip.read_u32(offset + cfh_external_attributes_offset);
        offset_of_local_header = zip.read_u32(offset + cfh_local_header_offset_offset);
        filename = zip.read_string(offset + cfh_filename_offset, filename_length);
        size_t const extra_field_offset = cfh_static_size + filename_length;
        extra = zip.read_string(offset + extra_field_offset, extra_length);
        size_t const comment_offset = extra_field_offset + extra_length;
        comment = zip.read_string(offset + comment_offset, comment_length);
        size = cfh_static_size + filename_length + extra_length + comment_length;
    }
};

// local file header

constexpr uint32_t const lfh_signature = 0x04034b50;
constexpr size_t const lfh_signature_offset = 0;
constexpr size_t const lfh_version_needed_offset = 4;
constexpr size_t const lfh_flags_offset = 6;
constexpr size_t const lfh_compression_method_offset = 8;
constexpr size_t const lfh_last_mod_time_offset = 10;
constexpr size_t const lfh_last_mod_date_offset = 12;
constexpr size_t const lfh_checksum_offset = 14;
constexpr size_t const lfh_compressed_size_offset = 18;
constexpr size_t const lfh_uncompressed_size_offset = 22;
constexpr size_t const lfh_filename_length_offset = 26;
constexpr size_t const lfh_extra_length_offset = 28;
constexpr size_t const lfh_filename_offset = 30;
constexpr size_t const lfh_static_size = 30;

struct local_file_header
{
    uint32_t signature;
    uint16_t version_needed;
    uint16_t flags;
    uint16_t compression_method;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint32_t checksum;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    std::string filename;
    std::string extra;
    size_t data_offset;

    void parse(mmap_file &zip, size_t offset)
    {
        signature = zip.read_u32(offset + lfh_signature_offset);
        if (signature != lfh_signature)
        {
            throw std::runtime_error("invalid local file header signature");
        }

        version_needed = zip.read_u16(offset + lfh_version_needed_offset);
        flags = zip.read_u16(offset + lfh_flags_offset);
        compression_method = zip.read_u16(offset + lfh_compression_method_offset);
        last_mod_time = zip.read_u16(offset + lfh_last_mod_time_offset);
        last_mod_date = zip.read_u16(offset + lfh_last_mod_date_offset);
        checksum = zip.read_u32(offset + lfh_checksum_offset);
        compressed_size = zip.read_u32(offset + lfh_compressed_size_offset);
        uncompressed_size = zip.read_u32(offset + lfh_uncompressed_size_offset);

        uint16_t const filename_length = zip.read_u16(offset + lfh_filename_length_offset);
        filename = zip.read_string(offset + lfh_filename_offset, filename_length);

        uint16_t const extra_length = zip.read_u16(offset + lfh_extra_length_offset);
        size_t extra_offset = lfh_static_size + filename_length;
        extra = zip.read_string(offset + extra_offset, extra_length);

        data_offset = offset + lfh_static_size + filename_length + extra_length;
    }
};

}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        char const * filename = argv[1];
        mmap_file zip(filename);

        std::cout << "size: " << zip.get_size() << std::endl;
        std::cout << std::endl;

        auto eocd_offset = find_end_of_central_directory(zip);
        std::cout << "end of central directory: 0x" << std::hex << eocd_offset << std::endl;
        std::cout << std::endl;

        end_of_central_directory eocd;
        read_end_of_central_directory(zip, eocd_offset, eocd);
        std::cout << "end of central directory:" << std::endl;
        std::cout << "  signature: 0x" << std::hex << eocd.signature << std::endl;
        std::cout << "  number of this disk: " << std::dec << eocd.disk_number << std::endl;
        std::cout << "  #disk with start of central directory:" << std::dec << eocd.disk_with_start_of_central_directory << std::endl;
        std::cout << "  entries on this disk: " << std::dec << eocd.entries_on_this_disk << std::endl;
        std::cout << "  total entries:" << std::dec << eocd.total_entries << std::endl;
        std::cout << "  offset of start of central directory: 0x" << std::hex << eocd.offset << std::endl;
        std::cout << "  comment length: " << std::dec << eocd.comment.size() << std::endl;
        std::cout << "  comment: " << eocd.comment << std::endl;
        std::cout << std::endl;


        std::vector<central_file_header> toc;
        uint32_t offset = eocd.offset;
        for(size_t i = 0; i < eocd.total_entries; i++) 
        {
            central_file_header cfh;
            cfh.parse(zip, offset);
            offset += cfh.size;
            toc.push_back(cfh);
        }

        for(auto const & cfh: toc)
        {
            std::cout << "central file header:" << std::endl;
            std::cout << "  signature: 0x" << std::hex << cfh.signature << std::endl;
            std::cout << "  version made by: 0x" << std::hex << cfh.version_made_by << std::endl;
            std::cout << "  version needed to extract: " << std::dec << cfh.version_needed_to_extract << std::endl;
            std::cout << "  general purpose bit flag: 0x" << std::hex << cfh.general_purpose_flag << std::endl;
            std::cout << "  compression method: " << std::dec << cfh.compression_method << std::endl;
            std::cout << "  last mod file time: " << std::dec << cfh.last_mod_time << std::endl;
            std::cout << "  last mod file date: " << std::dec << cfh.last_mod_date << std::endl;
            std::cout << "  crc32: 0x" << std::hex << cfh.checksum << std::endl;
            std::cout << "  compressed size: " << std::dec << cfh.compressed_size << std::endl;
            std::cout << "  uncompressed size: " << std::dec << cfh.uncompressed_size << std::endl;
            std::cout << "  file name: " << cfh.filename << std::endl;
            std::cout << "  extra field length: " << std::dec << cfh.extra.size() << std::endl;
            std::cout << "  file comment: " << cfh.comment << std::endl;
            std::cout << "  disk number start: " << std::dec << cfh.disk_number_start << std::endl;
            std::cout << "  interal file attributes: 0x" << std::hex << cfh.internal_file_attributes << std::endl;
            std::cout << "  external file attributes: 0x" << std::hex << cfh.external_file_attributes << std::endl;
            std::cout << "  local header offset: 0x" << std::hex << cfh.offset_of_local_header << std::endl; 
            std::cout << std::endl;


            local_file_header lfh;
            lfh.parse(zip, cfh.offset_of_local_header);

            std::cout << "  signature: 0x" << std::hex << lfh.signature << std::endl;
            std::cout << "  version needed to extract: " << std::dec << lfh.version_needed << std::endl;
            std::cout << "  general purpose bit flag: 0x" << std::hex << lfh.flags << std::endl;
            std::cout << "  compression method: " << std::dec << lfh.compression_method << std::endl;
            std::cout << "  last mod file time: " << std::dec << lfh.last_mod_time << std::endl;
            std::cout << "  last mod file date: " << std::dec << lfh.last_mod_date << std::endl;
            std::cout << "  crc32: 0x" << std::hex << lfh.checksum << std::endl;
            std::cout << "  compressed size: " << std::dec << lfh.compressed_size << std::endl;
            std::cout << "  uncompressed size: " << std::dec << lfh.uncompressed_size << std::endl;
            std::cout << "  filename: " << lfh.filename << std::endl;
            std::cout << "  extra field size: " << lfh.extra.size() << std::endl;
            std::cout << std::endl;
        }

    }

    return 0;
}