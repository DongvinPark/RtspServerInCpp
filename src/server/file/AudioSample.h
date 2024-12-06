#ifndef AUDIOSAMPLE_H
#define AUDIOSAMPLE_H

// use relative to include parent class in different directory
#include "../src/util/Buffer.h"
#include <vector>

class AudioSample : public Buffer {
public:
    AudioSample(std::vector<unsigned char>& buf, const int len);
    int size;
    std::string toString();
};

#endif // AUDIOSAMPLE_H