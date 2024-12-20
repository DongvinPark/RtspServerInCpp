#include "../include/FileReader.h"
#include "../../../constants/Util.h"

/*
    std::shared_ptr<Logger> logger;
    std::string ASC = "asc";
    std::string ASA = "asa";
    std::string ASV = "asv";
    std::string RTSP_MSG_DELIMITER = "###" + std::to_string(*C::CRLF);
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
*/

// constructor
FileReader::FileReader(const std::filesystem::path &path)
  : logger(Logger::getLogger(C::FILE_READER)),
    cidDirectory(path),
    id(path.filename().string()){}

FileReader::~FileReader() {
  shutdown();
}

// public
int FileReader::getNumberOfCamDirectories() const {
  return videoFiles.size();
}

int FileReader::getRefVideoSampleCnt() const {
  // return cam 0's V1 video files sample cnt
  return videoFiles.at(C::REF_CAM).getConstVideoSampleInfoList()[0].size();
}

FileReader & FileReader::init() {
  if (cidDirectory.empty() || !exists(cidDirectory)) {
    logger->severe("No content files or wrong content root directory!");
    throw std::logic_error("FileReader failed.");
  }

  bool initResult = handleCamDirectories(cidDirectory);
  if (!initResult) {
    logger->severe("Invalid cam directories! Content name : " + id);
    throw std::logic_error("Cam directory init failed.");
  }

  initResult = handleConfigFile(cidDirectory);
  if (!initResult) {
    logger->severe("Invalid SDP config! Content name : " + id);
    throw std::logic_error("SDP config init failed.");
  }

  initResult = handleV0Images(cidDirectory);
  if (!initResult) {
    logger->severe("Invalid V0 images! Content name : " + id);
    throw std::logic_error("V0 images init failed.");
  }

  return *this;
}

void FileReader::shutdown() {
  // close all access
  for (auto& kvPair : videoFiles) {
    for (std::ifstream& videoAccess : kvPair.second.getAccessList()) {
      if (videoAccess.is_open()) {
        videoAccess.close();
      }
    }
  }
  videoFiles.clear();

  audioFile.close();
}

RtpInfo FileReader::getRtpInfoCopyWithLock() {
  std::lock_guard<std::mutex> guard(lock);
  // call Copy Constructor
  RtpInfo rtpInfoCopy(rtpInfo);
  return rtpInfoCopy;
}

std::string FileReader::getMediaInfoCopyWithLock() {
  std::lock_guard<std::mutex> guard(lock);
  return rtspSdpMessage;
}
std::vector<unsigned char> FileReader::getAccDataCopyWithLock() {
  std::lock_guard<std::mutex> guard(lock);
  return Util::readAllBytesFromFilePath(configFile);
}

std::vector<std::vector<unsigned char>> FileReader::getAllV0ImagesCopyWithLock() {
  std::lock_guard<std::mutex> guard(lock);
  std::vector<std::vector<unsigned char>> imageBinaryList;
  for (std::filesystem::path& imageFilePath : v0Images) {
    imageBinaryList.push_back(Util::readAllBytesFromFilePath(imageFilePath));
  }
  return imageBinaryList;
}

int FileReader::getAudioSampleSize() const {
  return audioFile.getConstMeta().size();
}

int FileReader::getVideoSampleSize() const {
  if (videoFiles.contains(C::REF_CAM)) {
    return videoFiles.at(C::REF_CAM).getConstVideoSampleInfoList()[0].size();
  } else {
    // adaptive bitrate supporting case
    for (std::string possibleRefCam : C::ADAPTIVE_BITRATE_REF_CAM_LIST) {
      if (videoFiles.contains(possibleRefCam)) {
        return videoFiles.at(possibleRefCam).getConstVideoSampleInfoList()[0].size();
      }
    }
  }
  return C::INVALID;
}

std::vector<AudioSampleInfo> FileReader::getAudioMetaCopyWithLock() {
  std::lock_guard<std::mutex> guard(lock);
  // used iterator and constructor to return to copied vector
  return std::vector<AudioSampleInfo>(
    audioFile.getConstMeta().begin(), audioFile.getConstMeta().end()
  );
}

const std::unordered_map<std::string, VideoAccess>& FileReader::getVideoMetaWithLock() {
  std::lock_guard<std::mutex> guard(lock);
  // do not copy. just return const ref('&')
  if (videoFiles.empty()) {
    // return empty map.
    return std::unordered_map<std::string, VideoAccess>{};
  } else {
    return videoFiles;
  }
}

