#ifndef HYBRIDSAMPLEMETA_H
#define HYBRIDSAMPLEMETA_H

#include <string>
#include <vector>
#include <cstdint> // for int64_t

class HybridSampleMeta {
public:

    HybridSampleMeta();

    HybridSampleMeta(int sampleNo, int64_t startOffset, int64_t len, int64_t timeStamp);

    [[nodiscard]] std::vector<unsigned char> getHybridMetaBinary(
        unsigned char channelForAvptSampleQ,
        int camId,
        int viewNum,
        const std::string& frameType) const;

    [[nodiscard]] std::string toString() const;

private:
    int sampleNo;
    int64_t startOffset;
    int64_t len;
    int64_t timeStamp;

    [[nodiscard]] std::string makeStringForTx(int camId, int viewNum, const std::string& frameType) const;
    void recordChannelInfo(std::vector<unsigned char>& data, unsigned char channel, int len) const;
    void recordPayLoad(std::vector<unsigned char>& data, const std::string& payLoad) const;
};

#endif //HYBRIDSAMPLEMETA_H
