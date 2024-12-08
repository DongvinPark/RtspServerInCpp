#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <functional> // For std::function
#include "C.h"

class Buffer {
public:
    // used unsigned char for buf element type for ease of initialization of buf
    // when reading raw binary data from ifstream
    std::vector<unsigned char> buf;
    int len;
    int offset;
    int limit;
    int bodyLen = C::UNSET;
    int sampleNo = C::UNSET;
    int mediaType = C::UNSET;
    std::function<void()> afterTx;

    explicit Buffer(const std::vector<unsigned char>& buf);

    explicit Buffer(const std::vector<unsigned char>& buf, const int offset, const int len);

    static Buffer kill();

    // used 'const' to repesent this return data cannot be updated after returing.
    const std::string getString();

    const std::string toString();

private:
    void validateBuffer();
};

#endif // BUFFER_H