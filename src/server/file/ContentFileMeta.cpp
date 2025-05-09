#include "../include/ContentFileMeta.h"
#include "../../../constants/Util.h"

#include <algorithm>

ContentFileMeta::ContentFileMeta(const std::filesystem::path &path)
  : logger(Logger::getLogger(C::FILE_READER)),
    cidDirectory(path),
    contentTitle(path.filename().string()) {}

ContentFileMeta::~ContentFileMeta() {
  shutdown();
}

ContentFileMeta::ContentFileMeta(ContentFileMeta && other) noexcept
  : logger(std::move(other.logger)),
    cidDirectory(other.cidDirectory),
    configFile(other.configFile),
    audioFile(std::move(other.audioFile)),
    videoFiles(std::move(other.videoFiles)),
    rtspSdpMessage(other.rtspSdpMessage),
    rtpInfo(other.rtpInfo),
    v0Images(std::move(other.v0Images)) {
  other.shutdown();
}

int ContentFileMeta::getNumberOfCamDirectories() const {
  return static_cast<int>(videoFiles.size());
}

int ContentFileMeta::getRefVideoSampleCnt() const {
  // return cam 0's V1 video files sample cnt
  if (videoFiles.find(C::REF_CAM) == videoFiles.end()) {
    throw std::runtime_error("ref cam directory not initialized!");
  }
  return static_cast<int>(videoFiles.at(C::REF_CAM).getConstVideoSampleInfoList()[0].size());
}

bool ContentFileMeta::init() {
  if (cidDirectory.empty() || !exists(cidDirectory)) {
    logger->severe("No content files or wrong content root directory!");
    return false;
  }

  bool initResult = handleCamDirectories(cidDirectory);
  if (!initResult) {
    logger->severe("Invalid cam directories! Content name : " + contentTitle);
    return false;
  }

  initResult = handleConfigFile(cidDirectory);
  if (!initResult) {
    logger->severe("Invalid SDP config! Content name : " + contentTitle);
    return false;
  }

  initResult = handleV0Images(cidDirectory);
  if (!initResult) {
    logger->severe("Invalid V0 images! Content name : " + contentTitle);
    return false;
  }

  return true;
}

void ContentFileMeta::shutdown() {
  // close all access
  try {
    for (auto& kvPair : videoFiles) {
      for (std::ifstream& videoAccess : kvPair.second.getAccessList()) {
        if (videoAccess.is_open()) {
          videoAccess.close();
        }
      }
    }
    videoFiles.clear();
    audioFile.close();
  } catch (const std::exception &e) {
    logger->severe("Dongvin, exception while shutting down FileReader!");
    std::cerr << e.what() << "\n";
  }
}

RtpInfo ContentFileMeta::getRtpInfoCopy() {
  // call Copy Constructor
  RtpInfo rtpInfoCopy(rtpInfo);
  return rtpInfoCopy;
}

std::string ContentFileMeta::getMediaInfoCopy() {
  return rtspSdpMessage;
}
std::vector<unsigned char> ContentFileMeta::getAccDataCopy() {
  return Util::readAllBytesFromFilePath(configFile);
}

std::vector<std::vector<unsigned char>> ContentFileMeta::getAllV0ImagesCopy() {
  std::vector<std::vector<unsigned char>> imageBinaryList;
  for (std::filesystem::path& imageFilePath : v0Images) {
    imageBinaryList.push_back(Util::readAllBytesFromFilePath(imageFilePath));
  }
  return imageBinaryList;
}

int ContentFileMeta::getAudioSampleSize() const {
  return static_cast<int>(audioFile.getConstMeta().size());
}

int ContentFileMeta::getVideoSampleSize() const {
  if (videoFiles.find(C::REF_CAM) != videoFiles.end()) {
    return static_cast<int>(videoFiles.at(C::REF_CAM).getConstVideoSampleInfoList()[0].size());
  } else {
    // adaptive bitrate supporting case
    for (const std::string& possibleRefCam : C::ADAPTIVE_BITRATE_REF_CAM_LIST) {
      if (videoFiles.find(possibleRefCam) != videoFiles.end()) {
        return static_cast<int>(videoFiles.at(possibleRefCam).getConstVideoSampleInfoList()[0].size());
      }
    }
  }
  return C::INVALID;
}

const AudioAccess& ContentFileMeta::getConstAudioMeta() const {
  return audioFile;
}

const std::unordered_map<std::string, VideoAccess>& ContentFileMeta::getConstVideoMeta() const {
  return videoFiles;
}

