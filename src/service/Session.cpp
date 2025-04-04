#include "../include/Session.h"

#include <boost/asio/bind_executor.hpp>
#include <iostream>
#include <thread>

#include "../../constants/Util.h"
#include "../../include/PeriodicTask.h"
#include "../constants/C.h"

Session::Session(
  boost::asio::io_context & inputIoContext,
  std::shared_ptr<boost::asio::io_context> inputWorkerIoContextPtr,
  std::shared_ptr<boost::asio::ip::tcp::socket> inputSocketPtr,
  std::string inputSessionId,
  Server & inputServer,
  ContentsStorage & inputContentsStorage,
  SntpRefTimeProvider & inputSntpRefTimeProvider,
  std::chrono::milliseconds inputZeroIntervalMs
)
  : logger(Logger::getLogger(C::SESSION)),
    io_context(inputIoContext),
    workerIoContextPtr(inputWorkerIoContextPtr),
    strand(boost::asio::make_strand(*inputWorkerIoContextPtr)),
    socketPtr(std::move(inputSocketPtr)),
    sessionId(inputSessionId),
    parentServer(inputServer),
    contentsStorage(inputContentsStorage),
    sntpRefTimeProvider(inputSntpRefTimeProvider),
    rtpTransportTask(*inputWorkerIoContextPtr, strand, inputZeroIntervalMs),
    bitrateRecodeTask(inputIoContext, strand, inputZeroIntervalMs),
    rtpQueuePtr(std::make_unique<boost::lockfree::queue<RtpPacketInfo*>>(C::RTP_TX_QUEUE_SIZE)){
  const int64_t sessionInitTime = sntpRefTimeProvider.getRefTimeSecForCurrentTask();
  sessionInitTimeSecUtc = sessionInitTime;

  auto clientIpAddressEndpoint = socketPtr->local_endpoint();
  clientRemoteAddress = clientIpAddressEndpoint.address().to_string();
}

Session::~Session(){
  // cleanup and release resources one more time before object destruction.
  try {
    rtpTransportTask.stop();
    bitrateRecodeTask.stop();
    for (const auto& taskPtr : videoReadingTaskVec) taskPtr->stop();
    for (const auto& taskPtr : audioReadingTaskVec) taskPtr->stop();
    if (socketPtr->is_open()){
      socketPtr->close();
    }
    if (rtspHandlerPtr != nullptr){
      rtspHandlerPtr.reset();
      rtspHandlerPtr = nullptr;
    }
    if (acsHandlerPtr != nullptr){
      acsHandlerPtr.reset();
      acsHandlerPtr = nullptr;
    }
    if (rtpHandlerPtr != nullptr){
      rtpHandlerPtr.reset();
      rtpHandlerPtr = nullptr;
    }

    // try one more time to save bitrate tx/rx recode
    recordBitrateTestResult();
  } catch (const std::exception & e) {
    std::cerr << "exception occurred during destruction! session id : " + sessionId
    << ". error msg " << e.what() << "\n";
  } catch (...) {
    std::cerr << "unknown exception was thrown during session destruction! session id : " + sessionId << "\n";
  }
}

