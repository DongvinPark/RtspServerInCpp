#ifndef AVSAMPLEBUFFER_H
#define AVSAMPLEBUFFER_H

#include "../constants/C.h"
#include "../include/Buffer.h"
#include <vector>

class AVSampleBuffer {
public:
    explicit AVSampleBuffer(const int inputStreamId);
    explicit AVSampleBuffer(const Buffer& rtp, const int inputStreamId);
    explicit AVSampleBuffer(const Buffer& buffer);

    void setKill();

    int size();
    int rtpSize();
    int payloadSize();
    const std::string toString();

    void setStreamId(const int inputStreamId);
    void addRtpPacket(Buffer& rtp);
    void setSampleTimeIndex(const int inputSampleTimeIndex);
    void setMemberId(const int inputMemberId);
    void setTimeUs(const int64_t inputTimeUs);
    void setReq(const int inputReq);
    void setCseqNumber(const std::vector<int>& inputCseqVec);

private:
    int streamId = C::UNSET;
    std::vector<Buffer> rtps{};
    int sampleTimeIndex = C::UNSET;
    int memberId = C::UNSET;
    int64_t timeUs = C::UNSET;
    int req = C::UNSET;
    int totalBytes = C::UNSET;
    int payloadBytes = C::UNSET;
    std::vector<int> cSeqNumber{};
    explicit AVSampleBuffer();
};

#endif // AVSAMPLEBUFFER_H