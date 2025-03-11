#include "../include/HybridSampleMeta.h"
#include "../constants/C.h"
#include <cassert>
#include <sstream>

HybridSampleMeta::HybridSampleMeta()
    : sampleNo(C::UNSET),
    startOffset(C::INVALID_OFFSET),
    len(C::INVALID_OFFSET),
    timeStamp(C::INVALID_OFFSET) {}

HybridSampleMeta::HybridSampleMeta(int sampleNo, int64_t startOffset, int64_t len, int64_t timeStamp)
    : sampleNo(sampleNo), startOffset(startOffset), len(len), timeStamp(timeStamp) {}

[[nodiscard]] std::vector<unsigned char> HybridSampleMeta::getHybridMetaBinary(
    unsigned char channelForAvptSampleQ,
    int camId,
    int viewNum,
    const std::string& frameType) const {

    std::string payLoad = makeStringForTx(camId, viewNum, frameType);
    std::vector<unsigned char> data(C::RTP_CHANNEL_INFO_META_LENGTH + payLoad.length());
    recordChannelInfo(data, channelForAvptSampleQ, static_cast<int>(payLoad.length()));
    recordPayLoad(data, payLoad);
    return data;
}

[[nodiscard]] std::string HybridSampleMeta::makeStringForTx(int camId, int viewNum, const std::string& frameType) const {
    std::ostringstream oss;
    oss << C::HYBRID_META_PAYLOAD_PREFIX
        << camId << C::COMMA_SEPARATOR
        << viewNum << C::COMMA_SEPARATOR
        << frameType << C::COMMA_SEPARATOR
        << sampleNo << C::COMMA_SEPARATOR
        << startOffset << C::COMMA_SEPARATOR
        << len << C::COMMA_SEPARATOR
        << timeStamp;
    return oss.str();
}

void HybridSampleMeta::recordChannelInfo(std::vector<unsigned char>& data, unsigned char channel, int len) const {
    assert(data.size() >= C::RTP_CHANNEL_INFO_META_LENGTH);
    data[0] = static_cast<unsigned char>(C::INTERLEAVED_BINARY_DATA_MARKER);
    data[1] = channel;
    data[2] = static_cast<unsigned char>((len >> 8) & 0xFF);
    data[3] = static_cast<unsigned char>(len & 0xFF);
}

void HybridSampleMeta::recordPayLoad(std::vector<unsigned char>& data, const std::string& payLoad) const {
    for (auto i = C::RTP_CHANNEL_INFO_META_LENGTH; i < data.size(); ++i) {
        data[i] = static_cast<unsigned char>(payLoad[i - C::RTP_CHANNEL_INFO_META_LENGTH]);
    }
}

[[nodiscard]] std::string HybridSampleMeta::toString() const {
    std::ostringstream oss;
    oss << sampleNo << C::COMMA_SEPARATOR
        << startOffset << C::COMMA_SEPARATOR
        << len << C::COMMA_SEPARATOR
        << timeStamp;
    return oss.str();
}