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

  void stopVideo();
  void stopAudio();

  AudioSample& readAudioSampleWithLock(
    int sampleNo, HybridMetaMapType &hybridMetaMap
  ) noexcept;


  std::vector<VideoSample>& readRefVideoSampleWithLock(
    int sampleNo, HybridMetaMapType &hybridMetaMap
  ) noexcept;

  std::vector<VideoSample>& readVideoSampleWithLock(
    int camId,
    int vid,
    int memberId,
    int sampleNo,
    HybridMetaMapType &hybridMetaMap
  ) noexcept;


  std::vector<VideoSample>& readVideoSampleInternalWithLock(
    int camId,
    VideoAccess &va,
    int sampleNo,
    HybridMetaMapType &hybridMetaMap
  ) noexcept;

private:
};

#endif //RTPHANDLER_H
