#ifndef BUFFER_READER_H
#define BUFFER_READER_H

#include <istream>
#include <string.h>

struct buffer_reader {
    buffer_reader(std::istream &in);

    void reset();
    bool readChar(uint8_t& c);
    bool readCnt(uint64_t &c);

private:
    static const size_t buffSize = 65536;
    uint8_t buff[buffSize];
    std::istream &in;
    size_t curSize;
    size_t pos;

    void readBuff();
};

#endif //BUFFER_READER_H