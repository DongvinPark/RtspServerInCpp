#ifndef STREAMHANDLER_H
#define STREAMHANDLER_H

#include <unordered_set>

#include "../include/ContentFileMeta.h"
#include "../include/Session.h"
#include "../include/RtpInfo.h"
#include "../include/ReadInfo.h"
#include "../include/Buffer.h"

class Session;
class RtspHandler;
class RtpHandler;

struct Sample;

class StreamHandler {
public:
  explicit StreamHandler(
    std::string sessionId,
    std::weak_ptr<Session> parentSessionPtr,
    ContentsStorage& parentContentsStorage
  );
  ~StreamHandler();

  void updateRtpRemoteCnt(int cnt);
  void updateCurSampleNo(int mediaType, int idx);
  int getCamId();
  void shutdown();

  void setChannel(int streamId, std::vector<int> ch);
  void initUserRequestingPlaytime(std::vector<float> timeS);
  [[nodiscard]] bool setRtpInfo(RtpInfo inputRtpInfo);
  bool setReaderAndContentTitle(ContentFileMeta& inputReader, std::string contentTitle);
  int getLastVideoSampleNumber();
  int getLastAudioSampleNumber();
  std::vector<unsigned char> getAccData();
  std::vector<std::vector<unsigned char>> getAllV0Images();
  bool setVideoAudioSampleMetaDataCache(const std::string& contentTitle);
  void getNextVideoSample();
  void getNextAudioSample();
  bool isDone(int streamId);
  int64_t getUnitFrameTimeUs(int streamId);
  std::string getMediaInfo();
  std::vector<int64_t> getSsrc();
  int getMainVideoNumber();
  int getMaxCamNumber();
  std::vector<int> getInitialSeq();
  std::vector<int64_t> getTimestamp();
  int64_t getTimestamp0(int streamId);
  int64_t getUnitFrameCount(int streamId);
  std::vector<int64_t> getUnitFrameCount();
  std::vector<std::string> getStreamUrls();
  void setStreamUrl(int streamId, std::string url);
  std::vector<int64_t> getPlayTimeUs();
  int64_t getPlayTimeUs(int streamId);
  std::vector<int64_t> getGop();
  void setCamId(int camId);
  void findNextSampleForSwitching(int vid, std::vector<int64_t> timeInfo);

private:
  std::unique_ptr<Buffer> get1stRtpOfRefSample(int streamId, int sampleNo);
  void checkTimestamp(int streamId, ReadInfo& readInfo);
  int findKeySampleNumber(int streamId, int64_t timeUs, int way);
  int findSampleNumber(int streamId, int64_t timeUs);
  int getSampleNumber(int streamId, int64_t timeUs);
  void findNextSampleForSwitchingAudio(int64_t targetSampleNo);
  void findNextSampleForSwitchingVideo(int64_t targetSampleNo);
  std::vector<int64_t> getUnitFrameTimeUs();
  int64_t getSamplePresentationTimeUs(int streamId, int64_t timestamp);
  int64_t getSamplePresentationTimeUs(int streamId, int sampleTimeIndex);
  int getSampleTimeIndex(int streamId, int64_t timestamp);
  int64_t getTimestamp(int sampleNo);

  std::shared_ptr<Logger> logger;
  std::string sessionId;
  std::weak_ptr<Session> parentSessionPtr;
  ContentsStorage& contentsStorage;
  std::string contentTitle = C::EMPTY_STRING;

  // cam 0 meta cache
  const std::vector<VideoSampleInfo>* cachedCam0frontVSampleMetaListPtr = nullptr;
  const std::vector<VideoSampleInfo>* cachedCam0rearVSampleMetaListPtr = nullptr;

  // cam 1 meta cache
  const std::vector<VideoSampleInfo>* cachedCam1frontVSampleMetaListPtr = nullptr;
  const std::vector<VideoSampleInfo>* cachedCam1rearVSampleMetaListPtr = nullptr;

  // cam 2 meta cache
  const std::vector<VideoSampleInfo>* cachedCam2frontVSampleMetaListPtr = nullptr;
  const std::vector<VideoSampleInfo>* cachedCam2rearVSampleMetaListPtr = nullptr;

  // audio meta cache
  const std::vector<AudioSampleInfo>* cachedAudioSampleMetaListPtr = nullptr;

  std::unordered_map<int, ReadInfo> sInfo{};
  RtpInfo rtpInfo;
  int camId = C::ZERO;

  int videoRtpRemoveCnt = C::ZERO;
};

#endif //STREAMHANDLER_H
