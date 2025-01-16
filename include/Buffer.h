#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <functional> // For std::function
#include "../constants/C.h"

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

    explicit Buffer();

    explicit Buffer(const std::vector<unsigned char>& buf);

    explicit Buffer(const std::vector<unsigned char>& buf, const int offset, const int len);

    void updateBuf(const std::vector<unsigned char>& inputBuf);

    static Buffer kill();

    // used 'const' to represent this return data cannot be updated after returing.
    const std::string getString();

    const std::string toString();

private:
    void validateBuffer();
};

#endif // BUFFER_H