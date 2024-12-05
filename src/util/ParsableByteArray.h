#ifndef PARSABLEBYTEARRAY_H
#define PARSABLEBYTEARRAY_H

#include <vector>
#include <cstdint>
#include <stdexcept>
#include "Buffer.h"

class ParsableByteArray {
private:
    std::vector<unsigned char> data;
    int position = 0;
    int limit = 0;

public:
    // Constructors
    ParsableByteArray(const std::vector<unsigned char>& data, int len);
    ParsableByteArray(const std::vector<unsigned char>& data, int offset, int len);
    ParsableByteArray(const std::vector<unsigned char>& data);
    ParsableByteArray(const Buffer& buffer);
    
    // Reset methods
    void reset(const std::vector<unsigned char>& data, int len);
    void reset(const std::vector<unsigned char>& data, int offset, int len);

    // Accessors
    int byteLeft() const;
    int getPosition() const;
    void setPosition(int position);
    const std::vector<unsigned char>& getData() const;

    // Data operations
    void skipBytes(int bytes);
    void readBytes(std::vector<unsigned char>& buf, int offset, int length);

    // Size
    int size() const;

    // Read methods
    int readByte();
    int peekByte() const;
    int peekByte(int offset) const;
    int readUnsignedShort();
    int16_t readShort();
    uint32_t readUnsignedInt();
    int32_t readInt();
    uint64_t readLong();

    // Write methods
    void writeUnsignedShort(int pos, int value);
    void writeUnsignedShort(int value);
    void writeUnsignedInt(int pos, uint32_t value);
};

#endif // PARSABLEBYTEARRAY_H