void Session::start() {
  logger->info2("session id : " + sessionId + " starts.");
  sessionInitTimeSecUtc = sntpRefTimeProvider.getRefTimeSecForCurrentTask();

  asyncReceive();

  auto rtpTxTask = [&](){
    if (!isPaused){
      transmitRtp();
    }
  };
  rtpTransportTask.setTask(rtpTxTask);
  rtpTransportTask.start();

  auto txBitrateTask = [&]() {
    // shutdown session when client connection was lost
    if (
        readingEndSampleStatusVec[0] == true
        && readingEndSampleStatusVec[1] == true
    ) {
      logger->severe("Dongvin, sent last video and audio sample. session id : " + sessionId);
      return;
    }
    // do not close session while testing is in progress.
    /*if (
      Util::getCurrentTimeMillis() - latestOptionsReqTimeMillis
        > C::CLIENT_CONNECTION_LOSS_THRESHOLD_DURATION_MS
    ){
      logger->severe("Dongvin, client connection was lost. shutdown session. id : " + sessionId);
      onTeardown();
      return;
    }*/

    int32_t sentBit = sentBitsSize;
    sentBitsSize = 0;
    int64_t recordTimeUtcSec = sntpRefTimeProvider.getRefTimeSecForCurrentTask();
    if (utcTimeSecBitSizeMap.find(recordTimeUtcSec) != utcTimeSecBitSizeMap.end()) {
      int32_t prev = utcTimeSecBitSizeMap[recordTimeUtcSec];
      prev += sentBit;
      utcTimeSecBitSizeMap[recordTimeUtcSec] = prev;
    } else {
      utcTimeSecBitSizeMap.insert({recordTimeUtcSec, sentBit});
    }
  };
  bitrateRecodeTask.setTask(txBitrateTask);
  const std::chrono::milliseconds bitrateInterval(C::TX_BITRATE_SAMPLING_PERIOD_MS);
  bitrateRecodeTask.setInterval(bitrateInterval);
  bitrateRecodeTask.start();
}

boost::asio::io_context& Session::getIoContext(){
  return io_context;
}

void Session::setAcsHandlerPtr(std::shared_ptr<AcsHandler> inputAcsHandlerPtr){
  this->acsHandlerPtr = std::move(inputAcsHandlerPtr);
}

void Session::setRtspHandlerPtr(std::shared_ptr<RtspHandler> inputRtspHandlerPtr){
  this->rtspHandlerPtr = std::move(inputRtspHandlerPtr);
}

void Session::setRtpHandlerPtr(std::shared_ptr<RtpHandler> inputRtpHandlerPtr){
  this->rtpHandlerPtr = std::move(inputRtpHandlerPtr);
}

std::string Session::getSessionId() {
  return sessionId;
}

std::string Session::getClientRemoteAddress() {
  return clientRemoteAddress;
}

int64_t Session::getSessionInitTimeSecUtc() {
  return sessionInitTimeSecUtc;
}

int64_t Session::getSessionDestroyTimeSecUtc() {
  return sessionDestroyTimeSecUtc;
}

std::string Session::getDeviceModelNo() {
  return deviceModelNo;
}

void Session::updateDeviceModelNo(std::string name) {
  deviceModelNo = name;
}

std::string Session::getManufacturer() {
  return manufacturer;
}

void Session::updateManufacturer(std::string inputManufacturer) {
  manufacturer = inputManufacturer;
}

bool Session::getPauseStatus() {
  return isPaused;
}

void Session::updatePauseStatus(bool inputPausedStatus) {
  isPaused = inputPausedStatus;
}

std::string Session::getContentTitle() {
  return this->contentTitle;
}

void Session::updateContentTitleOfCurSession(std::string inputContentTitle) {
  contentTitle = inputContentTitle;
}

int64_t Session::getPlayTimeDurationMillis() {
  return playTimeMillis;
}

void Session::updatePlayTimeDurationMillis(int64_t inputPlayTimeDurationMillis) {
  playTimeMillis = inputPlayTimeDurationMillis;
}

void Session::stopCurrentMediaReadingTasks(bool needToStopAudioReadingTask) {
  for (const auto& videoReadingTaskPtr : videoReadingTaskVec) videoReadingTaskPtr->stop();
  if (needToStopAudioReadingTask) {
    for (const auto& audioReadingTaskPtr : audioReadingTaskVec) audioReadingTaskPtr->stop();
  }
}

float Session::get_mbpsCurBitrate() const {
  return static_cast<float>(kbpsCurrentBitrate)/1000.0f;
}

void Session::set_kbpsBitrate(int input_kbps) {
}

void Session::add_kbpsBitrateValue(int input_kbps) {
  kbpsCurrentBitrate += input_kbps;
}

int Session::get_kbpsCurBitrate() {
  return kbpsCurrentBitrate;
}

