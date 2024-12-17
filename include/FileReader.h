#ifndef FILEREADER_H
#define FILEREADER_H
#include <filesystem>

#include "../include/RtpInfo.h"
#include "../constants/C.h"
#include "../include/Logger.h"
#include "../include/VideoAccess.h"
#include "../include/AudioAccess.h"

class FileReader {
public:

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
