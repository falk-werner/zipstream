cmake_minimum_required(VERSION 3.20)
project(zipstream)

set(CMAKE_CXX_STANDARD 17)

add_library(zipstream STATIC
    src/zipstream/builder.cpp
    src/zipstream/crc32sum.cpp
    src/zipstream/stream.cpp
    src/zipstream/buffer.cpp)
target_include_directories(zipstream PUBLIC inc)
target_include_directories(zipstream PRIVATE src)

add_executable(zipper
    example/main.cpp)
target_link_libraries(zipper PRIVATE zipstream)

add_executable(read_zip
    tools/read_zip.cpp)


enable_testing()
include(CTest)

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(alltests
    test-src/test_crc32sum.cpp
    test-src/test_buffer.cpp)
target_include_directories(alltests PRIVATE src)

target_link_libraries(alltests PRIVATE zipstream GTest::gtest GTest::gtest_main)
gtest_discover_tests(alltests)