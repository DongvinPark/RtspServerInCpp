#include "../include/Session.h"

#include <iostream>

#include "../../constants/Util.h"
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
    socketPtr(std::move(inputSocketPtr)),
    sessionId(inputSessionId),
    parentServer(inputServer),
    contentsStorage(inputContentsStorage),
    sntpRefTimeProvider(inputSntpRefTimeProvider),
    strand(boost::asio::make_strand(io_context)){
  const int64_t sessionInitTime = sntpRefTimeProvider.getRefTimeSecForCurrentTask();
  sessionInitTimeSecUtc = sessionInitTime;

  auto clientIpAddressEndpoint = socketPtr->local_endpoint();
  clientRemoteAddress = clientIpAddressEndpoint.address().to_string();
}

Session::~Session() {
}

void Session::start() {
  logger->info2("session id : " + sessionId + " starts.");
  // Start asynchronous read operation
  //asyncRead();

  auto rxTask = [&](){
    try {
      while (true) {
        std::cout << "!!! rx while enter !!!\n";
        std::unique_ptr<Buffer> bufferPtr = receive(*socketPtr);
        if (bufferPtr != nullptr) {
          Buffer& buf = *bufferPtr;
          handleRtspRequest(buf);
          //queueTx(std::move(bufferPtr));
          transmit(std::move(bufferPtr));
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
        /*std::unique_ptr<Buffer> bufferPtrToTx = takeTxq();
        if (bufferPtrToTx == nullptr || bufferPtrToTx->len == C::INVALID) break;

        std::vector<std::string> resVec = Util::splitToVecByStringForRtspMsg(bufferPtrToTx->getString(), C::CRLF);
        std::cout << "!!! res check !!!" << std::endl;

        for (const auto & res : resVec)
        {
          std::cout << res << std::endl;
        }
        transmit(std::move(bufferPtrToTx));
        if (bufferPtrToTx->afterTx != nullptr){
          std::cout << "after tx not null!!!" << std::endl;
          bufferPtrToTx->afterTx();
        }*/
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
    std::cout << "Processing RTSP request..." << std::endl;

    // Process the RTSP request
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
  // TODO : update later!
  return true;
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
  Buffer& camChangeRspPtr
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
  std::cout << "!!! Tx completed !!!" << std::endl;
}

std::unique_ptr<Buffer> Session::receive(boost::asio::ip::tcp::socket &socket) {
  if (!socket.is_open()) {
    throw std::runtime_error("socket is not open");
  }
  std::vector<unsigned char> buf(2*1024);
  boost::system::error_code error;

  std::size_t bytesRead = socket.read_some(boost::asio::buffer(buf), error);
  std::cout << "!!! boost read completed!!!" << bytesRead << " bytes" << std::endl;
  if (error == boost::asio::error::eof) {
    // Connection closed cleanly by peer
    std::cerr << "Connection closed by peer." << std::endl;
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

std::unique_ptr<Buffer> Session::takeTxq() {
  try {
    return txQ.take();
  } catch (std::exception& ex) {
    std::cout << "Exception in takeTxq !!!" << std::endl;
    std::cerr << ex.what() << std::endl;
    return nullptr;
  }
}