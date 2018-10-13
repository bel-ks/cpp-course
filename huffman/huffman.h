#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "buffer_reader.h"
#include "buffer_writer.h"
#include "huf_tree.h"

struct huffman {
    static void encode(std::istream &in, std::ostream &out);
    static bool decode(std::istream &in, std::ostream &out);

    static std::vector<std::pair<uint64_t, uint8_t> > getCodes(huf_tree::Node* const& root);
};

#endif //HUFFMAN_H
