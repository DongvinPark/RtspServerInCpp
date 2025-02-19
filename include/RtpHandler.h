#ifndef RTPHANDLER_H
#define RTPHANDLER_H

#include <filesystem>

#include "../constants/Util.h"
#include "../include/Session.h"
#include "../include/VideoSample.h"
#include "../include/AudioSample.h"
#include "../include/VideoAccess.h"

class Session;
class AcsHandler;

struct VideoSampleRtp;
struct AudioSampleRtp;

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

  [[nodiscard]] bool openAllFileStreamsForVideoAndAudio();

  std::unique_ptr<Buffer> readFirstRtpOfCurVideoSample(int sampleNo, int64_t offset, int64_t len) noexcept;

  void readVideoSample(
    VideoSampleRtp* videoSampleRtpPtr,
    const VideoSampleInfo& curFrontVideoSampleInfo,
    const VideoSampleInfo& curRearVideoSampleInfo,
    int camId,
    int vid,
    int sampleNo,
    HybridMetaMapType &hybridMetaMap
  ) noexcept;

  std::unique_ptr<Buffer> readFirstRtpOfCurAudioSample(int sampleNo, int64_t offset, int64_t len) noexcept;

  void readAudioSample(
    AudioSampleRtp* audioSampleRtpPtr, int sampleNo, int64_t offset, int len, HybridMetaMapType &hybridMetaMap
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
