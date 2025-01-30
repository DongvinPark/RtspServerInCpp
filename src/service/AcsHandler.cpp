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

AcsHandler::~AcsHandler(){
  shutdown();
}

void AcsHandler::updateRtpRemoteCnt(int cnt) {
  videoRtpRemoveCnt = cnt;
}

void AcsHandler::updateCurSampleNo(int mediaType, int idx) {
  if (sInfo.contains(mediaType)){
    sInfo.at(mediaType).setCurSampleNo(idx);
  } else {
    logger->severe("Dongvin, Invalid streamId on updateCurSampleNo!");
  }
}

int AcsHandler::getCamId() {
  return camId;
}

void AcsHandler::shutdown() {
  sInfo.clear();
}

void AcsHandler::setChannel(int streamId, std::vector<int> ch) {
  if (sInfo.contains(streamId)){
    ReadInfo& readInfo = sInfo[streamId];
    readInfo.channel = ch;
  } else {
    logger->severe("Dongvin, Invalid streamId on setChannel!");
  }
}

void AcsHandler::initUserRequestingPlaytime(std::vector<float> timeS) {
  if (timeS.empty()) return;

  for (auto& pair : sInfo){
    int streamId = pair.first;
    ReadInfo& info = pair.second;

    info.startSampleNo = findKeySampleNumber(streamId, Util::secToUs(timeS[0]), C::NEXT_KEY);
    info.endSampleNo = findSampleNumber(streamId, Util::secToUs(timeS[1]));
    info.curSampleNo = info.startSampleNo;

    std::unique_ptr<Buffer> rtpPtr = get1stRtpOfRefSample(streamId, info.startSampleNo);
    info.timestamp = Util::findTimestamp(*rtpPtr);
    info.refSeq0 = Util::findSequenceNumber(*rtpPtr);

    // check --> removable.
    checkTimestamp(streamId, info);

    info.unitFrameCount = static_cast<int>(getUnitFrameCount(streamId));
    logger->info("Dongvin, init read info completed. streamId : " + std::to_string(streamId));
  }
}

void AcsHandler::setRtpInfo(RtpInfo inputRtpInfo) {
  this->rtpInfo = inputRtpInfo;

  std::vector<int64_t> us = getUnitFrameTimeUs();
  sInfo.at(C::VIDEO_ID).unitTimeUs = us[C::VIDEO_ID];
  sInfo.at(C::AUDIO_ID).unitTimeUs = us[C::AUDIO_ID];
}

bool AcsHandler::setReaderAndContentTitle(FileReader& inputReader, std::string inputContentTitle) {
  if (inputContentTitle != C::EMPTY_STRING) {
    contentTitle = inputContentTitle;
  }
  std::cout << "set reader enter" << std::endl;
  auto& videoMetaMap = inputReader.getVideoMetaWithLock();
  std::cout << "passed 1 " << std::endl;
  if (videoMetaMap.empty()){
    logger->severe("Dongvin, video meta init wrong!");
    return false;
  }

  std::cout << "passed 2 before" << std::endl;
  int refVideoSampleSize = inputReader.getVideoSampleSize();
  std::cout << "passed 2 after" << std::endl;
  int audioSampleSize = inputReader.getAudioSampleSize();
  std::cout << "passed 3 " << std::endl;

  if (refVideoSampleSize != C::INVALID && audioSampleSize != C::INVALID) {
    std::cout << "true if enter : " << refVideoSampleSize << "," << audioSampleSize << std::endl;
    sInfo.emplace(C::VIDEO_ID, ReadInfo());
    sInfo.emplace(C::AUDIO_ID, ReadInfo());

    sInfo.at(C::VIDEO_ID).maxSampleNo = refVideoSampleSize;
    sInfo.at(C::AUDIO_ID).maxSampleNo = audioSampleSize;

    setRtpInfo(inputReader.getRtpInfoCopyWithLock());
    return true;
  }
  logger->severe(
    "Dongvin, video meta init wrong! refVideoSampleCnt : audioSampleCnt "
    + std::to_string(refVideoSampleSize) + "/" + std::to_string(audioSampleSize)
  );
  return false;
}

int AcsHandler::getLastVideoSampleNumber() {
  return sInfo.at(C::VIDEO_ID).maxSampleNo;
}

int AcsHandler::getLastAudioSampleNumber() {
  return sInfo.at(C::AUDIO_ID).maxSampleNo;
}

