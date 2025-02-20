#include "../include/RtpHandler.h"

RtpHandler::RtpHandler(
  std::string inputSessionId,
  std::weak_ptr<Session> inputParentSessionPtr,
  std::weak_ptr<AcsHandler> inputAcsHandlerPtr
  ) : logger(Logger::getLogger(C::RTP_HANDLER)),
      sessionId(inputSessionId),
      parentSessionPtr(inputParentSessionPtr),
      acsHandlerPtr(inputAcsHandlerPtr){}

RtpHandler::~RtpHandler() {
  // close all video and audio std::ifstream.
  for (auto&[camId, ifstreamVec] : camIdVideoFileStreamMap) {
    for (auto& access : ifstreamVec) {
      if (access.is_open()) access.close();
    }
  }
  if (audioFileStream.is_open()) audioFileStream.close();
}

[[nodiscard]] bool RtpHandler::openAllFileStreamsForVideoAndAudio() {
  if (auto sessionPtr = parentSessionPtr.lock()) {
    // video
    int camDirCnt = sessionPtr->getNumberOfCamDirectories();
    const std::string& contentRootDir = sessionPtr->getContentRootPath();
    const std::string& contentTitle = sessionPtr->getContentTitle();

    const std::string contentPath = contentRootDir + DIR_SEPARATOR + contentTitle;

    for (int camId = 0; camId < camDirCnt; ++camId) {
      camIdVideoFileStreamMap.insert({camId, std::vector<std::ifstream>{}});

      const std::string camPath = contentPath + DIR_SEPARATOR + C::CAM_ID_LIST[camId];

      auto& streamVec = camIdVideoFileStreamMap.at(camId);
      // front Video in current cam
      streamVec.emplace_back(camPath + DIR_SEPARATOR + "V1H.asv", std::ios::in | std::ios::binary);
      // rear Video in current cam
      streamVec.emplace_back(camPath + DIR_SEPARATOR + "V2H.asv", std::ios::in | std::ios::binary);

      for (const auto& access : camIdVideoFileStreamMap.at(camId)) {
        if (!access.is_open()){
          logger->severe("Dongvin, failed to open video file! : ");
          return false;
        }
      }
    }

    // audio
    const std::string audioFilePath = contentPath + DIR_SEPARATOR + C::CAM_ID_LIST[0] + DIR_SEPARATOR + "V.asa";
    audioFileStream.open(audioFilePath, std::ios::in | std::ios::binary);
    if (!audioFileStream.is_open()){
      logger->severe("Dongvin, failed to open audio file! path : " + audioFilePath);
      return false;
    }

    return true;
  }
}

std::unique_ptr<Buffer> RtpHandler::readFirstRtpOfCurVideoSample(int sampleNo, int64_t offset, int64_t len) noexcept {
  std::vector<unsigned char> buf(len);

  const auto camIt = camIdVideoFileStreamMap.find(0);
  if (camIt == camIdVideoFileStreamMap.end() || camIt->second.empty()) {
    logger->severe("Dongvin, video file stream map is empty or invalid!");
    return nullptr;
  }

  std::ifstream& videoFileReadingStream = camIt->second[0]; // Ensure there is at least one element

  if (!videoFileReadingStream.is_open()) {
    logger->severe("Dongvin, video file stream is not open!");
    return nullptr;
  }

  videoFileReadingStream.seekg(offset, std::ios::beg);
  videoFileReadingStream.read(reinterpret_cast<std::istream::char_type*>(buf.data()), len);

  if (videoFileReadingStream.gcount() != len) {
    logger->severe("Dongvin, failed to read first rtp of current video sample! sampleNo : " + std::to_string(sampleNo));
    return nullptr;
  }

  // discard every byte except first rtp from buf
  const int rtpLen = Util::getRtpPacketLength(buf[2], buf[3]);
  buf.resize(4 + rtpLen);
  auto bufferPtr = std::make_unique<Buffer>(buf, 0, buf.size());
  return bufferPtr;
}