AudioSample & FileReader::readAudioSampleWithLock(int sampleNo, HybridMetaMapType &hybridMetaMap) {
  std::lock_guard<std::mutex> guard(lock);
}

std::vector<VideoSample> & FileReader::readRefVideoSampleWithLock(int sampleNo,
  int mbpsCurBitrate,
  std::vector<int> possibleBitrateList,
  HybridMetaMapType &hybridMetaMap) {
  std::lock_guard<std::mutex> guard(lock);
}

std::vector<VideoSample> & FileReader::readVideoSampleWithLock(int camId,
  int vid,
  int memberId,
  int sampleNo,
  int mbpsCurBitrate,
  std::vector<int> possibleBitrateList,
  HybridMetaMapType &hybridMetaMap) {
  std::lock_guard<std::mutex> guard(lock);
}

// private
bool FileReader::handleCamDirectories(const std::filesystem::path &inputCidDirectory) {
  std::vector<std::filesystem::path> camDirectoryList;
  for (std::filesystem::path camDirectory : std::filesystem::directory_iterator(inputCidDirectory)) {
    camDirectoryList.push_back(camDirectory);
  }
  // sort ascending order of filenames
  std::sort(
    camDirectoryList.begin(), camDirectoryList.end()
    , [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
      return lhs.filename() < rhs.filename();
    }
  );

  for (std::filesystem::path camDirectory : camDirectoryList) {
    if (!is_directory(camDirectory)) continue;
    if (camDirectory.filename() == C::HYBRID_META_DIR) continue;
    bool result = loadAcsFilesInCamDirectories(camDirectory);
    if (!result) {
      return false;
    }
  }
  return true;
}

bool FileReader::handleConfigFile(const std::filesystem::path &inputCidDirectory) {
  for (std::filesystem::path dir : std::filesystem::directory_iterator(inputCidDirectory)) {
    // dir.filename().string().find(..input str..) != std::string::npos >> this can be used as
    // java's .contains() method
    if (!is_directory(dir) && dir.filename().string().find("acc") != std::string::npos) {
      // std::filesystem::path type is 'Copyable'!!
      configFile = dir;
      return true;
    }
  }
  return false;
}

bool FileReader::handleV0Images(const std::filesystem::path &inputCidDirectory) {
  for (std::filesystem::path dir : std::filesystem::directory_iterator(inputCidDirectory)) {
    if (!is_directory(dir) && dir.filename().string().find("jpg") != std::string::npos) {
      v0Images.push_back(dir);
    }
  }
  return v0Images.size() > 0;
}

bool FileReader::loadAcsFilesInCamDirectories(const std::filesystem::path &inputCidDirectory) {
}

bool FileReader::loadRtspRtpConfig(const std::filesystem::path &rtspConfig) {
}

std::vector<int64_t> FileReader::getValues(std::string inputMsg, std::string inputKey) {
}

AudioAccess FileReader::loadRtpAudioMetaDataAndGetCopy(const std::filesystem::path &inputAudio) {
}

void FileReader::showAudioMinMaxSize(const std::vector<AudioSampleInfo> &audioMetaData) {
}

std::vector<std::ifstream> & FileReader::openVideosAndGetCopy(
  const std::vector<std::filesystem::path> &videos) {
}

VideoAccess & FileReader::loadRtpVideoMetaData(const std::vector<std::filesystem::path> &videos) {
}

std::vector<VideoSampleInfo> & FileReader::loadRtpMemberVideoMetaData(
    std::ifstream &member, int memberId
) {
}
void FileReader::showVideoMaxSize(const std::vector<VideoSampleInfo> &videoMetaData, int memberId) {
}

std::vector<VideoSample> & FileReader::readVideoSampleInternalWithLock(int camId,
  VideoAccess &va,
  int sampleNo,
  HybridMetaMapType &hybridMetaMap) {
  std::lock_guard<std::mutex> guard(lock);
}

std::vector<std::vector<VideoSample>> & FileReader::getVideoMeta(std::string camId) {
}

std::vector<unsigned char> FileReader::readMetaData(std::ifstream &inputFileStream) {
}

std::vector<int16_t> FileReader::getSizes(std::vector<unsigned char> metaData) {
}
