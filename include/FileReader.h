#ifndef FILEREADER_H
#define FILEREADER_H
#include <__filesystem/path.h>

#include "../include/RtpMetaInfo.h"
#include "../include/RtpInfo.h"
#include "../constants/C.h"
#include "../include/Logger.h"

class FileReader {
public:
    // TODO : make these four inner class as separate class.
    // TODO : create new directory named 'access' for these 4 inner calsses.
    class VideoAccess {
    public:
    private:
    };

    class AudioAccess{
    public:
    private:
    };

    class VideoSampleInfo {
    public:
        explicit VideoSampleInfo();
        void setSize(int inputSize);
        void setOffset(int inputOffset);
        void setFlag(int inputFlag);
        [[nodiscard]] int getSize();
        [[nodiscard]] int getOffset();
        [[nodiscard]] int getFlag();
        [[nodiscard]] std::vector<RtpMetaInfo>& getMetaInfo();
    private:
        std::vector<RtpMetaInfo> rtpMetaInfos;
        int size;
        int offset;
        int flag; // 1: Key(== I) frame, 0: P frame
    };

    class AudioSampleInfo : public RtpMetaInfo {
    public:
        explicit AudioSampleInfo(int len, int offset);
    };

private:
    std::shared_ptr<Logger> logger;
    std::string ASC = "asc";
    std::string ASA = "asa";
    std::string ASV = "asv";
    std::string REST_MSG_DELIMITER = "###" + std::to_string(*C::CRLF);
    int META_LEN_BYTES = 4;

    std::filesystem::path cidDirectory;
    std::filesystem::path configFile;
    std::string id;

    AudioAccess audioFile;
    std::unordered_map<std::string, VideoAccess> videoFiles;

    std::string rtspSdpMessage;
    RtpInfo rtpInfo;
    std::vector<std::filesystem::path> v0Images;
};

#endif //FILEREADER_H
