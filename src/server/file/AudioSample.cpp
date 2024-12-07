#include "AudioSample.h"

AudioSample::AudioSample(std::vector<unsigned char>& buf, const int len)
    : Buffer(buf, 0, len), size(len){}

std::string AudioSample::toString() const {
    return "audio sample size : " + std::to_string(size);
}