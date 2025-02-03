#include "../include/Session.h"

#include <iostream>

#include "../../constants/Util.h"
#include "../../include/PeriodicTask.h"
#include "../constants/C.h"

#ifdef _WIN32
    const char DIR_SEPARATOR = '\\';
#else
const char DIR_SEPARATOR = '/';
#endif

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
    strand(boost::asio::make_strand(io_context)),
    rtspTask(inputIoContext, inputIntervalMs),
    videoSampleReadingTask(inputIoContext, inputIntervalMs),
    audioSampleReadingTask(inputIoContext, inputIntervalMs),
    bitrateRecodeTask(inputIoContext, inputIntervalMs){
  const int64_t sessionInitTime = sntpRefTimeProvider.getRefTimeSecForCurrentTask();
  sessionInitTimeSecUtc = sessionInitTime;

  auto clientIpAddressEndpoint = socketPtr->local_endpoint();
  clientRemoteAddress = clientIpAddressEndpoint.address().to_string();
}

Session::~Session() {
}

void Session::start() {
  logger->info2("session id : " + sessionId + " starts.");
  sessionInitTimeSecUtc = sntpRefTimeProvider.getRefTimeSecForCurrentTask();

  auto rxTask = [&](){
    try {
      std::unique_ptr<Buffer> bufferPtr = receive(*socketPtr);
      if (bufferPtr != nullptr) {
        Buffer& buf = *bufferPtr;
        handleRtspRequest(buf);

        std::string res = buf.getString();
        logger->warning("Dongvin, " + sessionId + ", rtsp response: ");
        for (auto& reqLine : Util::splitToVecByStringForRtspMsg(res, C::CRLF)) {
          logger->info(reqLine);
        }
        std::cout << "\n";
        transmit(std::move(bufferPtr));
      } else {
        shutdownSession();
      }
    } catch (const std::exception & e) {
      logger->severe("session " + sessionId + " failed. stop rx. exception : " + e.what());
      recordBitrateTestResult();
    }
  };

  rtspTask.setTask(rxTask);
  const std::chrono::milliseconds rtspInterval(C::ONE);
  rtspTask.setInterval(rtspInterval);
  rtspTask.start();

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

void Session::callStopLoaders() {
}

int Session::get_mbpsCurBitrate() const {
  return kbpsCurrentBitrate/1000;
}

void Session::set_kbpsBitrate(int input_kbps) {
}

void Session::add_kbpsBitrateValue(int input_kbps) {
}

int Session::get_kbpsCurBitrate() {
}

std::unordered_map<int64_t, int> & Session::getUtiTimeSecBitSizeMap() {
  return utcTimeSecBitSizeMap;
}

void Session::addRxBitrate(RxBitrate &record) {
  rxBitrateRecord.push_back(std::move(record));
}

std::vector<int> Session::get_mbpsTypeList() {
  return {};
}

void Session::set_mbpsTypeList(std::vector<int> input_mbpsTypeList) {
}

int Session::getNumberOfCamDirectories() {
  std::string contentTitle = getContentTitle();
  if(contentsStorage.getReaders().count(contentTitle)) {
    return contentsStorage.getReaders().at(contentTitle).getNumberOfCamDirectories();
  } else {
    logger->severe("Dongvin, faild to find content in ContentsStorage! :: getNumberOfCamDirectories()");
    return C::INVALID;
  }
}

int Session::getRefVideoSampleCnt() {
  std::string contentTitle = getContentTitle();
  if (contentsStorage.getReaders().find(contentTitle) != contentsStorage.getReaders().end()) {
    return contentsStorage.getReaders().at(contentTitle).getRefVideoSampleCnt();
  } else {
    logger->severe("Dongvin, faild to find content in ContentsStorage! :: getRefVideoSampleCnt()");
    return C::INVALID;
  }
}

std::shared_ptr<RtpHandler> Session::getRtpHandlerPtr() {
}

std::string Session::getContentRootPath() const {
}

int Session::getCamId() const {
}

HybridMetaMapType & Session::getHybridMetaMap() {
  return hybridMeta;
}

void Session::shutdownSession() {
  sessionDestroyTimeSecUtc = sntpRefTimeProvider.getRefTimeSecForCurrentTask();
  closeHandlersAndSocket();
  stopAllTimerTasks();
  recordBitrateTestResult();
}

void Session::handleRtspRequest(Buffer& buf) {
  if (buf.buf.empty()) {
    std::cerr << "Empty or invalid buffer received!" << std::endl;
    return;
  }

  if (!rtspHandlerPtr) {
    std::cerr << "rtspHandlerPtr is null!" << std::endl;
    return;
  }

  try {
    rtspHandlerPtr->run(buf);
  } catch (const std::exception& ex) {
    std::cerr << "Exception in handleRtspRequest: " << ex.what() << std::endl;
    shutdownSession();
  } catch (...) {
    std::cerr << "Unknown error in handleRtspRequest!" << std::endl;
    shutdownSession();
  }
}


bool Session::onCid(std::string inputCid) {
  logger->warning("Dongvin, requested content : " + inputCid + ", session id : " + sessionId);
  FileReader& fileReader = contentsStorage.getCid(inputCid);
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
}

void Session::onTeardown() {
  logger->severe("Dongvin, teardown current session. session id : " + sessionId);
  shutdownSession();
}

void Session::onTransmitVideoSample(std::vector<std::unique_ptr<Buffer>> rtpPtrs) {
}

void Session::onTransmitAudioSample(std::vector<std::unique_ptr<Buffer>> rtpPtrs) {
}

void Session::onPlayDone(int streamId) {
}

void Session::recordBitrateTestResult() {
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
  std::string finalPath = projectRootPath + DIR_SEPARATOR + resultFileName;

  std::ofstream outFile(finalPath);
  try {
    if (outFile.is_open()) {
      outFile << finalRecord;  // Write content to file
      outFile.close();      // Close the file
      logger->warning("Dongvin, bitrate record successfully saved. session id : " + sessionId);
    } else {
      logger->severe("Dongvin, bitrate record failed. session id : " + sessionId);
    }
  } catch (const std::exception& e){
    outFile.close();
    logger->severe("Dongvin, exception was thrown in saving bitrate record. session id : " + sessionId);
    std::cerr << e.what() << std::endl;
  }
}

void Session::stopAllTimerTasks() {
  rtspTask.stop();
  bitrateRecodeTask.stop();
  videoSampleReadingTask.stop();
  audioSampleReadingTask.stop();
  logger->info("Dongvin, stopped all timers. session id : " + sessionId);
}

void Session::closeHandlersAndSocket() {
  socketPtr->close();
  acsHandlerPtr->shutdown();
  logger->info("Dongvin, closed socket and acs handler. session id : " + sessionId);
}

bool Session::isPlayDone(int streamId) {
}

void Session::transmit(std::unique_ptr<Buffer> bufPtr) {
  std::lock_guard<std::mutex> guard(lock);
  sentBitsSize += (bufPtr->len * 8);
  boost::system::error_code ignored_error;
  boost::asio::write(*socketPtr, boost::asio::buffer(bufPtr->buf), ignored_error);
}

std::unique_ptr<Buffer> Session::receive(boost::asio::ip::tcp::socket &socket) {
  if (!socket.is_open()) {
    throw std::runtime_error("socket is not open. session id : " + sessionId);
  }
  std::vector<unsigned char> buf(2*1024);
  boost::system::error_code error;

  std::size_t bytesRead = socket.read_some(boost::asio::buffer(buf), error);
  if (error == boost::asio::error::eof) {
    // Connection closed cleanly by peer
    std::cerr << "Connection closed by peer. session id : " << sessionId << std::endl;
    return nullptr;
  }
  if (error) {
    std::cerr << error.message() << std::endl;
    throw boost::system::system_error(error); // Other errors
  }

  buf.resize(bytesRead);
  auto bufferPtr = std::make_unique<Buffer>(buf, 0, bytesRead);
  return bufferPtr;
}