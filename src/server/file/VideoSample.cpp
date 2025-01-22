#include "../include/VideoSample.h"
#include "../constants/Util.h"
#include <stdexcept>

VideoSample::VideoSample() : size(0) {}

std::vector<Buffer>& VideoSample::getAllRtps() {
    return rtps;
}

Buffer& VideoSample::getFirstRtp() {
    if (rtps.empty()) throw std::out_of_range("No Rtp packets in video sample!");
	return rtps.front();
}

std::vector<int>& VideoSample::getCSeq() {
    if (rtps.empty()) throw std::logic_error("Cannot get CSeq - No Rtp packets!");
    if (cSeq.empty() == false) {
        return cSeq;
    }
   
    for (auto i = 0; i < rtps.size()-1; ++i) {
        cSeq.push_back(Util::findSequenceNumber(rtps.front()));
    }
    return cSeq;
}

int VideoSample::getSize() const {
    return size;
}

void VideoSample::addRtp(const Buffer& buffer) {
    rtps.push_back(buffer);
}

const std::string VideoSample::toString() {
    return "VideoSample: size = " + std::to_string(size) +
        ", rtps = " + std::to_string(rtps.size()) +
        ", cSeq = " + std::to_string(cSeq.size());
}