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

[[nodiscard]] bool RtpHandler::openAllFileStreams() {
  if (auto sessionPtr = parentSessionPtr.lock()) {

    // TODO : implement later
    int camDirCnt = sessionPtr->getNumberOfCamDirectories();
    const std::string& contentRootDir = sessionPtr->getContentRootPath();
    const std::string& contentTitle = sessionPtr->getContentTitle();

    return true;
  }
}

AudioSample & RtpHandler::readAudioSample(int sampleNo, HybridMetaMapType &hybridMetaMap) noexcept {
}


std::vector<VideoSample> & RtpHandler::readRefVideoSample(
  int sampleNo, HybridMetaMapType &hybridMetaMap
) noexcept {
}

std::vector<VideoSample> & RtpHandler::readVideoSample(
  int camId,
  int vid,
  int memberId,
  int sampleNo,
  HybridMetaMapType &hybridMetaMap
) noexcept {
}


std::vector<VideoSample> & RtpHandler::readVideoSampleInternal(int camId,
  VideoAccess &va,
  int sampleNo,
  HybridMetaMapType &hybridMetaMap) noexcept {
}
