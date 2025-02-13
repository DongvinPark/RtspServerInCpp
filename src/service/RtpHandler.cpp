#include "../include/RtpHandler.h"

RtpHandler::RtpHandler(std::string sessionId,
  std::weak_ptr<Session> parentSessionPtr,
  std::weak_ptr<AcsHandler> acsHandlerPtr) {}

RtpHandler::~RtpHandler() {}


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
