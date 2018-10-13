#ifndef BUFFER_WRITER_H
#define BUFFER_WRITER_H

#include <ostream>
#include <cstdint>
#include <string.h>

struct buffer_writer {
    buffer_writer(std::ostream &out);
    ~buffer_writer();

    void writeChar(uint8_t const& c);
    void writeCnt(uint64_t const &c);

private:
    static const size_t buffSize = 65536;
    uint8_t buff[buffSize];
    size_t pos;
    std::ostream &out;

    void clear();
};

#endif //BUFFER_WRITER_H
