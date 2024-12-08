#include "AVSampleBuffer.h"
#include <numeric> // for std::accumulate

AVSampleBuffer::AVSampleBuffer(const int inputStreamId) : streamId(inputStreamId) {}

AVSampleBuffer::AVSampleBuffer(const Buffer& rtp, const int inputStreamId)
    : streamId(inputStreamId) {
    rtps.push_back(rtp);
}

AVSampleBuffer::AVSampleBuffer(const Buffer& buffer) {
    rtps.push_back(buffer);
}

void AVSampleBuffer::setKill() {
    setReq(0);
}

int AVSampleBuffer::size() {
    if (totalBytes != C::UNSET) return totalBytes;
    // used std::accumulate util.
    totalBytes = std::accumulate(
        rtps.begin(), rtps.end(), 0, 
        [](int sum, const Buffer& b) { return sum + b.len; }
    );
    return totalBytes;
}

int AVSampleBuffer::rtpSize() {
    // since, last Buffer(==one rtp packet) is dummy.
    // dummy rtp packet is added for recording mornitoring info.
    return rtps.empty() == false? rtps.size() -1 : 0;
}

int AVSampleBuffer::payloadSize() {
    if(payloadBytes != C::UNSET) return payloadBytes;
    payloadBytes = std::accumulate(
        rtps.begin(), rtps.end(), 0,
        [](int sum, const Buffer& b){
            return sum + (b.len - C::RTP_HEADER_LEN - C::TCP_RTP_HEAD_LEN);
        }
    );
    return payloadBytes;
}

const std::string AVSampleBuffer::toString() {
    std::string cSeqString = cSeqNumber.empty() ? ""
        : ", [min,max]=[" + std::to_string(cSeqNumber.front()) 
          + "/" + std::to_string(cSeqNumber.back()) + "]";

    return "\nstream id: " + std::to_string(streamId) +
           "\nmember Id: " + std::to_string(memberId) +
           "\ntime: " + std::to_string(timeUs) + "(us)" +
           "\nsample time index: " + std::to_string(sampleTimeIndex) +
           "\ncSeq: size:" + std::to_string(cSeqNumber.size()) + cSeqString +
           "\ntotal size: " + std::to_string(size()) +
           "\npayload size: " + std::to_string(payloadSize()) +
           "\nrtp packets num: " + std::to_string(rtpSize());
}

void AVSampleBuffer::setStreamId(const int inputStreamId) {
    streamId = inputStreamId;
}

// DO NOT USE 'const' on target object of std::move!!
void AVSampleBuffer::addRtpPacket(Buffer &rtp) {
    // used std::move for performance
    rtps.push_back(std::move(rtp));
}

void AVSampleBuffer::setSampleTimeIndex(const int inputSampleTimeIndex) {
    sampleTimeIndex = inputSampleTimeIndex;
}

void AVSampleBuffer::setMemberId(const int inputMemberId) {
    memberId = inputMemberId;
}

void AVSampleBuffer::setTimeUs(const long inputTimeUs) {
    timeUs = inputTimeUs;
}

void AVSampleBuffer::setReq(const int inputReq) {
    req = inputReq;
}

void AVSampleBuffer::setCseqNumber(const std::vector<int> &inputCseqVec) {
    for(int cSeqVal : inputCseqVec){
        cSeqNumber.push_back(cSeqVal);
    }
}
