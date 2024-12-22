#ifndef VIDEOSAMPLE_H
#define VIDEOSAMPLE_H

#include <vector>
#include <string>
#include "../include/Buffer.h"

class VideoSample {
private:
    std::vector<Buffer> rtps;  // RTP packets
    std::vector<int> cSeq;     // Sequence numbers
    int size;                  // Total size of the sample

public:
    explicit VideoSample();

    std::vector<Buffer>& getAllRtps();
    Buffer& getFirstRtp();
    std::vector<int>& getCSeq();
    int getSize() const;

    void addRtp(const Buffer& buffer);

    const std::string toString();
};

#endif // VIDEOSAMPLE_H
