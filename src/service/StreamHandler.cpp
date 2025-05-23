#include <cstring>

#include "../include/StreamHandler.h"
#include "../../constants/Util.h"

StreamHandler::StreamHandler(
  std::string inputSessionId,
  std::weak_ptr<Session> inputParentSessionPtr,
  ContentsStorage& inputContentsStorage
)
: logger(Logger::getLogger(C::STREAM_HANDLER)),
  sessionId(inputSessionId),
  parentSessionPtr(inputParentSessionPtr),
  contentsStorage(inputContentsStorage){}

StreamHandler::~StreamHandler(){
  shutdown();
}

void StreamHandler::updateRtpRemoteCnt(int cnt) {
  videoRtpRemoveCnt = cnt;
}

void StreamHandler::updateCurSampleNo(int mediaType, int idx) {
  if (sInfo.find(mediaType) != sInfo.end()){
    sInfo.at(mediaType).setCurSampleNo(idx);
  } else {
    logger->severe("Dongvin, Invalid streamId on updateCurSampleNo!");
  }
}

int StreamHandler::getCamId() {
  return camId;
}

void StreamHandler::shutdown() {
  sInfo.clear();
}

void StreamHandler::setChannel(int streamId, std::vector<int> ch) {
  if (sInfo.find(streamId) != sInfo.end()){
    ReadInfo& readInfo = sInfo.at(streamId);
    readInfo.channel = std::move(ch);
  } else {
    logger->severe("Dongvin, Invalid streamId on setChannel!");
  }
}

void StreamHandler::initUserRequestingPlaytime(std::vector<float> timeS) {
  if (timeS.empty()) return;

  for (auto& pair : sInfo){
    int streamId = pair.first;
    ReadInfo& info = pair.second;

    info.startSampleNo = findKeySampleNumber(streamId, Util::secToUs(timeS[0]), C::NEXT_KEY);
    info.endSampleNo = findSampleNumber(streamId, Util::secToUs(timeS[1]));
    info.curSampleNo = info.startSampleNo;

    std::unique_ptr<Buffer> rtpPtr = get1stRtpOfRefSample(streamId, info.startSampleNo);
    if (rtpPtr != nullptr){
      info.timestamp = Util::findTimestamp(*rtpPtr);
      info.refSeq0 = Util::findSequenceNumber(*rtpPtr);

      // check --> removable.
      checkTimestamp(streamId, info);

      info.unitFrameCount = static_cast<int>(getUnitFrameCount(streamId));
      logger->info("Dongvin, init read info completed. streamId : " + std::to_string(streamId));
    }
  }
}

[[nodiscard]] bool StreamHandler::setRtpInfo(RtpInfo inputRtpInfo) {
  this->rtpInfo = inputRtpInfo;

  std::vector<int64_t> us = getUnitFrameTimeUs();
  if (sInfo.find(C::VIDEO_ID) != sInfo.end() && sInfo.find(C::AUDIO_ID) != sInfo.end()) {
    sInfo.at(C::VIDEO_ID).unitTimeUs = us[C::VIDEO_ID];
    sInfo.at(C::AUDIO_ID).unitTimeUs = us[C::AUDIO_ID];
    return true;
  }
  return false;
}