std::unordered_map<int64_t, int> & Session::getUtiTimeSecBitSizeMap() {
  return utcTimeSecBitSizeMap;
}

void Session::addRxBitrate(RxBitrate &record) {
  rxBitrateRecord.push_back(record);
}

std::vector<int> Session::get_mbpsTypeList() {
  return {};
}

void Session::set_mbpsTypeList(std::vector<int> input_mbpsTypeList) {
}

int Session::getNumberOfCamDirectories() {
  std::string contentTitle = getContentTitle();
  if(contentsStorage.getContentFileMetaMap().count(contentTitle)) {
    return contentsStorage.getContentFileMetaMap().at(contentTitle).getNumberOfCamDirectories();
  }
  logger->severe("Dongvin, failed to find content in ContentsStorage! :: getNumberOfCamDirectories()");
  return C::INVALID;
}

int Session::getRefVideoSampleCnt() {
  std::string contentTitle = getContentTitle();
  if (contentsStorage.getContentFileMetaMap().find(contentTitle) != contentsStorage.getContentFileMetaMap().end()) {
    return contentsStorage.getContentFileMetaMap().at(contentTitle).getRefVideoSampleCnt();
  } else {
    logger->severe("Dongvin, failed to find content in ContentsStorage! :: getRefVideoSampleCnt()");
    return C::INVALID;
  }
}

std::shared_ptr<RtpHandler> Session::getRtpHandlerPtr() {
  return rtpHandlerPtr;
}

std::string Session::getContentRootPath() const {
  return contentsStorage.getContentRootPath();
}

HybridMetaMapType & Session::getHybridMetaMap() {
  return hybridMeta;
}

const ContentsStorage& Session::getContentsStorage() const {
  return contentsStorage;
}

void Session::shutdownSession() {
  parentServer.afterTerminatingSession(sessionId);
}

void Session::handleRtspRequest(Buffer& buf) {
  if (buf.buf.empty()) {
    std::cerr << "Empty or invalid buffer received!\n";
    return;
  }

  if (!rtspHandlerPtr) {
    std::cerr << "rtspHandlerPtr is null!\n";
    return;
  }

  try {
    rtspHandlerPtr->run(buf);
  } catch (const std::exception& ex) {
    std::cerr << "Exception in handleRtspRequest: " << ex.what() << "\n";
  } catch (...) {
    std::cerr << "Unknown error in handleRtspRequest!" << "\n";
  }
}


bool Session::onCid(std::string inputCid) {
  logger->warning("Dongvin, requested content : " + inputCid + ", session id : " + sessionId);
  ContentFileMeta& fileReader = contentsStorage.getCid(inputCid);
  return acsHandlerPtr->setReaderAndContentTitle(fileReader, inputCid);
}


void Session::onChannel(int trackId, std::vector<int> channels) {
  if (acsHandlerPtr) {
    acsHandlerPtr->setChannel(trackId, channels);
  } else {
    logger->severe("Dongvin, No acs handler found!");
  }
}

void Session::onUserRequestingPlayTime(std::vector<float> playTimeSec) {
  logger->info(
    "Dongvin, cid: " + cid + ", play starting point : "
    + std::to_string(playTimeSec[0]) + "," + std::to_string(playTimeSec[1]) + " (sec)"
  );
  acsHandlerPtr->initUserRequestingPlaytime(playTimeSec);
}

void Session::onCameraChange(
  int nextCam,
  int nextId,
  std::vector<int64_t> switchingTimeInfo // first half :video, last half : audio
) {
  if (switchingTimeInfo.size() != 2){
    logger->severe("Dongvin, invalid switching time info!");
    return;
  }
  // stop sending rtp and restart with the new position of rtp
  acsHandlerPtr->setCamId(nextCam);
  int taIdx = static_cast<int>(switchingTimeInfo[1]);
  bool needToStopAudio{taIdx != C::INVALID};

  stopCurrentMediaReadingTasks(needToStopAudio);
  acsHandlerPtr->findNextSampleForSwitching(nextId, switchingTimeInfo);
  startPlayForCamSwitching();
}