bool ContentFileMeta::handleCamDirectories(const std::filesystem::path &inputCidDirectory) {
  std::vector<std::filesystem::path> camDirectoryList;
  for (std::filesystem::path camDirectory : std::filesystem::directory_iterator(inputCidDirectory)) {
    camDirectoryList.push_back(camDirectory);
  }
  // sort in ascending order.
  std::sort(
    camDirectoryList.begin(), camDirectoryList.end(),
    [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
      return lhs.filename() < rhs.filename();
    }
  );

  for (std::filesystem::path camDirectory : camDirectoryList) {
    if (!is_directory(camDirectory)) continue;
    if (camDirectory.filename() == C::HYBRID_META_DIR) continue;
    bool result = loadStreamFilesInCamDirectories(camDirectory);
    if (!result) {
      return false;
    }
  }
  return true;
}

bool ContentFileMeta::handleConfigFile(const std::filesystem::path &inputCidDirectory) {
  for (std::filesystem::path dir : std::filesystem::directory_iterator(inputCidDirectory)) {
    if (!is_directory(dir) && dir.filename().string().find("acc") != std::string::npos) {
      // std::filesystem::path type is 'Copyable'!!
      configFile = dir;
      return true;
    }
  }
  return false;
}

bool ContentFileMeta::handleV0Images(const std::filesystem::path &inputCidDirectory) {
  for (std::filesystem::path dir : std::filesystem::directory_iterator(inputCidDirectory)) {
    if (!is_directory(dir) && dir.filename().string().find("jpg") != std::string::npos) {
      v0Images.push_back(dir);
    }
  }
  return !v0Images.empty();
}

bool ContentFileMeta::loadStreamFilesInCamDirectories(const std::filesystem::path &inputCidDirectory) {
  std::string camDirectoryName = inputCidDirectory.filename().string();
  bool isRefCam = (
      camDirectoryName == C::REF_CAM ||
      C::ADAPTIVE_BITRATE_REF_CAM_LIST.at(0).find(camDirectoryName) != std::string::npos
    );

  std::vector<std::filesystem::path> streamingFileList;
  for (std::filesystem::path camDirectory : std::filesystem::directory_iterator(inputCidDirectory)) {
    streamingFileList.push_back(camDirectory);
  }

  if (streamingFileList.empty()) {
    logger->severe("Dongvin, no streaming(.asv, .asa, ...) file available in " + inputCidDirectory.string());
  }

  int fileCnt = 0;
  for (std::filesystem::path file : streamingFileList) {
    if (!is_directory(file)) fileCnt++;
  }
  if (fileCnt > C::FILE_NUM_LIMIT) {
    logger->severe("Dongvin, not support alphaview 2.0 contents!");
    return false;
  }

  std::filesystem::path config; // default value of config.filename().string() is "" empty string!
  std::filesystem::path audio;
  std::vector<std::filesystem::path> videos;
  for (std::filesystem::path f : streamingFileList) {
    if (f.filename().string().find(C::ASC) != std::string::npos) {
      if (!config.filename().string().empty()) {
        logger->warning("Dongvin, multiple asc detected. init by last one.");
      }
      // std::filesystem::path type is copyable.
      config = f;
    } else if (f.filename().string().find(C::ASA) != std::string::npos) {
      if (!audio.filename().string().empty()) {
        logger->warning("Dongvin, multiple asa detected. init by last one.");
      }
      audio = f;
    } else if (f.filename().string().find(C::ASV) != std::string::npos) {
      videos.push_back(f);
    }
  }
  if (
      videos.empty() ||
      (isRefCam && (config.filename().string().empty() || audio.filename().string().empty())) ||
      (!isRefCam && (!config.filename().string().empty() || !audio.filename().string().empty()))
    ) {
    logger->severe("No .asv file or wrong place in " + inputCidDirectory.filename().string());
    return false;
  }

  if (!config.filename().string().empty()) {
    loadRtspRtpConfig(config);
  }
  if (!audio.filename().string().empty()) {
    loadRtpAudioMetaData(audio, audioFile);
  }
  if (!videos.empty()) {
    loadRtpVideoMetaData(inputCidDirectory, videos);
  }
  return true;
}

void ContentFileMeta::loadRtspRtpConfig(const std::filesystem::path &rtspConfig) {
  if (rtspConfig.empty()) {
    return;
  }
  std::vector<unsigned char> rtspConfigBytes = Util::readAllBytesFromFilePath(rtspConfig);
  std::string data = std::string(rtspConfigBytes.begin(), rtspConfigBytes.end());
  std::vector<std::string> cfgs = Util::splitToVecByString(data, C::RTSP_MSG_DELIMITER);

  std::vector<std::string> CONFIG_KEYS = {
    C::GOP_KEY, C::MEDIA_INFO_KEY, C::SSRC_KEY, C::SEQ_KEY,
    C::TIMESTAMP_KEY, C::FRAME_COUNT_KEY, C::PLAY_TIME_KEY
  };

  for (std::string msg : cfgs) {
    for (std::string key : CONFIG_KEYS) {
      if (msg.find(key) != std::string::npos) {
        if (key == C::MEDIA_INFO_KEY) {
          rtspSdpMessage = msg;
        } else {
          rtpInfo.kv.insert({key, getValues(msg, key)});
        }
      }
    }// inner

    if (msg.rfind("m=video", 0) == 0) {
      rtspSdpMessage += msg;
    }
  }//outer
}

