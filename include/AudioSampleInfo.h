#ifndef AUDIOSAMPLEINFO_H
#define AUDIOSAMPLEINFO_H

#include "../include/RtpMetaInfo.h"

class AudioSampleInfo : public RtpMetaInfo {
public:
    explicit AudioSampleInfo(int len, int offset);
};

#endif //AUDIOSAMPLEINFO_H
