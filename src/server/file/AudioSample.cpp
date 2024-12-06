#include "AudioSample.h"
#include <sstream> // For std::ostringstream

AudioSample::AudioSample(std::vector<unsigned char>& buf, const int len)
    : Buffer(buf, 0, len), size(len){}

std::string AudioSample::toString(){
    std::ostringstream oss;
    oss << "audio sample size : " << len;
    return oss.str();
}