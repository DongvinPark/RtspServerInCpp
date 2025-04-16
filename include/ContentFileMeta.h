#ifndef FILEREADER_H
#define FILEREADER_H
#include <filesystem>
#include <mutex>
#include <cstdint> // For int64_t

#include "../include/HybridSampleMeta.h"
#include "../include/RtpInfo.h"
#include "../constants/C.h"
#include "../include/Logger.h"
#include "../include/VideoAccess.h"
#include "../include/AudioAccess.h"

using HybridMetaMapType
    = std::unordered_map<int, std::unordered_map<std::string, std::unordered_map<int, HybridSampleMeta>>>;

class ContentFileMeta : public std::enable_shared_from_this<ContentFileMeta> {
public:
    explicit ContentFileMeta(const std::filesystem::path& path);
    ~ContentFileMeta();

    // Rule of five. FileReader object is not allowed to copy.
    ContentFileMeta(const ContentFileMeta&) = delete;
    ContentFileMeta& operator=(const ContentFileMeta&) = delete;
    ContentFileMeta& operator=(ContentFileMeta&& other) noexcept = delete;
    // enable move constructor.
    ContentFileMeta(ContentFileMeta&& other) noexcept;

    // utils
    int getNumberOfCamDirectories() const;
    int getRefVideoSampleCnt() const;
    bool init();
    void shutdown();
    RtpInfo getRtpInfoCopy();
    std::string getMediaInfoCopy();
    std::vector<unsigned char> getAccDataCopy();
    std::vector<std::vector<unsigned char>> getAllV0ImagesCopy();
    int getAudioSampleSize() const;
    int getVideoSampleSize() const;
    const AudioAccess& getConstAudioMeta() const;
    const std::unordered_map<std::string, VideoAccess>& getConstVideoMeta() const;

private:
    bool handleCamDirectories(const std::filesystem::path& inputCidDirectory);
    bool handleConfigFile(const std::filesystem::path& inputCidDirectory);
    bool handleV0Images(const std::filesystem::path& inputCidDirectory);
    bool loadStreamFilesInCamDirectories(const std::filesystem::path& inputCidDirectory);
    void loadRtspRtpConfig(const std::filesystem::path& rtspConfig);
    // to mimic java's int64_t type in multiplatform.
    std::vector<int64_t> getValues(std::string inputMsg, std::string inputKey);
    void loadRtpAudioMetaData(
        const std::filesystem::path& inputAudio, AudioAccess& inputAudioFile
    );
    void showAudioMinMaxSize(const std::vector<AudioSampleInfo>& audioMetaData);
    void loadRtpVideoMetaData(
        const std::filesystem::path& inputCamDir, std::vector<std::filesystem::path>& videos
    );
    void loadRtpMemberVideoMetaData(
        int64_t videoFileSize,
        std::ifstream &inputIfstream,
        std::vector<std::vector<VideoSampleInfo>>& input2dMetaList,
        int memberId
    );
    void showVideoMinMaxSize(const std::vector<VideoSampleInfo>& videoMetaData, int memberId);

    std::vector<std::vector<VideoSampleInfo>> getVideoMetaInternal(std::string camId);
    std::vector<unsigned char> readMetaData(int64_t fileSize, std::ifstream& inputFileStream);
    // to mimic java's short type in multiplatform.
    std::vector<int16_t> getSizes(std::vector<unsigned char>& metaData);

    // members
    std::shared_ptr<Logger> logger;

    std::filesystem::path cidDirectory;
    std::filesystem::path configFile;
    std::string contentTitle;

    AudioAccess audioFile;
    std::unordered_map<std::string, VideoAccess> videoFiles;

    std::string rtspSdpMessage;
    RtpInfo rtpInfo;
    std::vector<std::filesystem::path> v0Images;
};

#endif //FILEREADER_H