void Session::onPlayStart(){
  // need to adjust sample reading and tx interval. if didn't, video stuttering occurs.
  int64_t videoInterval = acsHandlerPtr->getUnitFrameTimeUs(C::VIDEO_ID)/1000;
  int64_t audioInterval = acsHandlerPtr->getUnitFrameTimeUs(C::AUDIO_ID)/1000;

  if (videoInterval == C::INVALID || audioInterval == C::INVALID) {
    logger->severe(
      "Dongvin, failed to get unit frame time! video, audio >> " +
      std::to_string(videoInterval) + "," + std::to_string(audioInterval)
    );
    return;
  }

  std::chrono::milliseconds vInterval(videoInterval);
  auto videoSampleReadingTask = [&](){
    if (!isPaused){
      acsHandlerPtr->getNextVideoSample();
    }
  };
  auto videoTaskPtr = std::make_shared<PeriodicTask>(*workerIoContextPtr, strand, vInterval, videoSampleReadingTask);
  videoReadingTaskVec.emplace_back(std::move(videoTaskPtr));

  std::chrono::milliseconds aInterval(audioInterval);
  auto audioSampleReadingTask = [&](){
    if (!isPaused){
      acsHandlerPtr->getNextAudioSample();
    }
    deleteDanglingRtps();
  };
  auto audioTaskPtr = std::make_shared<PeriodicTask>(*workerIoContextPtr, strand, aInterval, audioSampleReadingTask);
  audioReadingTaskVec.emplace_back(std::move(audioTaskPtr));

  if (!videoReadingTaskVec.empty() && !audioReadingTaskVec.empty()){
    videoReadingTaskVec.back()->start();
    audioReadingTaskVec.back()->start();
  } else {
    throw std::runtime_error("Dongvin, failed to start media reading timer tasks : " + sessionId);
  }
}

void Session::startPlayForCamSwitching() {
  logger->info("Dongvin, start cam switching!");
  int64_t videoInterval = acsHandlerPtr->getUnitFrameTimeUs(C::VIDEO_ID)/1000;

  // fast transport video frames.
  for (int i = 0; i < C::FAST_TX_FACTOR_FOR_CAM_SWITCHING; ++i){
    if (!isPaused){
      acsHandlerPtr->getNextVideoSample();
    }
  }
  logger->info2("Dongvin, fast transported video samples. cnt : " + std::to_string(C::FAST_TX_FACTOR_FOR_CAM_SWITCHING));

  // start normal video tx task.
  std::chrono::milliseconds vInterval(videoInterval);
  auto videoSampleReadingTask = [&](){
    if (!isPaused){
      acsHandlerPtr->getNextVideoSample();
    }
  };
  auto videoTaskPtr = std::make_shared<PeriodicTask>(*workerIoContextPtr, strand, vInterval, videoSampleReadingTask);
  videoReadingTaskVec.emplace_back(std::move(videoTaskPtr));
  logger->info2("Dongvin, video reading task for cam switching started!");
  if (!videoReadingTaskVec.empty()){
    videoReadingTaskVec.back()->start();
  } else {
    throw std::runtime_error("Dongvin, failed to start video reading timer task for cam switching : " + sessionId);
  }
}

void Session::onTeardown() {
  logger->severe("Dongvin, teardown current session. session id : " + sessionId);
  sessionDestroyTimeSecUtc = sntpRefTimeProvider.getRefTimeSecForCurrentTask();
  stopAllPeriodicTasks();
  closeSocket();
  recordBitrateTestResult();
  shutdownSession();
}

