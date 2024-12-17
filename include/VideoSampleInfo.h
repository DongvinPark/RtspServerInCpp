#ifndef VIDEOSAMPLEINFO_H
#define VIDEOSAMPLEINFO_H

#include <vector>
#include "../include/RtpMetaInfo.h"

class VideoSampleInfo {
public:
    explicit VideoSampleInfo();
    void setSize(int inputSize);
    void setOffset(int inputOffset);
    void setFlag(int inputFlag);
    [[nodiscard]] int getSize();
    [[nodiscard]] int getOffset();
    [[nodiscard]] int getFlag();
    [[nodiscard]] std::vector<RtpMetaInfo>& getMetaInfo();
private:
    std::vector<RtpMetaInfo> rtpMetaInfos;
    int size;
    int offset;
    int flag; // 1: Key(== I) frame, 0: P frame
};

#endif //VIDEOSAMPLEINFO_H
