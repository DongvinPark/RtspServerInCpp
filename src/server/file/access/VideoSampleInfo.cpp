#include "../include/VideoSampleInfo.h"

VideoSampleInfo::VideoSampleInfo()
    : rtpMetaInfos(std::vector<RtpMetaInfo>{}), size(0), offset(0), flag(0) {}

std::vector<RtpMetaInfo>& VideoSampleInfo::getMetaInfoList() {
    return rtpMetaInfos;
}

[[nodiscard]] const std::vector<RtpMetaInfo>& VideoSampleInfo::getConstMetaInfoList() const {
    return rtpMetaInfos;
}

[[nodiscard]] int VideoSampleInfo::getSize() const noexcept {
    return size;
}

[[nodiscard]] int64_t VideoSampleInfo::getOffset() const noexcept {
    return offset;
}

[[nodiscard]] int VideoSampleInfo::getFlag() const noexcept {
    return flag;
}

void VideoSampleInfo::setSize(const int inputSize) {
    if (initialized == false) size = inputSize;
}

void VideoSampleInfo::setOffset(const int64_t inputOffset) {
    if (initialized == false) offset = inputOffset;
}

void VideoSampleInfo::setFlag(const int inputFlag) {
    if (initialized == false) flag = inputFlag;
}

void VideoSampleInfo::initCompleted() {
    initialized = true;
}