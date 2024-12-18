#ifndef FILEREADER_H
#define FILEREADER_H
#include <filesystem>
#include <mutex>
#include <cstdint> // For int64_t

#include "AudioSample.h"
#include "../include/VideoSample.h"
#include "../include/RtpInfo.h"
#include "../constants/C.h"
#include "../include/Logger.h"
#include "../include/VideoAccess.h"
#include "../include/AudioAccess.h"

class FileReader {
public:
    explicit FileReader(const std::filesystem::path& path);
    ~FileReader();

    // utils
    int getNumberOfCamDirectories() const;
    int getRefVideoSampleCnt() const;
    FileReader& init();
    void shutdown();
    RtpInfo getRtpInfoCopyWithLock();
    std::string getMediaInfoCopyWithLock();
    std::vector<unsigned char> getAccDataCopyWithLock();
    std::vector<unsigned char> getAllV0ImagesCopyWithLock();
    int getAudioSampleSize() const;
    int getVideoSampleSize() const;
    std::vector<AudioSampleInfo> getAudioMetaCopyWithLock();
    std::unordered_map<std::string, VideoAccess> getVideoMetaCopyWithLock();

    // reading sample
    AudioSample& readAudioSampleWithLock(/* TODO : define parameters */);
    std::vector<VideoSample>& readRefVideoSampleWithLock(/* TODO : define parameters */);
    std::vector<VideoSample>& readVideoSampleWithLock(/* TODO : define parameters */);

private:
    bool handleCamDirectories(const std::filesystem::path& inputCidDirectory);
    bool handleConfigFile(const std::filesystem::path& inputCidDirectory);
    bool handleV0Images(const std::filesystem::path& inputCidDirectory);
    bool loadAcsFilesInCamDirectories(const std::filesystem::path& inputCidDirectory);
    bool loadRtspRtpConfig(const std::filesystem::path& rtspConfig);
    // to mimic java's long type in multiplatform.
    std::vector<int64_t> getValues(std::string inputMsg, std::string inputKey);
    AudioAccess loadRtpAudioMetaDataAndGetCopy(const std::filesystem::path& inputAudio);
    void showAudioMinMaxSize(const std::vector<AudioSampleInfo>& audioMetaData);
    std::vector<std::ifstream>& openVideosAndGetCopy(const std::vector<std::filesystem::path>& videos);
    VideoAccess& loadRtpVideoMetaData(const std::vector<std::filesystem::path>& videos);
    std::vector<VideoSampleInfo>& loadRtpMemberVideoMetaData(std::ifstream& member, int memberId);
    void showVideoMaxSize(const std::vector<VideoSampleInfo>& videoMetaData, int memberId);

    // reading sample
    std::vector<VideoSample>& readVideoSample(/* TODO : define parameters */);

    std::vector<std::vector<VideoSample>>& getVideoMeta(std::string camId);
    std::vector<unsigned char> readMetaData(std::ifstream& inputFileStream);
    // to mimic java's short type in multiplatform.
    std::vector<int16_t> getSizes(std::vector<unsigned char> metaData);

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
    std::mutex lock;
};

#endif //FILEREADER_H
