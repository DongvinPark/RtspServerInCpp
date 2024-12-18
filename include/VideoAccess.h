#ifndef VIDEOACCESS_H
#define VIDEOACCESS_H

#include <fstream>

#include "../include/VideoSampleInfo.h"

class VideoAccess {
public:
    explicit VideoAccess();
    ~VideoAccess();

    std::vector<std::ifstream>& getAccessList();
    std::vector<std::vector<VideoSampleInfo>>& getVideoSampleInfoList();

    const std::vector<std::ifstream>& getConstAccessList() const;
    const std::vector<std::vector<VideoSampleInfo>>& getConstVideoSampleInfoList() const;

    void setMeta(const std::vector<std::vector<VideoSampleInfo>>& inputMeta);

    int getFileNumber() const;

    void close();

private:
    std::vector<std::ifstream> accesses;
    std::vector<std::vector<VideoSampleInfo>> meta;
};

#endif //VIDEOACCESS_H
