#include "../include/Session.h"

#include <iostream>

#include "../constants/C.h"


Session::Session(
  boost::asio::io_context & inputIoContext,
  std::shared_ptr<boost::asio::ip::tcp::socket> inputSocketPtr,
  std::string inputSessionId,
  Server & inputServer,
  ContentsStorage & inputContentsStorage,
  SntpRefTimeProvider & inputSntpRefTimeProvider
)
  : logger(Logger::getLogger(C::SESSION)),
    io_context(inputIoContext),
    socketPtr(inputSocketPtr),
    sessionId(inputSessionId),
    parentServer(inputServer),
    contentsStorage(inputContentsStorage),
    sntpRefTimeProvider(inputSntpRefTimeProvider) {
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
      while (true) {
        std::unique_ptr<Buffer> bufferPtr = receive(*socketPtr);
        if (bufferPtr != nullptr) {
          std::cout << "!!! get ptr from receive !!!\n";
          handleRtspRequest(std::move(bufferPtr));
        }
      }
    } catch (const std::exception & e) {
      logger->severe("session " + sessionId + " failed. stop rx. exception : " + e.what());
      recordBitrateTestResult();
    }
  };

  auto txTask = [&](){
    try {
      while (true) {
        std::unique_ptr<Buffer> bufferPtr = takeTxq();
        if (bufferPtr == nullptr || bufferPtr->len == C::INVALID) break;
        transmit(std::move(bufferPtr));
        if (bufferPtr->afterTx != nullptr) bufferPtr->afterTx();
      }
    } catch (const std::exception & e) {
      logger->severe("session " + sessionId + " failed. stop tx. exception : " + e.what());
      recordBitrateTestResult();
    }
  };

  // TODO : implement BitrateRecorderTimer Task using PeriodicTask

  std::thread(rxTask).detach();
  std::thread(txTask).detach();
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
}

void Session::updateDeviceModelNo(std::string name) {
}

std::string Session::getManufacturer() {
}

void Session::updateManufacturer(std::string inputManufacturer) {
}

bool Session::getPauseStatus() {
}

void Session::updatePauseStatus(bool inputPausedStatus) {
}

std::string Session::getContentTitle() {
}

void Session::updateContentTitleOfCurSession(std::string inputContentTitle) {
}

int64_t Session::getPlayTimeDurationMillis() {
}

void Session::updatePlayTimeDurationMillis(int64_t inputPlayTimeDurationMillis) {
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
}

BlockingQueue<Buffer> & Session::getRxBitrateQueue() {
}

std::vector<int> Session::get_mbpsTypeList() {
  return {};
}

void Session::set_mbpsTypeList(std::vector<int> input_mbpsTypeList) {
}

int Session::getNumberOfCamDirectories() const {
}

int Session::getRefVideoSampleCnt() const {
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
}

void Session::handleRtspRequest(std::unique_ptr<Buffer> bufPtr) {
  std::cout << "handle rtsp req enter !!!\n";
  queueTx(std::move(rtspHandlerPtr->run(std::move(bufPtr))));
}

bool Session::onCid(std::string inputCid) {
}

void Session::onChannel(int trackId, std::vector<int> channels) {
}

void Session::onUserRequestingPlayTime(std::vector<float> playTimeSec) {
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
  std::unique_ptr<Buffer> camChangeRspPtr
) {
}

void Session::onPlayStart() {
}

void Session::onTeardown() {
}

void Session::onTransmitVideoSample(std::vector<std::unique_ptr<Buffer>> rtpPtrs) {
}

void Session::onTransmitAudioSample(std::vector<std::unique_ptr<Buffer>> rtpPtrs) {
}

void Session::onPlayDone(int streamId) {
}

void Session::queueTx(std::unique_ptr<Buffer> bufPtr) {
  txQ.put(std::move(bufPtr));
}

void Session::recordBitrateTestResult() {
}

void Session::closeHandlers() {
}

bool Session::isPlayDone(int streamId) {
}

void Session::transmit(std::unique_ptr<Buffer> bufPtr) {
  sentBitsSize += bufPtr->len;
  boost::system::error_code ignored_error;
  boost::asio::write(*socketPtr, boost::asio::buffer(bufPtr->buf), ignored_error);
  std::cout << "!!! 전송 완료 !!!" << std::endl;
}

std::unique_ptr<Buffer> Session::receive(boost::asio::ip::tcp::socket &socket) {
  if (!socket.is_open()) {
    throw std::runtime_error("socket is not open");
  }
  std::vector<unsigned char> buf(2*1024);
  boost::system::error_code error;

  std::size_t bytesRead = socket.read_some(boost::asio::buffer(buf), error);
  if (error == boost::asio::error::eof) {
    // Connection closed cleanly by peer
    std::cerr << "Connection closed by peer." << std::endl;
    return nullptr;
  }
  if (error) {
    throw boost::system::system_error(error); // Other errors
  }
  std::cout << "!!! received !!! : " << std::endl;
  std::cout << "received : " << std::string(buf.begin(), buf.end()) << std::endl;

  auto bufferPtr = std::make_unique<Buffer>(buf, 0, bytesRead);
  return bufferPtr;
}

std::unique_ptr<Buffer> Session::takeTxq() {
  return txQ.take();
}