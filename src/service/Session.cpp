#include "../include/Session.h"

#include <iostream>

#include "../../constants/Util.h"
#include "../../include/PeriodicTask.h"
#include "../constants/C.h"

Session::Session(
  boost::asio::io_context & inputIoContext,
  std::shared_ptr<boost::asio::ip::tcp::socket> inputSocketPtr,
  std::string inputSessionId,
  Server & inputServer,
  ContentsStorage & inputContentsStorage,
  SntpRefTimeProvider & inputSntpRefTimeProvider,
  std::chrono::milliseconds inputIntervalMs
)
  : logger(Logger::getLogger(C::SESSION)),
    io_context(inputIoContext),
    socketPtr(std::move(inputSocketPtr)),
    sessionId(inputSessionId),
    parentServer(inputServer),
    contentsStorage(inputContentsStorage),
    sntpRefTimeProvider(inputSntpRefTimeProvider),
    bitrateRecodeTask(inputIoContext, inputIntervalMs){
  const int64_t sessionInitTime = sntpRefTimeProvider.getRefTimeSecForCurrentTask();
  sessionInitTimeSecUtc = sessionInitTime;

  auto clientIpAddressEndpoint = socketPtr->local_endpoint();
  clientRemoteAddress = clientIpAddressEndpoint.address().to_string();
}

Session::~Session() {}

