#ifndef RTPHANDLER_H
#define RTPHANDLER_H

#include "../include/Session.h"
#include "../include/VideoSample.h"
#include "../include/AudioSample.h"
#include "../include/VideoAccess.h"

class Session;
class AcsHandler;

using HybridMetaMapType
    = std::unordered_map<int, std::unordered_map<std::string, std::unordered_map<int, HybridSampleMeta>>>;

class RtpHandler {
public:
  explicit RtpHandler(
    std::string sessionId, std::weak_ptr<Session> parentSessionPtr, std::weak_ptr<AcsHandler> acsHandlerPtr
  );
  ~RtpHandler();

  AudioSample& readAudioSample(
    int sampleNo, HybridMetaMapType &hybridMetaMap
  ) noexcept;


  std::vector<VideoSample>& readRefVideoSample(
    int sampleNo, HybridMetaMapType &hybridMetaMap
  ) noexcept;

  std::vector<VideoSample>& readVideoSample(
    int camId,
    int vid,
    int memberId,
    int sampleNo,
    HybridMetaMapType &hybridMetaMap
  ) noexcept;


  std::vector<VideoSample>& readVideoSampleInternal(
    int camId,
    VideoAccess &va,
    int sampleNo,
    HybridMetaMapType &hybridMetaMap
  ) noexcept;

private:
};

#endif //RTPHANDLER_H
