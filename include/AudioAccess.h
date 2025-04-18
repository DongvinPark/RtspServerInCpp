#ifndef AUDIOACCESS_H
#define AUDIOACCESS_H
#include <fstream>
#include <vector>

#include "../include/AudioSampleInfo.h"

class AudioAccess {
public:
    explicit AudioAccess();
    ~AudioAccess();

    // enabled move semantics
    AudioAccess(AudioAccess&& other) noexcept;
    AudioAccess& operator=(AudioAccess&& other) noexcept;

    std::ifstream& getAccess();
    std::vector<AudioSampleInfo>& getMeta();

    // const version for read-only access
    const std::ifstream& getConstAccess() const;
    [[nodiscard]] const std::vector<AudioSampleInfo>& getConstMeta() const;

    void close();

private:
    std::ifstream access;
    std::vector<AudioSampleInfo> meta;
};

#endif //AUDIOACCESS_H