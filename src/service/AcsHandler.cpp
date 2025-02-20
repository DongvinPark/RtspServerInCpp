#include <cstring>

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
  if (sInfo.find(mediaType) != sInfo.end()){
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
  if (sInfo.find(streamId) != sInfo.end()){
    ReadInfo& readInfo = sInfo.at(streamId);
    readInfo.channel = std::move(ch);
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

[[nodiscard]] bool AcsHandler::setRtpInfo(RtpInfo inputRtpInfo) {
  this->rtpInfo = inputRtpInfo;

  std::vector<int64_t> us = getUnitFrameTimeUs();
  if (sInfo.find(C::VIDEO_ID) != sInfo.end() && sInfo.find(C::AUDIO_ID) != sInfo.end()) {
    sInfo.at(C::VIDEO_ID).unitTimeUs = us[C::VIDEO_ID];
    sInfo.at(C::AUDIO_ID).unitTimeUs = us[C::AUDIO_ID];
    return true;
  }
  return false;
}

bool AcsHandler::setReaderAndContentTitle(ContentFileMeta& inputReader, std::string inputContentTitle) {
  if (inputContentTitle != C::EMPTY_STRING) {
    contentTitle = inputContentTitle;
  }
  auto& videoMetaMap = inputReader.getConstVideoMeta();
  if (videoMetaMap.empty()){
    logger->severe("Dongvin, video meta init wrong!");
    return false;
  }

  int refVideoSampleSize = inputReader.getVideoSampleSize();
  int audioSampleSize = inputReader.getAudioSampleSize();

  if (refVideoSampleSize != C::INVALID && audioSampleSize != C::INVALID) {
    sInfo.emplace(C::VIDEO_ID, ReadInfo());
    sInfo.emplace(C::AUDIO_ID, ReadInfo());

    sInfo.at(C::VIDEO_ID).maxSampleNo = refVideoSampleSize-1;
    sInfo.at(C::AUDIO_ID).maxSampleNo = audioSampleSize-1;

    return setRtpInfo(inputReader.getRtpInfoCopy());
  }
  logger->severe(
    "Dongvin, video meta init wrong! refVideoSampleCnt : audioSampleCnt "
    + std::to_string(refVideoSampleSize) + "/" + std::to_string(audioSampleSize)
  );
  return false;
}

int AcsHandler::getLastVideoSampleNumber() {
  if (sInfo.find(C::VIDEO_ID) != sInfo.end()) {
    return sInfo.at(C::VIDEO_ID).maxSampleNo;
  }
  return C::INVALID;
}

int AcsHandler::getLastAudioSampleNumber() {
  if (sInfo.find(C::AUDIO_ID) != sInfo.end()){
    return sInfo.at(C::AUDIO_ID).maxSampleNo;
  }
  return C::INVALID;
}

std::vector<unsigned char> AcsHandler::getAccData() {
  return contentsStorage.getCid(contentTitle).getAccDataCopy();
}

std::vector<std::vector<unsigned char>> AcsHandler::getAllV0Images() {
  return contentsStorage.getCid(contentTitle).getAllV0ImagesCopy();
}

void AcsHandler::getNextVideoSample(VideoSampleRtp* videoSampleRtpPtr) {
  if (auto sessionPtr = parentSessionPtr.lock()) {
    std::weak_ptr<RtpHandler> weakPtr = sessionPtr->getRtpHandlerPtr();
    if (auto rtpHandlerPtr = weakPtr.lock()) {
      if (sInfo.find(C::VIDEO_ID) == sInfo.end()) {
        logger->severe("Dongvin, cannot find video RtpInfo! : getNextVideoSample.");
        return;
      }
      ReadInfo& info = sInfo.at(C::VIDEO_ID);
      if (info.isDone()) {
        videoSampleRtpPtr->length = C::INVALID;
        return;
      }

      int sampleNo = info.curSampleNo;
      const VideoSampleInfo& curFrontVideoSampleInfo = contentsStorage.getContentFileMetaMap().at(sessionPtr->getContentTitle())
          .getConstVideoMeta().at(C::CAM_ID_LIST[camId]).getConstVideoSampleInfoList().at(0).at(sampleNo);

      const VideoSampleInfo& curRearVideoSampleInfo = contentsStorage.getContentFileMetaMap().at(sessionPtr->getContentTitle())
          .getConstVideoMeta().at(C::CAM_ID_LIST[camId]).getConstVideoSampleInfoList().at(1).at(sampleNo);

      rtpHandlerPtr->readVideoSample(
        videoSampleRtpPtr,
        curFrontVideoSampleInfo,
        curRearVideoSampleInfo,
        camId,
        C::INVALID,
        sampleNo,
        sessionPtr->getHybridMetaMap()
      );

      if (videoSampleRtpPtr->length != C::INVALID) {
        // TODO : need to be tested with hybrid D & S
        const int rtpLen = Util::getRtpPacketLength(videoSampleRtpPtr->data[2], videoSampleRtpPtr->data[3]);
        const int len = 4 + rtpLen;
        unsigned char rtp[len];
        std::memcpy(rtp, videoSampleRtpPtr->data, len);

        std::vector<unsigned char> buf;
        for (unsigned char c : rtp) buf.push_back(c);

        const Buffer firstRtp(buf, 0, len);

        info.timestamp = Util::findTimestamp(firstRtp);
        info.curPresentationTimeUs = getSamplePresentationTimeUs(C::VIDEO_ID, info.timestamp);
        info.curSampleNo++;
      }
    } else {
      logger->severe("Dongvin, failed to get rtpHandler ptr! : getNextVideoSample()");
    }
  } else {
    logger->severe("Dongvin, failed to get parent session ptr! : getNextVideoSample()");
  }
}

void AcsHandler::getNextAudioSample(AudioSampleRtp* audioSampleRtpPtr) {
  if (auto sessionPtr = parentSessionPtr.lock()) {
    std::weak_ptr<RtpHandler> weakPtr = sessionPtr->getRtpHandlerPtr();
    if (auto rtpHandlerPtr = weakPtr.lock()) {
      if (sInfo.find(C::AUDIO_ID) == sInfo.end()) {
        logger->severe("Dongvin, cannot find audio RtpInfo! : getNextAudioSample.");
        return;
      }
      ReadInfo& info = sInfo.at(C::AUDIO_ID);
      if (info.isDone()) {
        audioSampleRtpPtr->length = C::INVALID;
        return;
      }

      const int sampleNo = info.curSampleNo;
      const AudioSampleInfo& curAudioSampleInfo = contentsStorage.getContentFileMetaMap().at(sessionPtr->getContentTitle())
        .getConstAudioMeta().getConstMeta().at(sampleNo);

      rtpHandlerPtr->readAudioSample(
        audioSampleRtpPtr,
        sampleNo,
        curAudioSampleInfo.offset,
        curAudioSampleInfo.len,
        sessionPtr->getHybridMetaMap()
      );

      if (audioSampleRtpPtr->length != C::INVALID) {
        // TODO : need to be tested with hybrid D & S
        const int rtpLen = Util::getRtpPacketLength(audioSampleRtpPtr->data[2], audioSampleRtpPtr->data[3]);
        const int len = 4 + rtpLen;
        unsigned char rtp[len];
        std::memcpy(rtp, audioSampleRtpPtr->data, len);

        std::vector<unsigned char> buf;
        for (unsigned char c : rtp) buf.push_back(c);

        const Buffer firstRtp(buf, 0, len);

        info.timestamp = Util::findTimestamp(firstRtp);
        info.curPresentationTimeUs = getSamplePresentationTimeUs(C::AUDIO_ID, info.timestamp);
        info.curSampleNo++;
      }
    } else {
      logger->severe("Dongvin, failed to get rtpHandler ptr! : getNextAudioSample()");
    }
  } else {
    logger->severe("Dongvin, failed to get parent session ptr! : getNextAudioSample()");
  }
}

bool AcsHandler::isDone(int streamId) {
  if (sInfo.empty()) return false;
  if (sInfo.find(streamId) == sInfo.end()) return false;
  return sInfo.at(streamId).isDone();
}

int64_t AcsHandler::getUnitFrameTimeUs(int streamId) {
  int targetStreamId = -1;
  if(streamId > 1) {
    targetStreamId = 0;
  } else {
    targetStreamId = streamId;
  }
  if (sInfo.find(targetStreamId) != sInfo.end()){
    return sInfo.at(targetStreamId).unitTimeUs;
  }
  return C::INVALID;
}

std::string AcsHandler::getMediaInfo() {
  return contentsStorage.getCid(contentTitle).getMediaInfoCopy();
}

std::vector<int64_t> AcsHandler::getSsrc() {
  if (rtpInfo.kv.find(C::SSRC_KEY) == rtpInfo.kv.end()) {
    throw std::runtime_error("Dongvin, failed to get ssrc vector!");
  }
  return rtpInfo.kv.at(C::SSRC_KEY);
}

int AcsHandler::getMainVideoNumber() {
  ContentFileMeta& fileReader = contentsStorage.getCid(contentTitle);
  const auto& videoMeta = fileReader.getConstVideoMeta();
  return videoMeta.at(C::REF_CAM).getFileNumber();
}

int AcsHandler::getMaxCamNumber() {
  return static_cast<int>(contentsStorage.getCid(contentTitle).getConstVideoMeta().size());
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
  if (rtpInfo.kv.find(C::TIMESTAMP_KEY) == rtpInfo.kv.end()) {
    throw std::runtime_error("Dongvin, failed to get first timestamp! streamId : " + std::to_string(streamId));
  }
  return rtpInfo.kv.at(C::TIMESTAMP_KEY)[streamId];
}

int64_t AcsHandler::getUnitFrameCount(int streamId) {
  if (rtpInfo.kv.find(C::FRAME_COUNT_KEY) == rtpInfo.kv.end()) {
    throw std::runtime_error("Dongvin, failed to get unit frame count! streamId : " + std::to_string(streamId));
  }
  return rtpInfo.kv.at(C::FRAME_COUNT_KEY)[streamId];
}

std::vector<int64_t> AcsHandler::getUnitFrameCount() {
  if (rtpInfo.kv.find(C::FRAME_COUNT_KEY) == rtpInfo.kv.end()) {
    throw std::runtime_error("Dongvin, failed to get unit frame count vector!");
  }
  return rtpInfo.kv.at(C::FRAME_COUNT_KEY);
}

std::vector<std::string> AcsHandler::getStreamUrls() {
  return rtpInfo.urls;
}

void AcsHandler::setStreamUrl(int streamId, std::string url) {
  if (rtpInfo.urls.empty()) {
    rtpInfo.urls.push_back(C::EMPTY_STRING);
    rtpInfo.urls.push_back(C::EMPTY_STRING);
  }
  rtpInfo.urls[streamId] = url;
}

std::vector<int64_t> AcsHandler::getPlayTimeUs() {
  if (rtpInfo.kv.find(C::PLAY_TIME_KEY) == rtpInfo.kv.end()) {
    throw std::runtime_error("Dongvin, failed to get play time duration vector!");
  }
  return rtpInfo.kv.at(C::PLAY_TIME_KEY);
}

int64_t AcsHandler::getPlayTimeUs(int streamId) {
  if (rtpInfo.kv.find(C::PLAY_TIME_KEY) == rtpInfo.kv.end()) {
    throw std::runtime_error("Dongvin, failed to get play time duration! streamId : " + std::to_string(streamId));
  }
  return rtpInfo.kv.at(C::PLAY_TIME_KEY)[streamId];
}

std::vector<int64_t> AcsHandler::getGop() {
  if (rtpInfo.kv.find(C::GOP_KEY) == rtpInfo.kv.end()) {
    throw std::runtime_error("Dongvin, failed to get gop vector!");
  }
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
  if (auto sessionPtr = parentSessionPtr.lock()) {
    std::weak_ptr<RtpHandler> weakPtr = sessionPtr->getRtpHandlerPtr();
    if (auto rtpHandlerPtr = weakPtr.lock()) {
      // read video sample.
      if (streamId == C::VIDEO_ID) {
        const VideoSampleInfo& curVideoSampleInfo = contentsStorage.getContentFileMetaMap().at(sessionPtr->getContentTitle())
          .getConstVideoMeta().at(C::CAM_ID_LIST[camId]).getConstVideoSampleInfoList().at(0).at(sampleNo);

        const int64_t offset = curVideoSampleInfo.getOffset();
        const int64_t len = curVideoSampleInfo.getSize();

        return rtpHandlerPtr->readFirstRtpOfCurVideoSample(sampleNo, offset, len);
      }
      // audio sample. one audio sample == one rtp packet.
      const AudioSampleInfo& curAudioSampleInfo = contentsStorage.getContentFileMetaMap().at(sessionPtr->getContentTitle())
        .getConstAudioMeta().getConstMeta().at(sampleNo);
      const int64_t offset = curAudioSampleInfo.offset;
      const int64_t len = curAudioSampleInfo.len;

      return rtpHandlerPtr->readFirstRtpOfCurAudioSample(sampleNo, offset, len);
    }
    logger->severe("Dongvin, failed to get RtpHandler Ptr!");
  }
  logger->severe("Dongvin, failed to get Session Ptr!");
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
    if (sInfo.find(streamId) == sInfo.end()) {
      throw std::runtime_error("Dongvin, failed to find key sample number!");
    }
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
    if (sInfo.find(streamId) == sInfo.end()){
      throw std::runtime_error("Dongvin, failed to find sample number!");
    }
    return sInfo.at(streamId).maxSampleNo;
  }
  return getSampleNumber(streamId, timeUs);
}

int AcsHandler::getSampleNumber(int streamId, int64_t timeUs) {
  if (sInfo.find(streamId) == sInfo.end()){
    throw std::runtime_error("Dongvin, failed to find sample number!");
  }
  int64_t t0 = sInfo.at(streamId).unitTimeUs;
  return static_cast<int>(std::round(static_cast<double>(timeUs)/static_cast<double>(t0)));
}

int AcsHandler::findNextSampleForSwitchingAudio(std::vector<int64_t> timeInfo) {
}

void AcsHandler::findNextSampleForSwitchingVideo(int nextVid, std::vector<int64_t> timeInfo) {
}

std::vector<int64_t> AcsHandler::getUnitFrameTimeUs() {
  if (rtpInfo.kv.find(C::FRAME_COUNT_KEY) == rtpInfo.kv.end()) {
    throw std::runtime_error("Dongvin, failed to get unit frame count! : getUnitFrameTimeUs()");
  }
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
  if (sInfo.find(streamId) != sInfo.end()){
    int clock = streamId == C::VIDEO_ID ? C::H265_CLOCK_RATE : C::AAC_CLOCK_RATE;
    int unitFrameCount = sInfo.at(streamId).unitFrameCount;
    return sampleTimeIndex*unitFrameCount*1000000L/clock;
  } else {
    logger->severe("Dongvin, failed to get unit frame count! stream id : " + std::to_string(streamId));
    return C::INVALID;
  }
}

int AcsHandler::getSampleTimeIndex(int streamId, int64_t timestamp) {
  if (sInfo.find(streamId) != sInfo.end()) {
    return static_cast<int>( (timestamp-getTimestamp0(streamId))/sInfo.at(streamId).unitFrameCount );
  }
  logger->severe("Dongvin, failed to get unit frame count! stream id : " + std::to_string(streamId));
  return C::INVALID;
}

int64_t AcsHandler::getTimestamp(int sampleNo) {
  if (auto sessionPtr = parentSessionPtr.lock()) {
    std::weak_ptr<RtpHandler> weakPtr = sessionPtr->getRtpHandlerPtr();
    if (auto rtpHandlerPtr = weakPtr.lock()) {

      const auto& curVideoSampleInfo = contentsStorage.getContentFileMetaMap().at(sessionPtr->getContentTitle())
          .getConstVideoMeta().at(C::CAM_ID_LIST[0]).getConstVideoSampleInfoList().at(0).at(sampleNo);

      const int64_t offset = curVideoSampleInfo.getOffset();
      const int64_t length = curVideoSampleInfo.getSize();

      const std::unique_ptr<Buffer> bufferPtr = rtpHandlerPtr->readFirstRtpOfCurVideoSample(sampleNo, offset, length);
      return Util::findTimestampInVideoSample(*bufferPtr);
    }
    return C::INVALID_BYTE;
  }
  return C::INVALID_BYTE;
}
