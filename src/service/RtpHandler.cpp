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
  } else {
    logger->severe("Dongvin, failed to open video file! RtpHandler::openAllFileStreamsForVideoAndAudio()");
    return false;
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
  const auto camIt = camIdVideoFileStreamMap.find(camId);
  if (camIt == camIdVideoFileStreamMap.end() || camIt->second.size() < 2) {
    logger->severe("Dongvin, invalid camId or insufficient video file streams! camId: " + std::to_string(camId));
    return;
  }

  std::ifstream& frontVideoFileReadingStream = camIt->second[0];
  std::ifstream& rearVideoFileReadingStream = camIt->second[1];

  int gop = C::INVALID;
  if (auto handlerPtr = acsHandlerPtr.lock()) {
    if (const std::vector<int64_t> gopVec = handlerPtr->getGop(); gopVec.empty()) {
      logger->severe("Dongvin, fail to get gop value vector!");
      videoSampleRtpPtr->length = C::INVALID;
      return;
    } else {
      gop = static_cast<int>(gopVec[0]);
    }
  }

  if (!frontVideoFileReadingStream.is_open() || !rearVideoFileReadingStream.is_open()) {
    logger->severe("Dongvin, video file stream is not open!");
  }

  const std::string frameType = sampleNo % gop == 0 ? C::KEY_FRAME_TYPE : C::P_FRAME_TYPE;
  const std::optional<HybridSampleMeta> frontVideoHybridMeta = Util::getHybridSampleMetaSafe(
    hybridMetaMap, camId, std::to_string(C::FRONT_VIDEO_VID)+frameType, sampleNo
  );
  const std::optional<HybridSampleMeta> rearVideoHybridMeta = Util::getHybridSampleMetaSafe(
    hybridMetaMap, camId, std::to_string(C::REAR_VIDEO_VID)+frameType, sampleNo
  );

  if (auto sessionPtr = parentSessionPtr.lock()) {
    // process front video.
    int64_t nextStartOffsetForRearVideo = 0;
    if (frontVideoHybridMeta.has_value()) {
      const std::vector<unsigned char> metaData = frontVideoHybridMeta->getHybridMetaBinary(
        C::getAvptSampleQChannel(C::FRONT_VIDEO_VID), camId, C::FRONT_VIDEO_VID, frameType
      );
      for (int i = 0; i < metaData.size(); ++i) {
        videoSampleRtpPtr->data[i] = metaData[i];
      }
      (videoSampleRtpPtr->refCount) += 1;
      (videoSampleRtpPtr->length) += metaData.size();
      nextStartOffsetForRearVideo = static_cast<int64_t>(metaData.size());
      auto* rtpInfo = new RtpPacketInfo();
      rtpInfo->flag = C::VIDEO_ID;
      rtpInfo->videoSamplePtr = videoSampleRtpPtr;
      rtpInfo->audioSamplePtr = nullptr;
      rtpInfo->offset = 0;
      rtpInfo->length = metaData.size();
      sessionPtr->enqueueRtpInfo(rtpInfo);
    } else {
      // no front V sample meta for hybrid D & S. read sample from file stream.
      frontVideoFileReadingStream.seekg(curFrontVideoSampleInfo.getOffset(), std::ios::beg);
      if (
        !frontVideoFileReadingStream.read(
          reinterpret_cast<std::ifstream::char_type *>(videoSampleRtpPtr->data),
          curFrontVideoSampleInfo.getSize()
        )
      ) {
        logger->severe("Dongvin, fail to read front video sample! sample no : " + std::to_string(sampleNo));
        videoSampleRtpPtr->length = C::INVALID;
        return;
      } else {
        (videoSampleRtpPtr->length) += curFrontVideoSampleInfo.getSize();
        const auto& frontRtpMetaVec = curFrontVideoSampleInfo.getConstMetaInfoList();
        int offsetForFrontVRtp = 0;
        (videoSampleRtpPtr->refCount) += static_cast<int>(frontRtpMetaVec.size());
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
        nextStartOffsetForRearVideo = static_cast<int64_t>(curFrontVideoSampleInfo.getSize());
      }
    }

    // process rear video.
    if (rearVideoHybridMeta.has_value()) {
      const std::vector<unsigned char> metaData = rearVideoHybridMeta->getHybridMetaBinary(
        C::getAvptSampleQChannel(C::REAR_VIDEO_VID), camId, C::REAR_VIDEO_VID, frameType
      );
      for (int i = 0; i < metaData.size(); ++i) {
        videoSampleRtpPtr->data[i+nextStartOffsetForRearVideo] = metaData[i];
      }
      (videoSampleRtpPtr->refCount) += 1;
      (videoSampleRtpPtr->length) += metaData.size();
      auto* rtpInfo = new RtpPacketInfo();
      rtpInfo->flag = C::VIDEO_ID;
      rtpInfo->videoSamplePtr = videoSampleRtpPtr;
      rtpInfo->audioSamplePtr = nullptr;
      rtpInfo->offset = nextStartOffsetForRearVideo;
      rtpInfo->length = metaData.size();
      sessionPtr->enqueueRtpInfo(rtpInfo);
    } else {
      // no rear V sample meta for hybrid D & S. read sample from file stream.
      rearVideoFileReadingStream.seekg(curRearVideoSampleInfo.getOffset(), std::ios::beg);
      if (
        !rearVideoFileReadingStream.read(
          reinterpret_cast<std::ifstream::char_type *>(videoSampleRtpPtr->data + videoSampleRtpPtr->length),
          curRearVideoSampleInfo.getSize()
        )
      ) {
        logger->severe("Dongvin, fail to read rear video sample! sample no : " + std::to_string(sampleNo));
        videoSampleRtpPtr->length = C::INVALID;
        return;
      } else {
        const auto& rearRtpMetaVec = curRearVideoSampleInfo.getConstMetaInfoList();
        int offsetForRearVRtp = static_cast<int>(videoSampleRtpPtr->length);
        (videoSampleRtpPtr->length) += curRearVideoSampleInfo.getSize();
        (videoSampleRtpPtr->refCount) += static_cast<int>(rearRtpMetaVec.size());
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
      }
    }
  } else {
    logger->severe("Dongvin, faild to get session ptr! RtpHandler::readVideoSample()");
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
  if (hybridMetaMap.empty()) {
    // full streaming. need to send audio rtp packets.
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
      (audioSampleRtpPtr->refCount) += 1;
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
  } else {
    // do not send audio rtp. send only meta.
    const HybridSampleMeta audioSampleMeta(sampleNo, C::INVALID_OFFSET, C::INVALID, C::INVALID_OFFSET);
    const std::vector<unsigned char> metaData = audioSampleMeta.getHybridMetaBinary(
      C::AUDIO_ID,
      C::HYBRID_META_FACTOR_FOR_AUDIO,
      C::INVALID,
      C::KEY_FRAME_TYPE
    );
    for (int i = 0; i < metaData.size(); ++i) {
      audioSampleRtpPtr->data[i] = metaData[i];
    }
    audioSampleRtpPtr->length = metaData.size();
    (audioSampleRtpPtr->refCount) += 1;
    if (auto sessionPtr = parentSessionPtr.lock()) {
      auto* rtpInfo = new RtpPacketInfo();
      rtpInfo->flag = C::AUDIO_ID;
      rtpInfo->videoSamplePtr = nullptr;
      rtpInfo->audioSamplePtr = audioSampleRtpPtr;
      rtpInfo->offset = 0;
      rtpInfo->length = metaData.size();
      sessionPtr->enqueueRtpInfo(rtpInfo);
    } else {
      logger->severe("Dongvin, failed to get session ptr! RtpHandler::readAudioSample");
    }
  }
}































