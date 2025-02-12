#include "../include/RtpHandler.h"

RtpHandler::RtpHandler(std::string sessionId,
  std::weak_ptr<Session> parentSessionPtr,
  std::weak_ptr<AcsHandler> acsHandlerPtr) {}

RtpHandler::~RtpHandler() {}

void RtpHandler::stopVideo() {}
void RtpHandler::stopAudio() {}

AudioSample & RtpHandler::readAudioSampleWithLock(int sampleNo, HybridMetaMapType &hybridMetaMap) noexcept {
}


std::vector<VideoSample> & RtpHandler::readRefVideoSampleWithLock(
  int sampleNo, HybridMetaMapType &hybridMetaMap
) noexcept {
}

std::vector<VideoSample> & RtpHandler::readVideoSampleWithLock(
  int camId,
  int vid,
  int memberId,
  int sampleNo,
  HybridMetaMapType &hybridMetaMap
) noexcept {
}


std::vector<VideoSample> & RtpHandler::readVideoSampleInternalWithLock(int camId,
  VideoAccess &va,
  int sampleNo,
  HybridMetaMapType &hybridMetaMap) noexcept {
}