void Session::start() {
  logger->info2("session id : " + sessionId + " starts.");
  sessionInitTimeSecUtc = sntpRefTimeProvider.getRefTimeSecForCurrentTask();

  socketPtr->set_option(boost::asio::socket_base::send_buffer_size(1 * 1024 * 1024)); // 1 MB
  socketPtr->set_option(boost::asio::ip::tcp::no_delay(true));

  asyncReceive();

  auto txBitrateTask = [&]() {
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
  const std::chrono::milliseconds bitrateInterval(C::TX_BITRATE_SAMPLING_PERIOD_MS); // 1 sec
  bitrateRecodeTask.setInterval(bitrateInterval);
  bitrateRecodeTask.start();
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

void Session::stopCurrentMediaReadingTasks() {
  for (const auto& videoReadingTaskPtr : videoReadingTaskVec) videoReadingTaskPtr->stop();
  for (const auto& audioReadingTaskPtr : audioReadingTaskVec) audioReadingTaskPtr->stop();
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
  } else {
    logger->severe("Dongvin, failed to find content in ContentsStorage! :: getNumberOfCamDirectories()");
    return C::INVALID;
  }
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
  acsHandlerPtr->setReaderAndContentTitle(fileReader, inputCid);
  return true;
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

void Session::onSwitching(
  int nextId,
  std::vector<int64_t> switchingTimeInfo,
  std::unique_ptr<Buffer> switchingRspPtr,
  bool neetToLimitSample
) {
}

void Session::onCameraChange(
  int nextCam,
  int nextId,
  std::vector<int64_t> switchingTimeInfo,
  Buffer& camChangeRspPtr
) {
}

void Session::onPlayStart() {
  int64_t videoInterval = acsHandlerPtr->getUnitFrameTimeUs(C::VIDEO_ID)/1000;
  int64_t audioInterval = acsHandlerPtr->getUnitFrameTimeUs(C::AUDIO_ID)/1000;

  std::chrono::milliseconds vInterval(videoInterval);
  auto videoSampleReadingTask = [&](){
    // pass object pool's memory to read video sample
    FrontVideoSampleRtps* frontVSamplePtr = frontVideoRtpPool.construct();
    RearVideoSampleRtps* rearVSamplePtr = rearVideoRtpPool.construct();
    acsHandlerPtr->getNextVideoSample(frontVSamplePtr, rearVSamplePtr);

    // send to client
    if (frontVSamplePtr->length != C::INVALID && rearVSamplePtr->length != C::INVALID) {
      transmitVideoRtp(frontVSamplePtr, rearVSamplePtr);
    }
  };
  auto videoTaskPtr = std::make_shared<PeriodicTask>(io_context, vInterval, videoSampleReadingTask);
  videoReadingTaskVec.emplace_back(std::move(videoTaskPtr));

  std::chrono::milliseconds aInterval(audioInterval);
  auto audioSampleReadingTask = [&](){
    // pass object pool's memory to read audio sample
    AudioSampleRtp* aSamplePtr = audioRtpPool.construct();
    acsHandlerPtr->getNextAudioSample(aSamplePtr);

    // send to clint
    if (aSamplePtr->length != C::INVALID) {
      transmitAudioRtp(aSamplePtr);
    }
  };
  auto audioTaskPtr = std::make_shared<PeriodicTask>(io_context, aInterval, audioSampleReadingTask);
  audioReadingTaskVec.emplace_back(std::move(audioTaskPtr));

  if (!videoReadingTaskVec.empty() && !audioReadingTaskVec.empty()){
    videoReadingTaskVec.back()->start();
    audioReadingTaskVec.back()->start();
  } else {
    throw std::runtime_error("Dongvin, failed to start media reading timer tasks : " + sessionId);
  }
}

void Session::onTeardown() {
  logger->severe("Dongvin, teardown current session. session id : " + sessionId);
  sessionDestroyTimeSecUtc = sntpRefTimeProvider.getRefTimeSecForCurrentTask();
  stopAllTimerTasks();
  closeHandlersAndSocket();
  recordBitrateTestResult();
  shutdownSession();
}

void Session::onPlayDone(const int streamId) {
  if (streamId == C::VIDEO_ID) for (const auto& taskPtr : videoReadingTaskVec) taskPtr->stop();
  if (streamId == C::AUDIO_ID) for (const auto& taskPtr : audioReadingTaskVec) taskPtr->stop();
}

void Session::recordBitrateTestResult() {
  if (isRecordSaved){
    logger->warning("Dongvin, bitrate test result already saved. session id : " + sessionId);
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

void Session::stopAllTimerTasks() {
  bitrateRecodeTask.stop();
  for (const auto& taskPtr : videoReadingTaskVec) taskPtr->stop();
  for (const auto& taskPtr : audioReadingTaskVec) taskPtr->stop();
  logger->info("Dongvin, stopped all timers. session id : " + sessionId);
}

void Session::closeHandlersAndSocket() {
  socketPtr->close();
  socketPtr.reset();
  rtspHandlerPtr.reset();
  acsHandlerPtr.reset();
  rtpHandlerPtr.reset();
  logger->info("Dongvin, closed socket and all handlers. session id : " + sessionId);
}

bool Session::isPlayDone(int streamId) {
}

void Session::transmitRtspRes(std::unique_ptr<Buffer> bufPtr) {
  boost::system::error_code ignored_error;
  boost::asio::write(*socketPtr, boost::asio::buffer(bufPtr->buf), ignored_error);
  sentBitsSize += (bufPtr->len * 8);
}

void Session::transmitVideoRtp(
  FrontVideoSampleRtps* videoSampleRtpsPtr, RearVideoSampleRtps* rearVSampleRtpsPtr
) {
  boost::system::error_code ignored_error;
  boost::asio::write(
    *socketPtr,
    // target index range to send : [ 0 : videoSampleRtpsPtr->length )
    boost::asio::buffer(videoSampleRtpsPtr->data, videoSampleRtpsPtr->length),
    ignored_error
  );
  boost::asio::write(
    *socketPtr,
    boost::asio::buffer(rearVSampleRtpsPtr->data, rearVSampleRtpsPtr->length),
    ignored_error
  );
  sentBitsSize += static_cast<int>(videoSampleRtpsPtr->length * 8);
  sentBitsSize += static_cast<int>(rearVSampleRtpsPtr->length * 8);
}

void Session::transmitAudioRtp(AudioSampleRtp* audioSampleRtpPtr) {
  boost::system::error_code ignored_error;
  boost::asio::write(
    *socketPtr,
    boost::asio::buffer(audioSampleRtpPtr->data, audioSampleRtpPtr->length),
    ignored_error
  );
  sentBitsSize += static_cast<int>(audioSampleRtpPtr->length * 8);
}

void Session::asyncReceive() {
  if (isRecordSaved) {
    logger->severe("Dongvin, receive:: session already shutdown.");
    return;
  }
  if (!socketPtr->is_open()) {
    logger->severe("Socket is not open. session id : " + sessionId);
    return;
  }

  auto buf = std::make_shared<std::vector<unsigned char>>(10 * 1024); // Shared buffer
  socketPtr->async_read_some(boost::asio::buffer(*buf),
    [this, buf](const boost::system::error_code& error, std::size_t bytesRead) {
      if (error) {
        if (error == boost::asio::error::eof) {
          logger->warning("Connection closed by peer. session id : " + sessionId);
        } else {
          logger->severe("Receive failed: " + error.message());
        }
        return;
      }

      buf->resize(bytesRead);
      auto bufferPtr = std::make_unique<Buffer>(*buf, 0, bytesRead);
      handleRtspRequest(*bufferPtr);

      bool isTearRes = false;
        bool isErrorRes = false;
        std::string res = bufferPtr->getString();
        logger->warning("Dongvin, " + sessionId + ", rtsp response: ");
        for (auto& resLine : Util::splitToVecByStringForRtspMsg(res, C::CRLF)) {
          logger->info(resLine);
          if (resLine.find("Teardown:") != std::string::npos) isTearRes = true;
          if (resLine.find("Error:") != std::string::npos) isTearRes = true;
        }
        std::cout << "\n";
        transmitRtspRes(std::move(bufferPtr));
        if (isTearRes || isErrorRes) {
          onTeardown();
        }

      // continue receiving
      asyncReceive();
    }
  );
}