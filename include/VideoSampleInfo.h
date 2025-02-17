#ifndef VIDEOSAMPLEINFO_H
#define VIDEOSAMPLEINFO_H

#include <vector>
#include "../include/RtpMetaInfo.h"

class VideoSampleInfo {
public:
    explicit VideoSampleInfo();
    std::vector<RtpMetaInfo>& getMetaInfoList();
    [[nodiscard]] const std::vector<RtpMetaInfo>& getConstMetaInfoList() const;

    [[nodiscard]] int getSize() const noexcept;
    [[nodiscard]] int64_t getOffset() const noexcept;
    [[nodiscard]] int getFlag() const noexcept;

    void setSize(int inputSize);
    void setOffset(int64_t inputOffset);
    void setFlag(int inputFlag);
    void initCompleted();

private:
    std::vector<RtpMetaInfo> rtpMetaInfos;
    int size;
    int64_t offset;
    int flag; // 1: Key(== I) frame, 0: P frame
    bool initialized = false;
};

#endif //VIDEOSAMPLEINFO_H