bool StreamHandler::setReaderAndContentTitle(ContentFileMeta& inputReader, std::string inputContentTitle) {
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

int StreamHandler::getLastVideoSampleNumber() {
  if (sInfo.find(C::VIDEO_ID) != sInfo.end()) {
    return sInfo.at(C::VIDEO_ID).maxSampleNo;
  }
  logger->severe("Dongvin, failed to get last video sample no!");
  return C::INVALID;
}

int StreamHandler::getLastAudioSampleNumber() {
  if (sInfo.find(C::AUDIO_ID) != sInfo.end()){
    return sInfo.at(C::AUDIO_ID).maxSampleNo;
  }
  logger->severe("Dongvin, failed to get last audio sample no!");
  return C::INVALID;
}

std::vector<unsigned char> StreamHandler::getAccData() {
  return contentsStorage.getCid(contentTitle).getAccDataCopy();
}

std::vector<std::vector<unsigned char>> StreamHandler::getAllV0Images() {
  return contentsStorage.getCid(contentTitle).getAllV0ImagesCopy();
}

bool StreamHandler::setVideoAudioSampleMetaDataCache(const std::string& contentTitle) {
  try {
    const auto& metaMap = contentsStorage.getContentFileMetaMap();
    const auto& contentMeta = metaMap.at(contentTitle).getConstVideoMeta();

    const auto cam0Iter = contentMeta.find(C::CAM_ID_LIST[0]);
    const auto cam1Iter = contentMeta.find(C::CAM_ID_LIST[1]);
    const auto cam2Inter = contentMeta.find(C::CAM_ID_LIST[2]);

    if (cam0Iter != contentMeta.end()) {
      const auto& frontVMeta
        = cam0Iter->second.getConstVideoSampleInfoList().at(C::FRONT_VIDEO_VID);
      const auto& rearVMeta
        = cam0Iter->second.getConstVideoSampleInfoList().at(C::REAR_VIDEO_VID);
      cachedCam0frontVSampleMetaListPtr = &frontVMeta;
      cachedCam0rearVSampleMetaListPtr = &rearVMeta;
    }
    if (cam1Iter != contentMeta.end()) {
      const auto& frontVMeta
        = cam1Iter->second.getConstVideoSampleInfoList().at(C::FRONT_VIDEO_VID);
      const auto& rearVMeta
        = cam1Iter->second.getConstVideoSampleInfoList().at(C::REAR_VIDEO_VID);
      cachedCam1frontVSampleMetaListPtr = &frontVMeta;
      cachedCam1rearVSampleMetaListPtr = &rearVMeta;
    }
    if (cam2Inter != contentMeta.end()) {
      const auto& frontVMeta
        = cam2Inter->second.getConstVideoSampleInfoList().at(C::FRONT_VIDEO_VID);
      const auto& rearVMeta
        = cam2Inter->second.getConstVideoSampleInfoList().at(C::REAR_VIDEO_VID);
      cachedCam2frontVSampleMetaListPtr = &frontVMeta;
      cachedCam2rearVSampleMetaListPtr = &rearVMeta;
    }

    const auto& audioMeta = contentsStorage.getContentFileMetaMap().at(contentTitle)
          .getConstAudioMeta().getConstMeta();
    cachedAudioSampleMetaListPtr = &audioMeta;

    return true;
  } catch (std::exception& e) {
    logger->severe("Dongvin, video and audio meta cache init failed!");
    std::cerr << e.what() << "\n";
    return false;
  } catch (...){
    logger->severe("Dongvin, unknown exception was thrown in setVideoAudioSampleMetaCache()!");
    return false;
  }
}

void StreamHandler::getNextVideoSample() {
  if (auto sessionPtr = parentSessionPtr.lock()) {
    std::weak_ptr<RtpHandler> weakPtr = sessionPtr->getRtpHandlerPtr();
    if (auto rtpHandlerPtr = weakPtr.lock()) {
      if (sInfo.find(C::VIDEO_ID) == sInfo.end()) {
        logger->severe("Dongvin, cannot find video RtpInfo! : getNextVideoSample.");
        return;
      }
      ReadInfo& info = sInfo.at(C::VIDEO_ID);
      if (info.isDone()) {
        sessionPtr->updateReadLastVideoSample();
        return;
      }

      const int sampleNo = info.curSampleNo;
      const VideoSampleInfo& curFrontVideoSampleInfo
        = camId == 0 ? cachedCam0frontVSampleMetaListPtr->at(sampleNo)
          : camId == 1 ? cachedCam1frontVSampleMetaListPtr->at(sampleNo)
            : camId == 2 ? cachedCam2frontVSampleMetaListPtr->at(sampleNo)
              : VideoSampleInfo();

      const VideoSampleInfo& curRearVideoSampleInfo
        = camId == 0 ? cachedCam0rearVSampleMetaListPtr->at(sampleNo)
          : camId == 1 ? cachedCam1rearVSampleMetaListPtr->at(sampleNo)
            : camId == 2 ? cachedCam2rearVSampleMetaListPtr->at(sampleNo)
              : VideoSampleInfo();

      rtpHandlerPtr->readVideoSample(
        curFrontVideoSampleInfo,
        curRearVideoSampleInfo,
        camId,
        C::INVALID,
        sampleNo,
        sessionPtr->getHybridMetaMap()
      );

      /* do not need on playing
       if (videoSampleRtpPtr->length != C::INVALID) {
        const int rtpLen = Util::getRtpPacketLength(videoSampleRtpPtr->data[2], videoSampleRtpPtr->data[3]);
        const int len = 4 + rtpLen;

        std::vector<unsigned char> buf;
        for (int i = 0; i < len; ++i) {
            buf.push_back(videoSampleRtpPtr->data[i]);
        }

        const Buffer firstRtp(buf, 0, len);

        info.timestamp = Util::findTimestamp(firstRtp);
        info.curPresentationTimeUs = getSamplePresentationTimeUs(C::VIDEO_ID, info.timestamp);
      }*/
      info.curSampleNo++;
    } else {
      logger->severe("Dongvin, failed to get rtpHandler ptr! : getNextVideoSample()");
    }
  } else {
    logger->severe("Dongvin, failed to get parent session ptr! : getNextVideoSample()");
  }
}

void StreamHandler::getNextAudioSample() {
  if (auto sessionPtr = parentSessionPtr.lock()) {
    std::weak_ptr<RtpHandler> weakPtr = sessionPtr->getRtpHandlerPtr();
    if (auto rtpHandlerPtr = weakPtr.lock()) {
      if (sInfo.find(C::AUDIO_ID) == sInfo.end()) {
        logger->severe("Dongvin, cannot find audio RtpInfo! : getNextAudioSample.");
        return;
      }
      ReadInfo& info = sInfo.at(C::AUDIO_ID);
      if (info.isDone()) {
        sessionPtr->updateReadLastAudioSample();
        return;
      }

      const int sampleNo = info.curSampleNo;
      const AudioSampleInfo& curAudioSampleInfo
        = cachedAudioSampleMetaListPtr->at(sampleNo);

      rtpHandlerPtr->readAudioSample(
        sampleNo,
        curAudioSampleInfo.offset,
        curAudioSampleInfo.len,
        sessionPtr->getHybridMetaMap()
      );

      /* do not need an playing
       if (audioSampleRtpPtr->length != C::INVALID) {
        const int rtpLen = Util::getRtpPacketLength(audioSampleRtpPtr->data[2], audioSampleRtpPtr->data[3]);
        const int len = 4 + rtpLen;

        std::vector<unsigned char> buf;
        for (int i = 0; i < len; ++i) {
            buf.push_back(audioSampleRtpPtr->data[i]);
        }

        const Buffer firstRtp(buf, 0, len);

        info.timestamp = Util::findTimestamp(firstRtp);
        info.curPresentationTimeUs = getSamplePresentationTimeUs(C::AUDIO_ID, info.timestamp);
      }*/
      info.curSampleNo++;
    } else {
      logger->severe("Dongvin, failed to get rtpHandler ptr! : getNextAudioSample()");
    }
  } else {
    logger->severe("Dongvin, failed to get parent session ptr! : getNextAudioSample()");
  }
}

bool StreamHandler::isDone(int streamId) {
  if (sInfo.empty()) return false;
  if (sInfo.find(streamId) == sInfo.end()) return false;
  return sInfo.at(streamId).isDone();
}

int64_t StreamHandler::getUnitFrameTimeUs(int streamId) {
  int targetStreamId = -1;
  if(streamId > 1) {
    targetStreamId = 0;
  } else {
    targetStreamId = streamId;
  }
  if (sInfo.find(targetStreamId) != sInfo.end()){
    return sInfo.at(targetStreamId).unitTimeUs;
  }
  logger->severe("Dongvin, failed to get unit frame time! streamId : " + std::to_string(targetStreamId));
  return C::INVALID;
}

std::string StreamHandler::getMediaInfo() {
  return contentsStorage.getCid(contentTitle).getMediaInfoCopy();
}

std::vector<int64_t> StreamHandler::getSsrc() {
  if (rtpInfo.kv.find(C::SSRC_KEY) == rtpInfo.kv.end()) {
    logger->severe("Dongvin, failed to get ssrc vector!");
    return {};
  }
  return rtpInfo.kv.at(C::SSRC_KEY);
}

int StreamHandler::getMainVideoNumber() {
  ContentFileMeta& fileReader = contentsStorage.getCid(contentTitle);
  const auto& videoMeta = fileReader.getConstVideoMeta();
  return videoMeta.at(C::REF_CAM).getFileNumber();
}

int StreamHandler::getMaxCamNumber() {
  return static_cast<int>(contentsStorage.getCid(contentTitle).getConstVideoMeta().size());
}

std::vector<int> StreamHandler::getInitialSeq() {
  std::vector<int> seqVec;
  for (auto streamId = 0; streamId < sInfo.size(); streamId++) {
    seqVec.push_back(sInfo[streamId].refSeq0);
  }
  return seqVec;
}

std::vector<int64_t> StreamHandler::getTimestamp() {
  std::vector<int64_t> timestampVec;
  for (auto streamId = 0; streamId < sInfo.size(); streamId++) {
    timestampVec.push_back(sInfo[streamId].timestamp);
  }
  return timestampVec;
}

int64_t StreamHandler::getTimestamp0(int streamId) {
  if (rtpInfo.kv.find(C::TIMESTAMP_KEY) == rtpInfo.kv.end()) {
    logger->severe("Dongvin, failed to get first timestamp! streamId : " + std::to_string(streamId));
    return C::INVALID;
  }
  return rtpInfo.kv.at(C::TIMESTAMP_KEY)[streamId];
}

int64_t StreamHandler::getUnitFrameCount(int streamId) {
  if (rtpInfo.kv.find(C::FRAME_COUNT_KEY) == rtpInfo.kv.end()) {
    logger->severe("Dongvin, failed to get unit frame count! streamId : " + std::to_string(streamId));
    return C::INVALID;
  }
  return rtpInfo.kv.at(C::FRAME_COUNT_KEY)[streamId];
}

std::vector<int64_t> StreamHandler::getUnitFrameCount() {
  if (rtpInfo.kv.find(C::FRAME_COUNT_KEY) == rtpInfo.kv.end()) {
    logger->severe("Dongvin, failed to get unit frame count vector!");
    return {};
  }
  return rtpInfo.kv.at(C::FRAME_COUNT_KEY);
}

std::vector<std::string> StreamHandler::getStreamUrls() {
  return rtpInfo.urls;
}

void StreamHandler::setStreamUrl(int streamId, std::string url) {
  if (rtpInfo.urls.empty()) {
    rtpInfo.urls.push_back(C::EMPTY_STRING);
    rtpInfo.urls.push_back(C::EMPTY_STRING);
  }
  rtpInfo.urls[streamId] = url;
}

std::vector<int64_t> StreamHandler::getPlayTimeUs() {
  if (rtpInfo.kv.find(C::PLAY_TIME_KEY) == rtpInfo.kv.end()) {
    logger->severe("Dongvin, failed to get play time duration vector!");
    return {};
  }
  return rtpInfo.kv.at(C::PLAY_TIME_KEY);
}

int64_t StreamHandler::getPlayTimeUs(int streamId) {
  if (rtpInfo.kv.find(C::PLAY_TIME_KEY) == rtpInfo.kv.end()) {
    logger->severe("Dongvin, failed to get play time duration! streamId : " + std::to_string(streamId));
    return C::INVALID;
  }
  return rtpInfo.kv.at(C::PLAY_TIME_KEY)[streamId];
}

std::vector<int64_t> StreamHandler::getGop() {
  if (rtpInfo.kv.find(C::GOP_KEY) == rtpInfo.kv.end()) {
    logger->severe("Dongvin, fail to get gop value vector!");
    return {};
  }
  return rtpInfo.kv.at(C::GOP_KEY);
}

void StreamHandler::setCamId(int inputCamId) {
  if (this->camId == inputCamId) return;
  logger->info3("Dongvin, sessionId : " + sessionId + ", camera is changed to " + std::to_string(inputCamId));
  this->camId = inputCamId;
}

void StreamHandler::findNextSampleForSwitching(int vid, std::vector<int64_t> timeInfo) {
  if (timeInfo.size() != 2) {
    logger->severe("Dongvin, invalid switching time info to find next samples!");
    return;
  }
  findNextSampleForSwitchingVideo(timeInfo[0]);
  findNextSampleForSwitchingAudio(timeInfo[1]);
}

std::unique_ptr<Buffer> StreamHandler::get1stRtpOfRefSample(int streamId, int sampleNo) {
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
      // read audio sample. one audio sample == one rtp packet.
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

void StreamHandler::checkTimestamp(int streamId, ReadInfo &readInfo) {
  int64_t t = getTimestamp0(streamId) + (readInfo.startSampleNo * getUnitFrameTimeUs(streamId));
  if (t != readInfo.timestamp) {
    logger->warning("Dongvin, timestamp calculation is wrong. stream id : " + std::to_string(streamId));
  }
}

int StreamHandler::findKeySampleNumber(int streamId, int64_t timeUs, int way) {
  int gop = static_cast<int>(getGop()[0]);

  if(timeUs<0 || timeUs>=getPlayTimeUs(streamId)) {
    if (sInfo.find(streamId) == sInfo.end()) {
      logger->severe("Dongvin, failed to get key sample no! : streamId : " + std::to_string(streamId));
      return C::INVALID_SAMPLE_NO;
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

int StreamHandler::findSampleNumber(int streamId, int64_t timeUs) {
  if (timeUs < 0 || timeUs >= getPlayTimeUs(streamId)){
    if (sInfo.find(streamId) == sInfo.end()){
      logger->severe("Dongvin, failed to get sample no! streamId : " + std::to_string(streamId));
      return C::INVALID_SAMPLE_NO;
    }
    return sInfo.at(streamId).maxSampleNo;
  }
  return getSampleNumber(streamId, timeUs);
}

int StreamHandler::getSampleNumber(int streamId, int64_t timeUs) {
  if (sInfo.find(streamId) == sInfo.end()){
    logger->severe("Dongvin, failed to get sample no! streamId : " + std::to_string(streamId));
    return C::INVALID;
  }
  int64_t t0 = sInfo.at(streamId).unitTimeUs;
  return static_cast<int>(std::round(static_cast<double>(timeUs)/static_cast<double>(t0)));
}

void StreamHandler::findNextSampleForSwitchingAudio(const int64_t targetSampleNo) {
  const int targetSampleIdx = static_cast<int>(targetSampleNo);
  if (targetSampleIdx == C::INVALID) {
    logger->info3("Dongvin, no switching time for audio was givven. id : " + sessionId);
    return;
  }
  const auto infoIt = sInfo.find(C::AUDIO_ID);
  if (infoIt == sInfo.end()){
    logger->severe("Dongvin, cannot find audio ReadInfo!");
    return;
  }

  ReadInfo& readInfo = infoIt->second;

  // All calculations to find the next sample number are done by the client.
  if (targetSampleIdx > readInfo.maxSampleNo) {
    logger->warning("Dongvin, don't switch on audio. sample number is over the end.");
    return;
  }

  // Actually, sampleNo is the number of sample in the client device's codec.
  // there are many samples on the network line and the client's sample buffer.
  const int sampleDiff = readInfo.curSampleNo - targetSampleIdx;
  readInfo.curSampleNo = targetSampleIdx;
  logger->info3(
  "Dongvin, id:"+ sessionId +"," +
              "\n<switching audio sample>"
              +"\ncurrent presentation time: "+std::to_string(readInfo.curPresentationTimeUs)+"(us)"
              +"\nsample diff: "+std::to_string(sampleDiff)
              +"\nswitching next sample no: "+std::to_string(targetSampleIdx)
  );
}

void StreamHandler::findNextSampleForSwitchingVideo(const int64_t targetSampleNo) {
  const auto infoIt = sInfo.find(C::VIDEO_ID);
  if (infoIt == sInfo.end()){
    logger->severe("Dongvin, cannot find video ReadInfo!");
    return;
  }

  // all calculation to find the next sample number is done in the client side.
  // Server just transmits the sample corresponding to the sample number.
  int targetSampleIdx = static_cast<int>(targetSampleNo);
  ReadInfo& readInfo = infoIt->second;

  if (targetSampleIdx > readInfo.maxSampleNo) {
    logger->warning("Dongvin, don't switch on video. sample no is over the end.");
    return;
  }

  int sampleDiff = readInfo.curSampleNo - targetSampleIdx;
  readInfo.curSampleNo = targetSampleIdx;
  readInfo.timestamp = getTimestamp(targetSampleIdx);
  logger->info3("Dongvin, id:"+sessionId
                +"\n <switching video sample>"
                +"\nnext member id: "+std::to_string(readInfo.curMemberVid)
                +"\ncurrent presentation time: "+std::to_string(readInfo.curPresentationTimeUs)+"(us)"
                +"\nsample diff: "+std::to_string(sampleDiff)
                +"\nswitching next sample no: "+std::to_string(targetSampleIdx));
}

std::vector<int64_t> StreamHandler::getUnitFrameTimeUs() {
  if (rtpInfo.kv.find(C::FRAME_COUNT_KEY) == rtpInfo.kv.end()) {
    logger->severe("Dongvin, failed to get unit frame time vector!");
    return {};
  }
  std::vector<int64_t> frameCount = rtpInfo.kv.at(C::FRAME_COUNT_KEY);
  return {
    (1000000*frameCount[0]/C::H265_CLOCK_RATE), // us
    (1000000*frameCount[1]/C::AAC_CLOCK_RATE)
  };
}

// this time is stream's presentation time.
// input: timestamp is the time that had generated at the stream creation.
// output: the presentation time = play time.
int64_t StreamHandler::getSamplePresentationTimeUs(int streamId, int64_t timestamp) {
  // we know the clock for video 90kHz (for h.265) and for audio 48kHz
  // actually this information is given by the message of ANNOUNCE.
  int clock = streamId == C::VIDEO_ID ? C::H265_CLOCK_RATE : C::AAC_CLOCK_RATE;
  int elapsedTimeCount = static_cast<int>(timestamp - getTimestamp0(streamId));
  return (1000000L*elapsedTimeCount/clock);
}

int64_t StreamHandler::getSamplePresentationTimeUs(int streamId, int sampleTimeIndex) {
  if (sInfo.find(streamId) != sInfo.end()){
    int clock = streamId == C::VIDEO_ID ? C::H265_CLOCK_RATE : C::AAC_CLOCK_RATE;
    int unitFrameCount = sInfo.at(streamId).unitFrameCount;
    return sampleTimeIndex*unitFrameCount*1000000L/clock;
  } else {
    logger->severe("Dongvin, failed to get unit frame count! stream id : " + std::to_string(streamId));
    return C::INVALID;
  }
}

int StreamHandler::getSampleTimeIndex(int streamId, int64_t timestamp) {
  if (sInfo.find(streamId) != sInfo.end()) {
    return static_cast<int>( (timestamp-getTimestamp0(streamId))/sInfo.at(streamId).unitFrameCount );
  }
  logger->severe("Dongvin, failed to get unit frame count! stream id : " + std::to_string(streamId));
  return C::INVALID;
}

int64_t StreamHandler::getTimestamp(const int sampleNo) {
  if (auto sessionPtr = parentSessionPtr.lock()) {
    std::weak_ptr<RtpHandler> weakPtr = sessionPtr->getRtpHandlerPtr();
    if (const auto rtpHandlerPtr = weakPtr.lock()) {

      const auto& curVideoSampleInfo = contentsStorage.getContentFileMetaMap().at(sessionPtr->getContentTitle())
          .getConstVideoMeta().at(C::CAM_ID_LIST[0]).getConstVideoSampleInfoList().at(0).at(sampleNo);

      const int64_t offset = curVideoSampleInfo.getOffset();
      const int64_t length = curVideoSampleInfo.getSize();

      const std::unique_ptr<Buffer> bufferPtr = rtpHandlerPtr->readFirstRtpOfCurVideoSample(sampleNo, offset, length);
      return Util::findTimestampInVideoSample(*bufferPtr);
    }
    logger->severe("Dongvin, failed to get rtp handler ptr!");
    return C::INVALID_BYTE;
  }
  logger->severe("Dongvin, failed to get session ptr!");
  return C::INVALID_BYTE;
}
