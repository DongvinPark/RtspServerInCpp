#ifndef RTPHANDLER_H
#define RTPHANDLER_H

#include <filesystem>

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
    std::string inputSessionId,
    std::weak_ptr<Session> inputParentSessionPtr,
    std::weak_ptr<AcsHandler> inputAcsHandlerPtr
  );
  ~RtpHandler();

  [[nodiscard]] bool openAllFileStreams();

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
  std::shared_ptr<Logger> logger;
  std::string sessionId;
  std::weak_ptr<Session> parentSessionPtr;
  std::weak_ptr<AcsHandler> acsHandlerPtr;

  // usage example
  // std::ifstream& cam0FrontVFileStream = map.at(0).at(0);
  std::unordered_map<int, std::vector<std::ifstream>> camIdVideoFileStreamMap;
  std::ifstream audioFileStream;
};

#endif //RTPHANDLER_H
