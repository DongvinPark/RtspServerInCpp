#ifndef ACSHANDLER_H
#define ACSHANDLER_H

#include "../include/ContentFileMeta.h"
#include "../include/Session.h"
#include "../include/RtpInfo.h"
#include "../include/ReadInfo.h"
#include "../include/Buffer.h"
#include "../include/AVSampleBuffer.h"

class Session;
class RtspHandler;
class RtpHandler;
struct FrontVideoSampleRtps;
struct RearVideoSampleRtps;
struct AudioSampleRtp;

class AcsHandler {
public:
  explicit AcsHandler(
    std::string sessionId,
    std::weak_ptr<Session> parentSessionPtr,
    ContentsStorage& parentContentsStorage
  );
  ~AcsHandler();

  void updateRtpRemoteCnt(int cnt);
  void updateCurSampleNo(int mediaType, int idx);
  int getCamId();
  void shutdown();

  void setChannel(int streamId, std::vector<int> ch);
  void initUserRequestingPlaytime(std::vector<float> timeS);
  void setRtpInfo(RtpInfo inputRtpInfo);
  bool setReaderAndContentTitle(ContentFileMeta& inputReader, std::string contentTitle);
  int getLastVideoSampleNumber();
  int getLastAudioSampleNumber();
  std::vector<unsigned char> getAccData();
  std::vector<std::vector<unsigned char>> getAllV0Images();
  void getNextVideoSample(FrontVideoSampleRtps* videoSampleRtpsPtr, RearVideoSampleRtps* rearVSampleRtpsPtr);
  void getNextAudioSample(AudioSampleRtp* audioSampleRtp);
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
  int findNextSampleForSwitchingAudio(std::vector<int64_t> timeInfo);
  void findNextSampleForSwitchingVideo(int nextVid, std::vector<int64_t> timeInfo);
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

  std::unordered_map<int, ReadInfo> sInfo{};
  RtpInfo rtpInfo;
  int camId = C::ZERO;

  int videoRtpRemoveCnt = C::ZERO;
};

#endif //ACSHANDLER_H
