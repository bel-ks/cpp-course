#include "buffer_writer.h"

void buffer_writer::clear() {
    out.write((char *)buff, pos);
    pos = 0;
}

buffer_writer::buffer_writer(std::ostream &out): pos(0), out(out) {}

buffer_writer::~buffer_writer() {
    clear();
}

void buffer_writer::writeChar(uint8_t const &c) {
    if (pos >= buffSize) {
        clear();
    }
    buff[pos] = c;
    ++pos;
}

void buffer_writer::writeCnt(uint64_t const &c) {
    if (pos + sizeof(c) > buffSize) {
        clear();
    }
    memcpy(buff + pos, &c, sizeof(c));
    pos += sizeof(c);
}