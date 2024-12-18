#ifndef PARSABLEBYTEARRAY_H
#define PARSABLEBYTEARRAY_H

#include <vector>
#include <cstdint>
#include <stdexcept>
#include "../include/Buffer.h"

class ParsableByteArray {
private:
    std::vector<unsigned char> data;
    int position = 0;
    int limit = 0;

public:
    // Constructors
    explicit ParsableByteArray(const std::vector<unsigned char>& data, int len);
    explicit ParsableByteArray(const std::vector<unsigned char>& data, int offset, int len);
    explicit ParsableByteArray(const std::vector<unsigned char>& data);
    explicit ParsableByteArray(const Buffer& buffer);
    
    // Reset methods
    void reset(const std::vector<unsigned char>& data, int len);
    void reset(const std::vector<unsigned char>& data, int offset, int len);

    // Accessors
    int byteLeft() const;
    int getPosition() const;
    void setPosition(int position);
    const std::vector<unsigned char>& getData();

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
    int16_t readShort(); // int16_t type means fixed 16 bit size int.
    uint32_t readUnsignedInt();// uint32_t type means fixed 32 bit size unsigned int.
    int32_t readInt();
    uint64_t readint64_t();

    // Write methods
    void writeUnsignedShort(int pos, int value);
    void writeUnsignedShort(int value);
    void writeUnsignedInt(int pos, uint32_t value);
};

#endif // PARSABLEBYTEARRAY_H