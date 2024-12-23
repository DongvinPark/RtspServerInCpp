#ifndef VIDEOSAMPLEINFO_H
#define VIDEOSAMPLEINFO_H

#include <vector>
#include "../include/RtpMetaInfo.h"

class VideoSampleInfo {
public:
    explicit VideoSampleInfo();
    std::vector<RtpMetaInfo>& getMetaInfoList();

    int getSize() const;
    int getOffset() const;
    int getFlag() const;

    void setSize(int inputSize);
    void setOffset(int64_t inputOffset);
    void setFlag(int inputFlag);

private:
    std::vector<RtpMetaInfo> rtpMetaInfos;
    int size;
    int64_t offset;
    int flag; // 1: Key(== I) frame, 0: P frame
};

#endif //VIDEOSAMPLEINFO_H