void Session::recordBitrateTestResult() {
  if (isRecordSaved){
    logger->warning("Dongvin, bitrate test result already saved. session id : " + sessionId);
    return;
  }

  if (utcTimeSecBitSizeMap.size() < 1) {
    logger->severe("Dongvin, no data to save!");
    return;
  }

  std::string utcTime = Util::getCurrentUtcTimeString();

  std::string resultFileName = contentTitle + "_"
    + utcTime + "_"
    + deviceModelNo + "_"
    + manufacturer + "_"
    + sessionId + "_"
    + clientRemoteAddress + ".txt";

  std::ostringstream serverSideStream;
  serverSideStream << "Server_Tx_Bitrate_Record\n";
  std::vector<std::pair<int64_t, int32_t>> txRecord;
  for (auto& pair : utcTimeSecBitSizeMap) {
    txRecord.push_back(std::make_pair(pair.first, pair.second));
  }
  // sort in ascending order based on the first elem : the utc time sec.
  std::sort(txRecord.begin(), txRecord.end(), [](const auto& a, const auto& b) {
      return a.first < b.first;
  });
  int64_t bitSizeSum = 0;
  for (auto& pair : txRecord) {
    serverSideStream << pair.first << "," << pair.second << "\n";
    bitSizeSum += pair.second;
  }
  serverSideStream << "\n\n";

  int64_t playTimeDurationMillis =
    ( (txRecord.at(txRecord.size()-1)).first - (txRecord.at(0).first) )*1000;
  float avgTxMbps =
    (static_cast<float>(bitSizeSum)/static_cast<float>(1000) )/ static_cast<float>(playTimeDurationMillis);

  std::ostringstream clientSideStream;
  clientSideStream << "Client_Rx_Bitrate_Record\n";
  int64_t rxBitSizeSum = 0;
  for (auto& record : rxBitrateRecord) {
    clientSideStream << std::to_string(record.getUtcTimeMillis()/1000) << "," << record.getBitrate() << "\n";
    rxBitSizeSum += record.getBitrate();
  }

  int64_t clientPlayTimeDurationMillis = 1;
  if (rxBitrateRecord.size() > 2) {
    clientPlayTimeDurationMillis =
      rxBitrateRecord.at(rxBitrateRecord.size()-1).getUtcTimeMillis() - rxBitrateRecord.at(0).getUtcTimeMillis();
  }
  float avgRxMbps =
    (static_cast<float>(rxBitSizeSum)/static_cast<float>(1000)) / static_cast<float>(clientPlayTimeDurationMillis);

  std::ostringstream testInfos;
  testInfos << "TestInfo\n";
  testInfos << "Contents=" << contentTitle << "\n";
  testInfos << "ContentsPlayTimeDurationMillis=" << playTimeMillis << "\n";
  testInfos << "ContentsAvgFullStreamingBitrateMbps=" << get_mbpsCurBitrate() << "\n";
  testInfos << "ServerRealAvgTxBitrateMbps=" << avgTxMbps << "\n";
  testInfos << "ClientRealAvgRxBitrateMbps=" << avgRxMbps << "\n";
  testInfos << "DeviceModel=" << deviceModelNo << "\n";
  testInfos << "Manufacturer=" << manufacturer << "\n";
  testInfos << "SessionId=" << sessionId << "\n";
  testInfos << "ClientIPAddr=" << clientRemoteAddress << "\n\n";

  std::string testInfo = testInfos.str();
  std::string serverSideInfo = serverSideStream.str();
  std::string clientSideInfo = clientSideStream.str();
  std::string finalRecord;
  finalRecord += testInfo;
  finalRecord += serverSideInfo;
  finalRecord += clientSideInfo;

  std::string projectRootPath = parentServer.getProjectRootPath();
  std::filesystem::path finalPath(projectRootPath + DIR_SEPARATOR + resultFileName);

  std::ofstream outFile(finalPath);
  try {
    if (outFile.is_open()) {
      outFile << finalRecord;  // Write content to file
      outFile.close();      // Close the file
      isRecordSaved = true;
      logger->warning("Dongvin, bitrate record successfully saved. session id : " + sessionId);
    } else {
      logger->severe("Dongvin, bitrate record failed. session id : " + sessionId);
    }
  } catch (const std::exception& e){
    outFile.close();
    logger->severe("Dongvin, exception was thrown in saving bitrate record. session id : " + sessionId);
    std::cerr << e.what() << "\n";
  }
}