std::vector<unsigned char> AcsHandler::getAccData() {
  return contentsStorage.getCid(contentTitle).getAccDataCopyWithLock();
}

std::vector<std::vector<unsigned char>> AcsHandler::getAllV0Images() {
  return contentsStorage.getCid(contentTitle).getAllV0ImagesCopyWithLock();
}

std::unique_ptr<AVSampleBuffer> AcsHandler::getNextSample() {
}

bool AcsHandler::isDone(int streamId) {
  if (sInfo.empty()) return false;
  if (!sInfo.contains(streamId)) return false;
  return sInfo.at(streamId).isDone();
}

int64_t AcsHandler::getUnitFrameTimeUs(int streamId) {
  int targetStreamId = -1;
  if(streamId > 1) {
    targetStreamId = 0;
  } else {
    targetStreamId = streamId;
  }
  return sInfo.at(targetStreamId).unitTimeUs;
}

std::string AcsHandler::getMediaInfo() {
  return contentsStorage.getCid(contentTitle).getMediaInfoCopyWithLock();
}

std::vector<int64_t> AcsHandler::getSsrc() {
  return rtpInfo.kv.at(C::SSRC_KEY);
}

int AcsHandler::getMainVideoNumber() {
  FileReader& fileReader = contentsStorage.getCid(contentTitle);
  const auto& videoMeta = fileReader.getVideoMetaWithLock();
  return videoMeta.at(C::REF_CAM).getFileNumber();
}

int AcsHandler::getMaxCamNumber() {
  return static_cast<int>(contentsStorage.getCid(contentTitle).getVideoMetaWithLock().size());
}

std::vector<int> AcsHandler::getInitialSeq() {
  std::vector<int> seqVec;
  for (auto streamId = 0; streamId < sInfo.size(); streamId++) {
    seqVec.push_back(sInfo[streamId].refSeq0);
  }
  return seqVec;
}

std::vector<int64_t> AcsHandler::getTimestamp() {
  std::vector<int64_t> timestampVec;
  for (auto streamId = 0; streamId < sInfo.size(); streamId++) {
    timestampVec.push_back(sInfo[streamId].timestamp);
  }
  return timestampVec;
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
  FileReader& fileReader = contentsStorage.getCid(contentTitle);

  if (auto sessionPtr = parentSessionPtr.lock()) {
    if (streamId == C::VIDEO_ID) {
      return std::make_unique<Buffer>(
        fileReader.readRefVideoSampleWithLock(sampleNo, sessionPtr->getHybridMetaMap())[0]
        .getFirstRtp()
      );
    }
    // audio sample. one audio sample is consist of one rtp packet.
    return std::make_unique<Buffer>(
      fileReader.readAudioSampleWithLock(sampleNo, sessionPtr->getHybridMetaMap())
    );
  }
  logger->severe("Dongvin, fail to get Session Ptr!");
  return nullptr;
}

void AcsHandler::checkTimestamp(int streamId, ReadInfo &readInfo) {
  int64_t t = getTimestamp0(streamId) + (readInfo.startSampleNo * getUnitFrameTimeUs(streamId));
  if (t != readInfo.timestamp) {
    logger->warning("Dongvin, video timestamp calculation is wrong.");
  }
}

int AcsHandler::findKeySampleNumber(int streamId, int64_t timeUs, int way) {
  int gop = static_cast<int>(getGop()[0]);

  if(timeUs<0 || timeUs>=getPlayTimeUs(streamId)) {
    int max = sInfo.at(streamId).maxSampleNo;
    int residue = max%gop;
    return residue==0?max:max-residue;
  } else if(timeUs==0) {
    return 0;
  }

  int sampleNo = getSampleNumber(streamId, timeUs);
  if(streamId==C::VIDEO_ID){
    int indexInGop = sampleNo%gop;
    if(indexInGop!=0){
      if(way==C::NEXT_KEY) sampleNo += gop-indexInGop;
      else if (way==C::PREVIOUS_KEY) sampleNo -= indexInGop;
      else {
        if(indexInGop>=gop/2) sampleNo += gop-indexInGop;
        else sampleNo -= indexInGop;
      }
    }
  }
  return sampleNo;
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
  FileReader& fileReader = contentsStorage.getCid(contentTitle);
  if (auto sessionPtr = parentSessionPtr.lock()) {
    return Util::findTimestampInVideoSample(
      fileReader.readRefVideoSampleWithLock(sampleNo, sessionPtr->getHybridMetaMap())[0]
    );
  }
  return C::INVALID_BYTE;
}
