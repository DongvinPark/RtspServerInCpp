#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <cstddef>  // For std::byte
#include <functional> // For std::function
#include "C.h"

class Buffer {
public:
    std::vector<std::byte> buf;
    int len;
    int offset;
    int limit;
    int bodyLen = C::UNSET;
    int sampleNo = C::UNSET;
    int mediaType = C::UNSET;
    std::function<void()> afterTx;

    Buffer(const std::vector<std::byte>& buf);

    Buffer(const std::vector<std::byte>& buf, const int offset, const int len); 

    static Buffer kill();

    // used 'const' to repesent this return data cannot be updated after returing.
    std::string getString() const;

    std::string toString() const;

private:
    void validateBuffer() const;
};

#endif // BUFFER_H