#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <functional> // For std::function
#include "C.h"

class Buffer {
public:
    std::vector<unsigned char> buf;
    int len;
    int offset;
    int limit;
    int bodyLen = C::UNSET;
    int sampleNo = C::UNSET;
    int mediaType = C::UNSET;
    std::function<void()> afterTx;

    Buffer(const std::vector<unsigned char>& buf);

    Buffer(const std::vector<unsigned char>& buf, const int offset, const int len); 

    static Buffer kill();

    // used 'const' to repesent this return data cannot be updated after returing.
    std::string getString() const;

    std::string toString() const;

private:
    void validateBuffer() const;
};

#endif // BUFFER_H