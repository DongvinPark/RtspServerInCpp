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

  // TODO : implement BitrateRecorderTimer Task using PeriodicTask
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
}

std::string Session::getClientRemoteAddress() {
}

int64_t Session::getSessionInitTimeSecUtc() const {
}

int64_t Session::getSessionDestroyTimeSecUtc() const {
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
}

void Session::set_kbpsBitrate(int input_kbps) {
}

void Session::add_kbpsBitrateValue(int input_kbps) {
}

int Session::get_kbpsCurBitrate() {
}

std::unordered_map<int64_t, int> & Session::getUtiTimeSecBitSizeMap() {
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
}

void Session::shutdownSession() {
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
  for (auto& record : rxBitrateRecord) {
    // TODO : need to implement bitrate recording saving logic later.
    std::cout << record.getBitrate() << "," << record.getUtcTimeMillis() << std::endl;
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
  sentBitsSize += bufPtr->len;
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