bool Session::getPFrameTxStatus() {
  return needSendPFrames;
}

void Session::updatePFrameTxStatus(bool newState) {
  needSendPFrames = newState;
}

bool Session::getIsInCamSwitching() {
  return isInCamSwitching;
}

void Session::updateIsInCamSwitching(bool newState) {
  isInCamSwitching = newState;
}

void Session::deleteDanglingRtps() {
  while (true) {
    if (rtpMemoryQueue.empty()) {
      break;
    }
    auto rtp = rtpMemoryQueue.front();
    if (rtp == nullptr || rtp->samplePtr == nullptr) {
      rtpMemoryQueue.pop();
    } else if (rtp->samplePtr->refCount == 0) {
      rtpMemoryQueue.pop();
    } else {
      break;
    }
  }
}

void Session::stopAllPeriodicTasks() {
  try {
    bitrateRecodeTask.stop();

    // stop the sample reading tasks first.
    for (const auto& taskPtr : videoReadingTaskVec) taskPtr->stop();
    for (const auto& taskPtr : audioReadingTaskVec) taskPtr->stop();

    // stop the rtp sending timer task.
    rtpTransportTask.stop();

    // discard all rtp packets in the queue.
    // this makes all std::shared_ptrs of Sample to be deleted from memory.
    clearRtpQueue();
    logger->warning("Dongvin, stopped all timers. session id : " + sessionId);
  } catch (const std::exception& e) {
    std::cerr << "exception was thrown on stopping PeriodicTasks! : " << e.what() << "\n";
  } catch (...) {
    std::cerr << "unknown exception was thrown on stopping PeriodicTasks! \n";
  }
}

void Session::closeSocket() {
  try {
    if (socketPtr->is_open()){
      socketPtr->close();
    }
    logger->warning("Dongvin, closed socket and all handlers. session id : " + sessionId);
  } catch (const std::exception& e) {
    std::cerr << "exception was thrown on closing socket and handlers! : " << e.what() << "\n";
  } catch (...) {
    std::cerr << "unknown exception was thrown on closing socket and handlers! \n";
  }
}

void Session::transmitRtspRes(std::unique_ptr<Buffer> bufPtr) {
  boost::system::error_code ignored_error;
  boost::asio::write(*socketPtr, boost::asio::buffer(bufPtr->buf), ignored_error);
  sentBitsSize += (bufPtr->len * 8);
}

void Session::enqueueRtpInfo(RtpPacketInfo* rtpPacketInfoPtr) {
  // repeat until success
  while (!rtpQueuePtr->push(rtpPacketInfoPtr)) {}
}

void Session::enqueueRtpForMemoryMgmt(std::shared_ptr<RtpPacketInfo> rtpPacketPtr) {
  rtpMemoryQueue.push(rtpPacketPtr);
}

void Session::updateReadLastVideoSample(){
  if(readingEndSampleStatusVec.size() == 2) readingEndSampleStatusVec[0] = true;
}

void Session::updateReadLastAudioSample(){
  if(readingEndSampleStatusVec.size() == 2) readingEndSampleStatusVec[1] = true;
}

void Session::clearRtpQueue() {
  // replace the rtp queue with a fresh instance since boost lock free queue does not support .clear() util.
  // it's because, clear() util needs a locking mechanism but boost lock free has no locking mechanism.
  auto new_queue = std::make_unique<boost::lockfree::queue<RtpPacketInfo*>>(C::RTP_TX_QUEUE_SIZE);
  rtpQueuePtr.swap(new_queue);  // old queue is discarded
}

void Session::updateOptionsReqTimeMillis(const int64_t inputOptionsReqTimeMillis){
  latestOptionsReqTimeMillis = inputOptionsReqTimeMillis;
}

