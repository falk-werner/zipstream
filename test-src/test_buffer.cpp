#include "zipstream/buffer.hpp"
#include <gtest/gtest.h>

TEST(buffer, u16)
{
    zipstream::buffer buf(6);
    buf.write_u16(0x3231);
    buf.write_u16(0x3433);
    buf.write_u16(0x3635);

    char out[7];
    out[6] = '\0';
    buf.read(out, 6);

    ASSERT_STREQ("123456", out);
}

TEST(buffer, u32)
{
    zipstream::buffer buf(8);
    buf.write_u32(0x34333231);
    buf.write_u32(0x38373635);

    char out[9];
    out[8] = '\0';
    buf.read(out, 8);

    ASSERT_STREQ("12345678", out);
}

TEST(buffer, str)
{
    zipstream::buffer buf(13);
    buf.write_str("Hello,");
    buf.write_str(" world!");

    char out[14];
    out[13] = '\0';
    buf.read(out, 13);

    ASSERT_STREQ("Hello, world!", out);
}

TEST(buffer, read_multi)
{
    zipstream::buffer buf(32);
    buf.write_u16(0x3231);
    buf.write_u32(0x34333231);
    buf.write_str("Hello");

    char out[6];
    out[2] = '\0';
    buf.read(out, 2);
    ASSERT_STREQ("12", out);

    out[4] = '\0';
    buf.read(out, 4);
    ASSERT_STREQ("1234", out);

    out[5] = '\0';
    buf.read(out, 5);
    ASSERT_STREQ("Hello", out);
}

TEST(buffer, throw_on_overflow_u16)
{
    zipstream::buffer buf(1);
    ASSERT_ANY_THROW({
        buf.write_u16(0xbad);
    });
}

TEST(buffer, throw_on_overflow_u32)
{
    zipstream::buffer buf(1);
    ASSERT_ANY_THROW({
        buf.write_u32(0xbad);
    });
}

TEST(buffer, throw_on_overflow_str)
{
    zipstream::buffer buf(1);
    ASSERT_ANY_THROW({
        buf.write_str("bad");
    });
}

TEST(buffer, reset_after_exhaustive_read)
{
    zipstream::buffer buf(2);
    char out[3] = {0,0,0};

    buf.write_u16(0x3231);
    buf.read(out, 2);
    ASSERT_STREQ("12", out);

    buf.write_u16(0x3433);
    buf.read(out, 2);
    ASSERT_STREQ("34", out);
    
    buf.write_u16(0x3635);
    buf.read(out, 2);
    ASSERT_STREQ("56", out);
}