std::vector<int64_t> ContentFileMeta::getValues(std::string inputMsg, std::string inputKey) {
  std::string midVal = Util::splitToVecByString(inputMsg, inputKey)[1];
  std::vector<std::string> v = Util::splitToVecBySingleChar(midVal, ',');
  std::vector<int64_t> values;
  for (auto i =0; i < v.size(); ++i) {
    if (v[i].find("null") != std::string::npos) {
      continue;
    }
    values.push_back(static_cast<int64_t>(std::stoll(v[i])));
  }
  return values;
}

void ContentFileMeta::loadRtpAudioMetaData(
    const std::filesystem::path &inputAudio, AudioAccess& inputAudioFile
  ) {
  inputAudioFile.getAccess().open(inputAudio, std::ios::in | std::ios::binary);
  int64_t audioFileSize = Util::getFileSize(inputAudio);
  std::vector<unsigned char> metaData = readMetaData(audioFileSize, inputAudioFile.getAccess());
  std::vector<int16_t> sizes = getSizes(metaData);

  int64_t offset = 0;
  for (const int16_t size : sizes) {
    inputAudioFile.getMeta().emplace_back(size, offset);
    offset += size;
  }
  showAudioMinMaxSize(inputAudioFile.getConstMeta());

  inputAudioFile.getAccess().close();
}

void ContentFileMeta::showAudioMinMaxSize(const std::vector<AudioSampleInfo> &audioMetaData) {
  std::vector<int> lenList;
  for (auto i = 0; i < audioMetaData.size(); ++i) {
    AudioSampleInfo aInfo = audioMetaData[i];
    if (aInfo.len == 0) continue; // dummy info
    lenList.push_back(aInfo.len);
  }
  // int sort ascending
  std::sort(lenList.begin(), lenList.end());
  int64_t min = lenList[0];
  int64_t max = lenList[lenList.size() - 1];
  logger->info(
    "Dongvin, id : " + contentTitle + ", audio (min, max)=("
    + std::to_string(min) + "," + std::to_string(max) + ")"
  );
}

void ContentFileMeta::loadRtpVideoMetaData(
const std::filesystem::path& inputCamDir, std::vector<std::filesystem::path>& videos
) {
  VideoAccess va{};

  // open ifstreams for member viedos
  std::sort(
    videos.begin(), videos.end(),
    [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
      return lhs.filename() < rhs.filename();
    }
  );
  for (const std::filesystem::path& videoPath : videos) {
    va.getAccessList().emplace_back(videoPath, std::ios::in | std::ios::binary);
  }

  // check
  if (videos.size() != va.getConstAccessList().size()) {
    throw std::runtime_error("opening video reading ifstreams failed!");
  }
  for (auto i = 0; i < videos.size(); ++i) {
    if (!va.getConstAccessList()[i].is_open()) {
      throw std::runtime_error("opening video reading ifstreams failed! filename : " + videos[i].filename().string());
    }
  }

  // open Video Sample meta data
  int memberVideoId = 0;
  for (std::ifstream& access : va.getAccessList()) {
    if (access.is_open()) {
      auto& videoSampleMetaList = va.getVideoSampleInfoList();
      int64_t videoFileSize = Util::getFileSize(videos.at(memberVideoId));
      loadRtpMemberVideoMetaData(videoFileSize, access, videoSampleMetaList, memberVideoId);
      memberVideoId++;
    } else {
      logger->severe("Failed to open video reading ifstream!");
      throw std::runtime_error("Failed to open video reading ifstream!");
    }
  }

  // std::ifstream object in VideoAccess should never be user after initializing video sample meta.
  va.clearAllIFStreams();
  videoFiles.insert({inputCamDir.filename().string(), std::move(va)});
}