void Session::transmitRtp() {
  if (rtpQueuePtr->empty()) {
    return;
  }
  RtpPacketInfo* rtpPacketInfoPtr = nullptr;
  if (rtpQueuePtr->pop(rtpPacketInfoPtr) && rtpPacketInfoPtr) {
    boost::system::error_code ignored_error;
    // send only valid rtps
    if (rtpPacketInfoPtr->length != C::INVALID){
      if (rtpPacketInfoPtr->flag == C::VIDEO_ID) {
        // tx video rtp
        boost::asio::write(
            *socketPtr,
            boost::asio::buffer(
                rtpPacketInfoPtr->samplePtr->buf.data() + rtpPacketInfoPtr->offset,
                rtpPacketInfoPtr->length
                ),
            ignored_error
        );
        rtpPacketInfoPtr->samplePtr->refCount -= 1;
      } else {
        // tx audio rtp
        boost::asio::write(
          *socketPtr,
          boost::asio::buffer(rtpPacketInfoPtr->samplePtr->buf.data(), rtpPacketInfoPtr->length),
          ignored_error
        );
        rtpPacketInfoPtr->samplePtr->refCount -= 1;
      }
      sentBitsSize += static_cast<int>(rtpPacketInfoPtr->length * 8);
    }// end of length check if
  }
}

void Session::asyncReceive() {
  if (isRecordSaved) {
    logger->severe("Dongvin, session already shutdown.");
    return;
  }
  if (!socketPtr->is_open()) {
    logger->severe("Socket is not open. session id : " + sessionId);
    return;
  }

  auto self = shared_from_this();
  auto buf = std::make_shared<std::vector<unsigned char>>(C::RTSP_MSG_BUFFER_SIZE);  // shared buffer
  socketPtr->async_read_some(
    boost::asio::buffer(*buf),
    boost::asio::bind_executor(
      strand,
      // this lambda will be passed to io_context.
      // to safely reference the Session object in lambda, self ptr is made and captured in this lambda.
    [this, self, buf](const boost::system::error_code& error, std::size_t bytesRead) {
      if (error) {
        if (error == boost::asio::error::eof) {
          logger->warning("Dongvin, connection closed by peer. session id : " + sessionId);
        } else {
          logger->severe("Dongvin, receiving request failed: " + error.message());
        }
        return;
      }
      // append new data to the RTSP buffer. rtspBuffer is initialized at Session.h as a member of Session class.
      rtspBuffer.append(reinterpret_cast<char*>(buf->data()), bytesRead);
      // parse and extract one requests and process it
      while (true) {
        size_t pos = rtspBuffer.find(C::CRLF2);  // look for full RTSP request
        if (pos == std::string::npos) break;  // no complete request yet
        std::string request = rtspBuffer.substr(0, pos + 4);
        rtspBuffer.erase(0, pos + 4);  // Remove processed request
        auto bufferPtr = std::make_unique<Buffer>(
          std::vector<unsigned char>(request.begin(), request.end()), 0, request.size()
        );
        handleRtspRequest(*bufferPtr);

        bool isTearRes = false;
        bool isErrorRes = false;
        std::string res = bufferPtr->getString();
        logger->warning("Dongvin, " + sessionId + ", rtsp response: ");
        for (auto& resLine : Util::splitToVecByStringForRtspMsg(res, C::CRLF)) {
          logger->info(resLine);
          if (resLine.find("Teardown:") != std::string::npos) isTearRes = true;
          if (resLine.find("Error:") != std::string::npos) isErrorRes = true;
        }
        std::cout << "\n";
        transmitRtspRes(std::move(bufferPtr));
        if (isTearRes || isErrorRes) {
          onTeardown();
          return; // stop receiving rtsp req after shutting down session
        }
      }
      // post next asyncReceive() on strand to avoid deep recursion
      boost::asio::post(strand, [self](){ self->asyncReceive(); });
    }// end of lambda which will be passed to io_context.

  ));// end of bind_executor() and async_read_some()
}