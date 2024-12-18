#include "../include/VideoSampleInfo.h"

VideoSampleInfo::VideoSampleInfo() {}

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

void VideoSampleInfo::setOffset(const int inputOffset) {
    offset = inputOffset;
}

void VideoSampleInfo::setFlag(const int inputFlag) {
    flag = inputFlag;
}
