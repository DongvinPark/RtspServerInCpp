#ifndef FILEREADER_H
#define FILEREADER_H
#include <filesystem>
#include <mutex>
#include <cstdint> // For int64_t

#include "../include/AudioSample.h"
#include "../include/HybridSampleMeta.h"
#include "../include/VideoSample.h"
#include "../include/RtpInfo.h"
#include "../constants/C.h"
#include "../include/Logger.h"
#include "../include/VideoAccess.h"
#include "../include/AudioAccess.h"

using HybridMetaMapType
    = std::unordered_map<int, std::unordered_map<std::string, std::unordered_map<int, HybridSampleMeta>>>;

class FileReader : public std::enable_shared_from_this<FileReader> {
public:
    explicit FileReader(const std::filesystem::path& path);
    ~FileReader();

    // Rule of five. FileReader object is not allowed to copy.
    FileReader(const FileReader&) = delete;
    FileReader& operator=(const FileReader&) = delete;
    FileReader& operator=(FileReader&& other) noexcept = delete;
    // enable move constructor.
    FileReader(FileReader&& other) noexcept;

    // utils
    int getNumberOfCamDirectories() const;
    int getRefVideoSampleCnt() const;
    void init();
    void shutdown();
    RtpInfo getRtpInfoCopyWithLock();
    std::string getMediaInfoCopyWithLock();
    std::vector<unsigned char> getAccDataCopyWithLock();
    std::vector<std::vector<unsigned char>> getAllV0ImagesCopyWithLock();
    int getAudioSampleSize() const;
    int getVideoSampleSize() const;
    std::vector<AudioSampleInfo> getAudioMetaCopyWithLock();
    std::unordered_map<std::string, VideoAccess>& getVideoMetaWithLock();

    // reading sample
    AudioSample& readAudioSampleWithLock(int sampleNo, HybridMetaMapType& hybridMetaMap);
    std::vector<VideoSample>& readRefVideoSampleWithLock(
        int sampleNo, HybridMetaMapType& hybridMetaMap
    );
    std::vector<VideoSample>& readVideoSampleWithLock(
        int camId, int vid, int memberId, int sampleNo,
        HybridMetaMapType& hybridMetaMap
    );

private:
    bool handleCamDirectories(const std::filesystem::path& inputCidDirectory);
    bool handleConfigFile(const std::filesystem::path& inputCidDirectory);
    bool handleV0Images(const std::filesystem::path& inputCidDirectory);
    bool loadAcsFilesInCamDirectories(const std::filesystem::path& inputCidDirectory);
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

    // reading sample
    std::vector<VideoSample>& readVideoSampleInternalWithLock(
        int camId, VideoAccess& va, int sampleNo,
        HybridMetaMapType& hybridMetaMap
    );

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
    std::mutex lock;
};

#endif //FILEREADER_H
