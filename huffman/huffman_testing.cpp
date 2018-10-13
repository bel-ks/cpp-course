#include <iostream>
#include "gtest/gtest.h"
#include "huffman.h"

TEST(correctness, empty_file)
{
    std::stringstream input("");
    std::stringstream en;
    std::stringstream de;
    huffman::encode(input, en);
    huffman::decode(en, de);

    EXPECT_EQ(de.str(), input.str());
}

TEST(correctness, one_symbol)
{
    std::stringstream input("&");
    std::stringstream en;
    std::stringstream de;
    huffman::encode(input, en);
    huffman::decode(en, de);

    EXPECT_EQ(de.str(), input.str());
}

TEST(correctness, all_symbols)
{
    std::stringstream input;
    for (auto s = -128; s <= 127; ++s) {
        input << (char)s;
    }
    std::stringstream en;
    std::stringstream de;
    huffman::encode(input, en);
    huffman::decode(en, de);

    EXPECT_EQ(de.str(), input.str());
}

TEST(correctness, invalid_file)
{
    std::stringstream en;
    en << "876534567";
    std::stringstream de;

    EXPECT_EQ(huffman::decode(en, de), false);
}

TEST(correctness, average_file)
{
    auto len = ((size_t)2048 * 2048) % (size_t)rand() % (size_t)65536;
    std::stringstream input("");
    for (size_t ind = 0; ind < len; ++ind) {
        input << (char)(rand() % 256);
    }
    std::stringstream en;
    std::stringstream de;
    huffman::encode(input, en);
    huffman::decode(en, de);

    EXPECT_EQ(de.str(), input.str());
}

TEST(correctness, large_file)
{
    auto len = ((size_t)2048 * 2048) % (size_t)rand() + (size_t)65536;
    std::stringstream input("");
    for (size_t ind = 0; ind < len; ++ind) {
        input << (char)(rand() % 256);
    }
    std::stringstream en;
    std::stringstream de;
    huffman::encode(input, en);
    huffman::decode(en, de);

    EXPECT_EQ(de.str(), input.str());
}

TEST(correctness, twice_encoded)
{
    auto len = ((size_t)2048 * 2048) % (size_t)rand() + (size_t)65536;
    std::stringstream input("");
    for (size_t ind = 0; ind < len; ++ind) {
        input << (char)(rand() % 256);
    }
    std::stringstream en1, en2;
    std::stringstream de1, de2;
    huffman::encode(input, en1);
    huffman::encode(en1, en2);
    huffman::decode(en2, de2);
    huffman::decode(de2, de1);

    EXPECT_EQ(de1.str(), input.str());
}