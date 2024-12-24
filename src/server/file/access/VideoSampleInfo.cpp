#include "../include/VideoSampleInfo.h"

VideoSampleInfo::VideoSampleInfo()
    : rtpMetaInfos(std::vector<RtpMetaInfo>{}), size(0), offset(0), flag(0) {}

std::vector<RtpMetaInfo>& VideoSampleInfo::getMetaInfoList() {
    return rtpMetaInfos;
}

int VideoSampleInfo::getSize() const {
    return size;
}

int VideoSampleInfo::getOffset() const {
    return offset;
}

int VideoSampleInfo::getFlag() const {
    return flag;
}

void VideoSampleInfo::setSize(const int inputSize) {
    size = inputSize;
}

void VideoSampleInfo::setOffset(const int64_t inputOffset) {
    offset = inputOffset;
}

void VideoSampleInfo::setFlag(const int inputFlag) {
    flag = inputFlag;
}