void ContentFileMeta::loadRtpMemberVideoMetaData(
    int64_t videoFileSize,
    std::ifstream &inputIfstream,
    std::vector<std::vector<VideoSampleInfo>>& input2dMetaList,
    int memberId
) {
  input2dMetaList.emplace_back();

  std::vector<unsigned char> metaData = readMetaData(videoFileSize, inputIfstream);
  std::vector<int16_t> sizes = getSizes(metaData);

  int sampleCount = 0;
  int64_t offset = 0;
  if (rtpInfo.kv.find(C::GOP_KEY) == rtpInfo.kv.end()) {
    throw std::runtime_error("Failed to find gop key! : loadRtpMemberVideoMetaData");
  }
  std::vector<int64_t> gops = rtpInfo.kv[C::GOP_KEY];
  int gop = static_cast<int>(gops[0]);

  for (const int16_t size : sizes) { // size must start with -1, refer to transcoder app.
    if (size == C::INVALID) {
      VideoSampleInfo newVSampleInfo{};
      newVSampleInfo.setOffset(offset);
      newVSampleInfo.setFlag( (sampleCount % gop) == 0 ? C::KEY_FRAME_FLAG : C::P_FRAME_FLAG );
      input2dMetaList.at(input2dMetaList.size() - 1).push_back(newVSampleInfo);
      sampleCount++;
      continue;
    }

    std::vector<VideoSampleInfo>& sampleInfoList = input2dMetaList.at(input2dMetaList.size() - 1);
    VideoSampleInfo& latestVideoSampleInfo = sampleInfoList.at(sampleInfoList.size() - 1);

    latestVideoSampleInfo.getMetaInfoList().emplace_back(size, offset);
    int prevSize = latestVideoSampleInfo.getSize();
    latestVideoSampleInfo.setSize(prevSize + size);

    offset += size;
  }// for

  for (auto& videoSampleInfo : input2dMetaList.at(input2dMetaList.size() - 1)) {
    videoSampleInfo.initCompleted();
  }

  showVideoMinMaxSize(input2dMetaList.at(input2dMetaList.size() - 1), memberId);
}

void ContentFileMeta::showVideoMinMaxSize(
  const std::vector<VideoSampleInfo> &videoMetaData, int memberId
) {
  std::vector<int> lenList;
  for (auto i=0; i < videoMetaData.size(); ++i) {
    VideoSampleInfo vInfo = videoMetaData[i];
    if (vInfo.getSize()==0) continue; // dummy info
    lenList.push_back(vInfo.getSize());
  }
  std::sort(lenList.begin(), lenList.end());
  int64_t min = lenList[0];
  int64_t max = lenList[lenList.size() - 1];
  logger->info(
    "Dongvin, id : " + contentTitle + ", memberId: " + std::to_string(memberId)
    + ", video (min, max)=(" + std::to_string(min) + "," + std::to_string(max) + ")"
  );
}

std::vector<std::vector<VideoSampleInfo>> ContentFileMeta::getVideoMetaInternal(std::string camId) {
  std::vector<std::vector<VideoSampleInfo>> vMetaList;
  for (std::vector<VideoSampleInfo> vInfo : vMetaList) {
    vMetaList.push_back(vInfo);
  }
  return vMetaList;
}

std::vector<unsigned char> ContentFileMeta::readMetaData(
  int64_t fileSize, std::ifstream &inputFileStream
) {
  if (!inputFileStream.is_open()) {
    throw std::runtime_error("Failed to open file! size : " + std::to_string(fileSize));
  }
  int64_t size = fileSize;
  int64_t pos = size - C::META_LEN_BYTES;
  inputFileStream.seekg(pos, std::ios::beg);

  // read 4 bytes.
  std::vector<unsigned char> metaLenBuf(C::META_LEN_BYTES);
  inputFileStream.read(reinterpret_cast<std::istream::char_type*>(metaLenBuf.data()), C::META_LEN_BYTES);
  int32_t metaLen = Util::convertToInt32(metaLenBuf);
  if (inputFileStream.fail() || metaLen <= 0) {
    throw std::runtime_error("reading meta data length failed! : metaLen : " + std::to_string(metaLen));
  }

  pos -= metaLen;
  inputFileStream.seekg(pos, std::ios::beg);

  std::vector<unsigned char> metaBuf(metaLen);
  inputFileStream.read(reinterpret_cast<std::istream::char_type*>(metaBuf.data()), metaLen);
  if (inputFileStream.fail()) {
    throw std::runtime_error("reading meta data failed!");
  }

  return metaBuf;
}

std::vector<int16_t> ContentFileMeta::getSizes(std::vector<unsigned char>& metaData) {
  std::vector<int16_t> sizes;
  sizes.reserve(metaData.size() / 2); // reserve memory for efficiency

  for (auto i = 0; i < metaData.size(); i += 2) {
    // execute bitwise operation to two bytes(big-endian)
    int16_t value = (static_cast<int16_t>(metaData[i]) << 8) | static_cast<int16_t>(metaData[i + 1]);
    sizes.push_back(value);
  }

  return sizes;
}