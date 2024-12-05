#include "ParsableByteArray.h"

ParsableByteArray::ParsableByteArray(const std::vector<unsigned char>& data, int len) {
    reset(data, len);
}

ParsableByteArray::ParsableByteArray(const std::vector<unsigned char>& data, int offset, int len) {
    reset(data, offset, len);
}

ParsableByteArray::ParsableByteArray(const std::vector<unsigned char>& data) {
    reset(data, data.size());
}

ParsableByteArray::ParsableByteArray(const Buffer& buffer) {
    reset(buffer.buf, buffer.offset, buffer.len);
}

void ParsableByteArray::reset(const std::vector<unsigned char>& data, int len) {
    this->data = data;
    limit = len;
    position = 0;
}

void ParsableByteArray::reset(const std::vector<unsigned char>& data, int offset, int len) {
    this->data = data;
    limit = offset + len;
    position = offset;
}

int ParsableByteArray::byteLeft() const {
    return limit - position;
}

int ParsableByteArray::getPosition() const {
    return position;
}

void ParsableByteArray::setPosition(int newPosition) {
    if (newPosition < 0 || newPosition > limit) {
        throw std::out_of_range("Position out of bounds");
    }
    position = newPosition;
}

const std::vector<unsigned char>& ParsableByteArray::getData() const {
    return data;
}

void ParsableByteArray::skipBytes(int bytes) {
    setPosition(position + bytes);
}

void ParsableByteArray::readBytes(std::vector<unsigned char>& buf, int offset, int length) {
    if (position + length > limit) {
        throw std::out_of_range("Not enough bytes to read");
    }
    std::copy(data.begin() + position, data.begin() + position + length, buf.begin() + offset);
    position += length;
}

int ParsableByteArray::size() const {
    return limit;
}

int ParsableByteArray::readByte() {
    if (position >= limit) {
        throw std::out_of_range("Position out of bounds");
    }
    return data[position++] & 0xFF;
}

int ParsableByteArray::peekByte() const {
    if (position >= limit) {
        throw std::out_of_range("Position out of bounds");
    }
    return data[position] & 0xFF;
}

int ParsableByteArray::peekByte(int offset) const {
    if (position + offset >= limit) {
        throw std::out_of_range("Position out of bounds");
    }
    return data[position + offset] & 0xFF;
}

int ParsableByteArray::readUnsignedShort() {
    if (position + 2 > limit) {
        throw std::out_of_range("Not enough bytes to read");
    }
    int byte1 = data[position++] & 0xFF;
    int byte2 = data[position++] & 0xFF;
    return (byte1 << 8) | byte2;
}

int16_t ParsableByteArray::readShort() {
    return static_cast<int16_t>(readUnsignedShort());
}

uint32_t ParsableByteArray::readUnsignedInt() {
    if (position + 4 > limit) {
        throw std::out_of_range("Not enough bytes to read");
    }
    uint32_t byte1 = data[position++] & 0xFF;
    uint32_t byte2 = data[position++] & 0xFF;
    uint32_t byte3 = data[position++] & 0xFF;
    uint32_t byte4 = data[position++] & 0xFF;
    return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
}

int32_t ParsableByteArray::readInt() {
    return static_cast<int32_t>(readUnsignedInt());
}

uint64_t ParsableByteArray::readLong() {
    if (position + 8 > limit) {
        throw std::out_of_range("Not enough bytes to read");
    }
    uint64_t byte1 = static_cast<uint64_t>(data[position++] & 0xFF);
    uint64_t byte2 = static_cast<uint64_t>(data[position++] & 0xFF);
    uint64_t byte3 = static_cast<uint64_t>(data[position++] & 0xFF);
    uint64_t byte4 = static_cast<uint64_t>(data[position++] & 0xFF);
    uint64_t byte5 = static_cast<uint64_t>(data[position++] & 0xFF);
    uint64_t byte6 = static_cast<uint64_t>(data[position++] & 0xFF);
    uint64_t byte7 = static_cast<uint64_t>(data[position++] & 0xFF);
    uint64_t byte8 = static_cast<uint64_t>(data[position++] & 0xFF);
    return (byte1 << 56) | (byte2 << 48) | (byte3 << 40) | (byte4 << 32) |
           (byte5 << 24) | (byte6 << 16) | (byte7 << 8) | byte8;
}

void ParsableByteArray::writeUnsignedShort(int pos, int value) {
    if (pos + 1 >= limit) {
        throw std::out_of_range("Position out of bounds");
    }
    data[pos] = static_cast<unsigned char>((value & 0xFF00) >> 8);
    data[pos + 1] = static_cast<unsigned char>(value & 0x00FF);
}

void ParsableByteArray::writeUnsignedShort(int value) {
    writeUnsignedShort(position, value);
    position += 2;
}

void ParsableByteArray::writeUnsignedInt(int pos, uint32_t value) {
    if (pos + 3 >= limit) {
        throw std::out_of_range("Position out of bounds");
    }
    data[pos] = static_cast<unsigned char>((value & 0xFF000000) >> 24);
    data[pos + 1] = static_cast<unsigned char>((value & 0x00FF0000) >> 16);
    data[pos + 2] = static_cast<unsigned char>((value & 0x0000FF00) >> 8);
    data[pos + 3] = static_cast<unsigned char>(value & 0x000000FF);
}