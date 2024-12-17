#ifndef AUDIOACCESS_H
#define AUDIOACCESS_H
#include <fstream>
#include <vector>

#include "../include/AudioSampleInfo.h"

class AudioAccess {
public:
    explicit AudioAccess();
    ~AudioAccess();

    // fstream is impossible to copy.
    std::fstream& getAccess();
    std::vector<AudioSampleInfo>& getMeta();

    void setAccess(const std::string& inputFilePath, std::ios::openmode mode);
    void setMeta(const std::vector<AudioSampleInfo>& inputMeta);

    void close();

private:
    std::fstream access;
    std::vector<AudioSampleInfo> meta;
};

#endif //AUDIOACCESS_H