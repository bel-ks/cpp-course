cmake_minimum_required(VERSION 3.12)
project(huffman)

set(CMAKE_CXX_STANDARD 14)

add_library(
        huffman_common
        huffman.cpp
        huffman.h
        huf_tree.cpp
        huf_tree.h
        buffer_reader.cpp
        buffer_reader.h
        buffer_writer.cpp
        buffer_writer.h)

add_executable(
        huffman
        main.cpp)

add_executable(
        huffman_testing
        huffman_testing.cpp
        gtest/gtest.h
        gtest/gtest-all.cc
        gtest/gtest-main.cc)

target_link_libraries(huffman huffman_common)
target_link_libraries(huffman_testing huffman_common -lpthread)