#include "buffer_reader.h"

void buffer_reader::readBuff() {
    in.read((char *)buff, buffSize);
    curSize = (size_t)in.gcount();
    pos = 0;
}

buffer_reader::buffer_reader(std::istream &in): in(in) {
    readBuff();
}

void buffer_reader::reset() {
    in.clear();
    in.seekg(0);
}

bool buffer_reader::readChar(uint8_t &c) {
    if (pos >= curSize) {
        readBuff();
    }
    if (pos < curSize) {
        c = (uint8_t)buff[pos];
        ++pos;
        return true;
    }
    return false;
}

bool buffer_reader::readCnt(uint64_t &c) {
    if (pos + sizeof(c) > curSize) {
        auto d = curSize - pos;
        memcpy(&c, buff + pos, d);
        readBuff();
        uint64_t t = 0;
        d = sizeof(uint64_t) - d;
        memcpy(&t, buff + pos, d);
        t <<= ((sizeof(uint64_t) - d) * 8);
        c += t;
        pos += d;
        return true;
    }
    if (pos + sizeof(c) <= curSize) {
        memcpy(&c, buff + pos, sizeof(c));
        pos += sizeof(c);
        return true;
    }
    return false;
}