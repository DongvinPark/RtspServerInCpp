#ifndef VIDEOSAMPLE_H
#define VIDEOSAMPLE_H

#include <vector>
#include <string>
#include "../src/util/Buffer.h"

class VideoSample {
private:
    std::vector<Buffer> rtps;  // RTP packets
    std::vector<int> cSeq;     // Sequence numbers
    int size;                  // Total size of the sample

public:
    VideoSample();

    /*
    function signature tip;
    
    > when returning '&'.
        permit modification:
            std::vector<Buffer>& getAllRtps();
        forbidden modification:
            const std::vector<Buffer>& getAllRtps();

    > when return value.
        int size() const;
    */

    std::vector<Buffer>& getAllRtps();
    Buffer& getFirstRtp();
    std::vector<int>& getCSeq();
    int getSize() const;

    void addRtp(const Buffer& buffer);

    const std::string toString();
};

#endif // VIDEOSAMPLE_H
