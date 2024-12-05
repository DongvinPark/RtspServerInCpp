#include "Buffer.h"
#include <sstream> // For std::ostringstream
#include <stdexcept>

Buffer::Buffer(const std::vector<std::byte>& buf)
    : buf(buf), len(), offset(0), limit() {}

Buffer::Buffer(const std::vector<std::byte>& buf, const int offset, const int len)
    : buf(buf), len(len), offset(offset), limit(offset+len) {
        validateBuffer();
    }

Buffer Buffer::kill(){
    return Buffer({}, C::UNSET, C::UNSET);
}

std::string Buffer::getString() const {
    validateBuffer();
    if(buf.empty() || len <= 0) return {};
    return std::string(
        // reinterpret_cast was used to make std::string from std::byte elements in vector.
        reinterpret_cast<const char*>(buf.data() + offset), len
        );
}

std::string Buffer::toString() const {
    std::ostringstream oss;
    oss << "buf null? " << (buf.empty() ? "true" : "false") 
        << ", len: " << len 
        << ", offset: " << offset 
        << ", limit: " << limit
        << ", bodyLen: " << bodyLen
        << ", sampleNo: " << sampleNo
        << ", mediaType: " << mediaType;

    if (!buf.empty() && len > 0) {
        oss << ", data: " << getString();
    }

    return oss.str();
}

void Buffer::validateBuffer() const {
    if(offset < 0 || len < 0 || offset + len > static_cast<int>(buf.size())){
        throw std::out_of_range("Invalid offset or length for buffer.");
    }
}