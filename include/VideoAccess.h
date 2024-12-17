#ifndef VIDEOACCESS_H
#define VIDEOACCESS_H

#include <fstream>

#include "../include/VideoSampleInfo.h"

class VideoAccess {
public:
    explicit VideoAccess();
    ~VideoAccess();

    std::vector<std::fstream>& getAccess();
    std::vector<VideoSampleInfo>& getVideoSampleInfo();

    void setAccesses(std::vector<std::fstream>& access);
    void setMeta(std::vector<std::vector<VideoSampleInfo>>& meta);

    int getFileNumber();

    void close();

private:
    std::vector<std::fstream> accesses;
    std::vector<std::vector<VideoSampleInfo>> meta;
};

#endif //VIDEOACCESS_H