void RtpHandler::readVideoSample(
  VideoSampleRtp* videoSampleRtpPtr,
  const VideoSampleInfo& curFrontVideoSampleInfo,
  const VideoSampleInfo& curRearVideoSampleInfo,
  int camId,
  int vid,
  int sampleNo,
  HybridMetaMapType &hybridMetaMap
) noexcept {
  // TODO : need to implement hybrid D & S later.

  const auto camIt = camIdVideoFileStreamMap.find(camId);
  if (camIt == camIdVideoFileStreamMap.end() || camIt->second.size() < 2) {
    logger->severe("Dongvin, invalid camId or insufficient video file streams! camId: " + std::to_string(camId));
    return;
  }

  std::ifstream& frontVideoFileReadingStream = camIt->second[0];
  std::ifstream& rearVideoFileReadingStream = camIt->second[1];

  if (!frontVideoFileReadingStream.is_open() || !rearVideoFileReadingStream.is_open()) {
    logger->severe("Dongvin, video file stream is not open!");
  }

  frontVideoFileReadingStream.seekg(curFrontVideoSampleInfo.getOffset(), std::ios::beg);
  rearVideoFileReadingStream.seekg(curRearVideoSampleInfo.getOffset(), std::ios::beg);

  if (
      !frontVideoFileReadingStream.read(
          reinterpret_cast<std::ifstream::char_type *>(videoSampleRtpPtr->data),
          curFrontVideoSampleInfo.getSize()
        ) ||
      !rearVideoFileReadingStream.read(
        reinterpret_cast<std::ifstream::char_type *>(videoSampleRtpPtr->data + curFrontVideoSampleInfo.getSize()),
        curRearVideoSampleInfo.getSize()
        )
    ) {
    logger->severe("Dongvin, failed to read video sample! sample no : " + std::to_string(sampleNo));
  } else if (auto sessionPtr = parentSessionPtr.lock()) {
    // reading successful. need to enqueue all rtp packet's meta.
    videoSampleRtpPtr->length = curFrontVideoSampleInfo.getSize() + curRearVideoSampleInfo.getSize();

    const auto& frontRtpMetaVec = curFrontVideoSampleInfo.getConstMetaInfoList();
    const auto& rearRtpMetaVec = curRearVideoSampleInfo.getConstMetaInfoList();
    videoSampleRtpPtr->refCount = static_cast<int>(frontRtpMetaVec.size() + rearRtpMetaVec.size());

    int offsetForFrontVRtp = 0;
    for (int i=0; i<frontRtpMetaVec.size(); ++i) {
      const auto& rtpMeta = curFrontVideoSampleInfo.getConstMetaInfoList()[i];
      // enqueue front v's all rtp
      auto* rtpInfo = new RtpPacketInfo();
      rtpInfo->flag = C::VIDEO_ID;
      rtpInfo->videoSamplePtr = videoSampleRtpPtr;
      rtpInfo->audioSamplePtr = nullptr;
      rtpInfo->offset = offsetForFrontVRtp;
      rtpInfo->length = rtpMeta.len;
      offsetForFrontVRtp += rtpMeta.len;
      sessionPtr->enqueueRtpInfo(rtpInfo);
    }

    int offsetForRearVRtp = curFrontVideoSampleInfo.getSize();
    for (int i=0; i<rearRtpMetaVec.size(); ++i) {
      const auto& rtpMeta = curRearVideoSampleInfo.getConstMetaInfoList()[i];
      // enqueue rear v's all rtp
      auto* rtpInfo = new RtpPacketInfo();
      rtpInfo->flag = C::VIDEO_ID;
      rtpInfo->videoSamplePtr = videoSampleRtpPtr;
      rtpInfo->audioSamplePtr = nullptr;
      rtpInfo->offset = offsetForRearVRtp;
      rtpInfo->length = rtpMeta.len;
      offsetForRearVRtp += rtpMeta.len;
      sessionPtr->enqueueRtpInfo(rtpInfo);
    }
  } else {
    logger->severe("Dongvin, failed to get session ptr! RtpHandler::readVideoSample");
  }
}

std::unique_ptr<Buffer> RtpHandler::readFirstRtpOfCurAudioSample(int sampleNo, int64_t offset, int64_t len) noexcept {
  if (!audioFileStream.is_open()) {
    logger->severe("Dongvin, audio file stream is not open!");
    return nullptr;
  }

  std::vector<unsigned char> buf(len);
  audioFileStream.seekg(offset, std::ios::beg);
  audioFileStream.read(reinterpret_cast<std::istream::char_type*>(buf.data()), len);

  if (audioFileStream.gcount() != len) {
    logger->severe("Dongvin, failed to read first rtp of current audio sample! sampleNo : " + std::to_string(sampleNo));
    return nullptr;
  }

  auto bufferPtr = std::make_unique<Buffer>(buf, 0, buf.size());
  return bufferPtr;
}

void RtpHandler::readAudioSample(
  AudioSampleRtp* audioSampleRtpPtr,
  const int sampleNo,
  const int64_t offset,
  const int len,
  HybridMetaMapType& hybridMetaMap
) noexcept {
  // TODO : need to implement hybrid D & S later.
  if (!audioFileStream.is_open()) {
    logger->severe("Dongvin, audio file stream is not open!");
    audioSampleRtpPtr->length = C::INVALID;
    return;
  }

  audioFileStream.seekg(offset, std::ios::beg);
  if (!audioFileStream.read(reinterpret_cast<std::ifstream::char_type *>(audioSampleRtpPtr->data), len)) {
    audioSampleRtpPtr->length = C::INVALID;
    logger->severe("Dongvin, failed to read audio sample! sample no : " + std::to_string(sampleNo));
  } else if (auto sessionPtr = parentSessionPtr.lock()) {
    audioSampleRtpPtr->length = len;
    auto* rtpInfo = new RtpPacketInfo();
    rtpInfo->flag = C::AUDIO_ID;
    rtpInfo->videoSamplePtr = nullptr;
    rtpInfo->audioSamplePtr = audioSampleRtpPtr;
    rtpInfo->offset = 0;
    rtpInfo->length = len;
    sessionPtr->enqueueRtpInfo(rtpInfo);
  } else {
    logger->severe("Dongvin, failed to get session ptr! RtpHandler::readAudioSample");
  }
}