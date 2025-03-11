#include "../include/Buffer.h"
#include <sstream>
#include <stdexcept>

Buffer::Buffer()
    : len(C::UNSET), offset(C::UNSET), limit(C::UNSET){}

Buffer::Buffer(const std::vector<unsigned char>& buf)
    : buf(buf), len(buf.size()), offset(0), limit() {}

Buffer::Buffer(const std::vector<unsigned char>& buf, const int offset, const int len)
    : buf(buf), len(len), offset(offset), limit(offset+len) {
        validateBuffer();
    }

void Buffer::updateBuf(const std::vector<unsigned char> &inputBuf) {
    buf = std::move(inputBuf);
    len = inputBuf.size();
    offset = 0;
}


Buffer Buffer::kill(){
    return Buffer({}, C::UNSET, C::UNSET);
}

const std::string Buffer::getString() {
    validateBuffer();
    if(buf.empty() || len <= 0) return {};
    return std::string(
        // reinterpret_cast was used to make std::string from unsigned char elements in vector.
        reinterpret_cast<const char*>(buf.data() + offset), len
        );
}

const std::string Buffer::toString() {
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

void Buffer::validateBuffer() {
    if(offset < 0 || len < 0 || offset + len > static_cast<int>(buf.size())){
        throw std::out_of_range("Invalid offset or length for buffer.");
    }
}