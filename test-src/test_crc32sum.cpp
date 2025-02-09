#include "zipstream/crc32sum.hpp"
#include <gtest/gtest.h>

TEST(crc32sum, from_string)
{
    ASSERT_EQ(0x00000000, zipstream::crc32sum::from_string(""));
    ASSERT_EQ(0x3224b088, zipstream::crc32sum::from_string("42"));
}
