#include "../include/AcsHandler.h"

#include "../../constants/Util.h"

AcsHandler::AcsHandler(
  std::string inputSessionId,
  std::weak_ptr<Session> inputParentSessionPtr,
  ContentsStorage& inputContentsStorage
)
: logger(Logger::getLogger(C::ACS_HANDLER)),
  sessionId(inputSessionId),
  parentSessionPtr(inputParentSessionPtr),
  contentsStorage(inputContentsStorage){}

AcsHandler::~AcsHandler() {}

void AcsHandler::updateRtpRemoteCnt(int cnt) {
}

void AcsHandler::updateCurSampleNo(int mediaType, int idx) {
}

int AcsHandler::getCamId() {
}

void AcsHandler::shutdown() {
}

std::weak_ptr<FileReader> AcsHandler::getFileReaderPtr() {
}

void AcsHandler::setChannel(int streamId, std::vector<int> ch) {
}

void AcsHandler::initUserRequestingPlaytime(std::vector<float> timeS) {
}

void AcsHandler::setRtpInfo(RtpInfo &rtpInfo) {
}

void AcsHandler::setReader(std::weak_ptr<FileReader> reader) {
}

int AcsHandler::getLastVideoSampleNumber() {
}

int AcsHandler::getLastAudioSampleNumber() {
}

std::vector<unsigned char> AcsHandler::getAccData() {
}

std::vector<std::vector<unsigned char>> AcsHandler::getAllV0Images() {
}

std::unique_ptr<AVSampleBuffer> AcsHandler::getNextSample() {
}

bool AcsHandler::isDone(int streamId) {
}

int64_t AcsHandler::getUnitFrameTimeUs(int streamId) {
}

std::string AcsHandler::getMediaInfo() {
}

std::vector<int64_t> AcsHandler::getSsrc() {
}

int AcsHandler::getMainVideoNumber() {
}

int AcsHandler::getMaxCamNumber() {
}

std::vector<int> AcsHandler::getInitialSeq() {
}

std::vector<int64_t> AcsHandler::getTimestamp() {
}

int64_t AcsHandler::getTimestamp0(int streamId) {
  return rtpInfo.kv.at(C::TIMESTAMP_KEY)[streamId];
}

int64_t AcsHandler::getUnitFrameCount(int streamId) {
  return rtpInfo.kv.at(C::FRAME_COUNT_KEY)[streamId];
}

std::vector<int64_t> AcsHandler::getUnitFrameCount() {
  return rtpInfo.kv.at(C::FRAME_COUNT_KEY);
}

std::vector<std::string> AcsHandler::getStreamUrls() {
  return rtpInfo.urls;
}

void AcsHandler::setStreamUrl(int streamId, std::string url) {
  rtpInfo.urls[streamId] = url;
}

std::vector<int64_t> AcsHandler::getPlayTimeUs() {
  return rtpInfo.kv.at(C::PLAY_TIME_KEY);
}

int64_t AcsHandler::getPlayTimeUs(int streamId) {
  return rtpInfo.kv.at(C::PLAY_TIME_KEY)[streamId];
}

std::vector<int64_t> AcsHandler::getGop() {
  return rtpInfo.kv.at(C::GOP_KEY);
}

void AcsHandler::setCamId(int inputCamId) {
  if (this->camId == inputCamId) return;
  logger->info3("Dongvin, sessionId : " + sessionId + ", camera is changed to " + std::to_string(inputCamId));
  this->camId = inputCamId;
}

void AcsHandler::findNextSampleForSwitching(int vid, std::vector<int64_t> timeInfo) {
}

std::unique_ptr<Buffer> AcsHandler::get1stRtpOfRefSample(int streamId, int sampleNo) {
}

void AcsHandler::checkTimestamp(int streamId, ReadInfo &readInfo) {
}

int AcsHandler::findKeySampleNumber(int streamId, int64_t timeUs, int way) {
}

int AcsHandler::findSampleNumber(int streamId, int64_t timeUs) {
  if (timeUs < 0 || timeUs >= getPlayTimeUs(streamId)){
    return sInfo.at(streamId).maxSampleNo;
  }
  return getSampleNumber(streamId, timeUs);
}

int AcsHandler::getSampleNumber(int streamId, int64_t timeUs) {
  int64_t t0 = sInfo.at(streamId).unitTimeUs;
  return static_cast<int>(std::round(static_cast<double>(timeUs)/static_cast<double>(t0)));
}

int AcsHandler::findNextSampleForSwitchingAudio(std::vector<int64_t> timeInfo) {
}

void AcsHandler::findNextSampleForSwitchingVideo(int nextVid, std::vector<int64_t> timeInfo) {
}

std::vector<int64_t> AcsHandler::getUnitFrameTimeUs() {
  std::vector<int64_t> frameCount = rtpInfo.kv.at(C::FRAME_COUNT_KEY);
  return {
    (1000000*frameCount[0]/C::H265_CLOCK_RATE), // us
    (1000000*frameCount[1]/C::AAC_CLOCK_RATE)
  };
}

// this time is stream's presentation time.
// input: timestamp is the time that had generated at the acs creation.
// output: the presentation time = play time.
int64_t AcsHandler::getSamplePresentationTimeUs(int streamId, int64_t timestamp) {
  // we know the clock for video 90kHz (for h.265) and for audio 48kHz
  // actually this information is given by the message of ANNOUNCE.
  int clock = streamId == C::VIDEO_ID ? C::H265_CLOCK_RATE : C::AAC_CLOCK_RATE;
  int elapsedTimeCount = static_cast<int>(timestamp - getTimestamp0(streamId));
  return (1000000L*elapsedTimeCount/clock);
}

int64_t AcsHandler::getSamplePresentationTimeUs(int streamId, int sampleTimeIndex) {
  int clock = streamId == C::VIDEO_ID ? C::H265_CLOCK_RATE : C::AAC_CLOCK_RATE;
  int unitFrameCount = sInfo.at(streamId).unitFrameCount;
  return sampleTimeIndex*unitFrameCount*1000000L/clock;
}

int AcsHandler::getSampleTimeIndex(int streamId, int64_t timestamp) {
  return static_cast<int>( (timestamp-getTimestamp0(streamId))/sInfo.at(streamId).unitFrameCount );
}

int64_t AcsHandler::getTimestamp(int sampleNo) {
  if (auto readerPtr = fileReaderPtr.lock()) {
    if (auto sessionPtr = parentSessionPtr.lock()) {
      return Util::findTimestampInVideoSample(
        readerPtr->readRefVideoSampleWithLock(sampleNo, sessionPtr->getHybridMetaMap())[0]
      );
    } else return C::INVALID_BYTE;
  } else return C::INVALID_OFFSET;
}
