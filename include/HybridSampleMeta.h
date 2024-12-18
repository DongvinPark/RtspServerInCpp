#ifndef HYBRIDSAMPLEMETA_H
#define HYBRIDSAMPLEMETA_H

#include <string>
#include <vector>

class HybridSampleMeta {
public:
    HybridSampleMeta(int sampleNo, long startOffset, long len, long timeStamp);

    std::vector<unsigned char> getHybridMetaBinary(
        unsigned char channelForAvptSampleQ,
        int camId,
        int viewNum,
        const std::string& frameType) const;

    std::string toString() const;

private:
    int sampleNo;
    long startOffset;
    long len;
    long timeStamp;

    std::string makeStringForTx(int camId, int viewNum, const std::string& frameType) const;
    void recordChannelInfo(std::vector<unsigned char>& data, unsigned char channel, int len) const;
    void recordPayLoad(std::vector<unsigned char>& data, const std::string& payLoad) const;
};

#endif //HYBRIDSAMPLEMETA_H
