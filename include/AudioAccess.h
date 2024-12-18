#ifndef AUDIOACCESS_H
#define AUDIOACCESS_H
#include <fstream>
#include <vector>

#include "../include/AudioSampleInfo.h"

class AudioAccess {
public:
    explicit AudioAccess();
    ~AudioAccess();

    std::ifstream& getAccess();
    std::vector<AudioSampleInfo>& getMeta();

    // const version for read-only access
    const std::ifstream& getConstAccess() const;
    const std::vector<AudioSampleInfo>& getConstMeta() const;

    void openAccessFileReadOnly(const std::string& inputFilePath);
    void setMeta(const std::vector<AudioSampleInfo>& inputMeta);

    void close();

private:
    std::ifstream access;
    std::vector<AudioSampleInfo> meta;
};

#endif //AUDIOACCESS_H