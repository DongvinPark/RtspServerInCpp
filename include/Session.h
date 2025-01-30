#ifndef SESSION_H
#define SESSION_H
#include <boost/asio.hpp>
#include <atomic>
#include <cstdint> // For int64_t
#include <unordered_map>

#include "../include/Logger.h"
#include "../include/Server.h"
#include "../include/ContentsStorage.h"
#include "../include/SntpRefTimeProvider.h"
#include "../include/Buffer.h"
#include "../constants/C.h"
#include "../include/AcsHandler.h"
#include "../include/RtspHandler.h"
#include "../include/RtpHandler.h"
#include "../include/RxBitrate.h"

// forward declaration of Server, ContentsStorage, and SntpRefTimeProvider
// to prevent circular referencing
class Server;
class ContentsStorage;
class SntpRefTimeProvider;

class AcsHandler;
class RtspHandler;
class RtpHandler;

// dongvin : for hybrid streaming
/*
hybridMetaMap inside : camId >> view number & frame type >> sampleNo & sampleMetaData
Map : hybridMeta<camId, val>
                         |---> Map<view number + frame type string , val>
                                                                       |---> Map<sampleNo, val>
                                                                                            |---> sample meta data
                                                                                                  for avpt 6.1
 */
using HybridMetaMapType
    = std::unordered_map<int, std::unordered_map<std::string, std::unordered_map<int, HybridSampleMeta>>>;

class Session {
public:
  explicit Session(
    boost::asio::io_context& inputIoContext,
    std::shared_ptr<boost::asio::ip::tcp::socket> inputSocketPtr,
    std::string inputSessionId,
    Server& inputServer,
    ContentsStorage& inputContentsStorage,
    SntpRefTimeProvider& inputSntpRefTimeProvider
  );
  ~Session();

  // Rule of five. Session object is not allowed to copy and move.
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;
  Session& operator=(Session&&) noexcept = delete;
  Session(Session&&) noexcept = delete;

  void start();

  void setAcsHandlerPtr(std::shared_ptr<AcsHandler> inputAcsHandlerPtr);
  void setRtspHandlerPtr(std::shared_ptr<RtspHandler> inputRtspHandlerPtr);
  void setRtpHandlerPtr(std::shared_ptr<RtpHandler> inputRtpHandlerPtr);

  std::string getSessionId();
  std::string getClientRemoteAddress();
  int64_t getSessionInitTimeSecUtc() const;
  int64_t getSessionDestroyTimeSecUtc() const;

  std::string getDeviceModelNo();
  void updateDeviceModelNo(std::string name);
  std::string getManufacturer();
  void updateManufacturer(std::string inputManufacturer);

  bool getPauseStatus();
  void updatePauseStatus(bool inputPausedStatus);

  std::string getContentTitle();
  void updateContentTitleOfCurSession(std::string inputContentTitle);

  int64_t getPlayTimeDurationMillis();
  void updatePlayTimeDurationMillis(int64_t inputPlayTimeDurationMillis);

  void callStopLoaders();

  int get_mbpsCurBitrate() const;
  void set_kbpsBitrate(int input_kbps);
  void add_kbpsBitrateValue(int input_kbps);
  int get_kbpsCurBitrate();
  std::unordered_map<int64_t, int>& getUtiTimeSecBitSizeMap();
  void addRxBitrate(RxBitrate& record);
  std::vector<int> get_mbpsTypeList();
  void set_mbpsTypeList(std::vector<int> input_mbpsTypeList);

  int getNumberOfCamDirectories() const;
  int getRefVideoSampleCnt() const;
  std::shared_ptr<RtpHandler> getRtpHandlerPtr();

  std::string getContentRootPath() const;
  int getCamId() const;
  HybridMetaMapType& getHybridMetaMap();

  void shutdownSession();

  // for rtsp messages
  void handleRtspRequest(Buffer& buf);
  bool onCid(std::string inputCid);
  void onChannel(int trackId, std::vector<int> channels);
  void onUserRequestingPlayTime(std::vector<float> playTimeSec); // TODO: need to test at multi platform.

  void onSwitching(int nextId, std::vector<int64_t> switchingTimeInfo, std::unique_ptr<Buffer> switchingRspPtr, bool neetToLimitSample);
  void onCameraChange(int nextCam, int nextId, std::vector<int64_t> switchingTimeInfo, Buffer& camChangeRspPtr);

  void onPlayStart();
  void onTeardown();

  void onTransmitVideoSample(std::vector<std::unique_ptr<Buffer>> rtpPtrs);
  void onTransmitAudioSample(std::vector<std::unique_ptr<Buffer>> rtpPtrs);

  void onPlayDone(int streamId);

  void recordBitrateTestResult();

private:
  void closeHandlers();

  bool isPlayDone(int streamId);
  void transmit(std::unique_ptr<Buffer> bufPtr);

  void asyncRead();
  void asyncWrite();

  std::unique_ptr<Buffer> receive(boost::asio::ip::tcp::socket& socket);

  std::shared_ptr<Logger> logger;
  std::mutex lock;
  boost::asio::io_context& io_context;
  std::shared_ptr<boost::asio::ip::tcp::socket> socketPtr;
  std::string sessionId;
  Server& parentServer;
  ContentsStorage& contentsStorage;
  SntpRefTimeProvider& sntpRefTimeProvider;
  std::string cid = C::EMPTY_STRING;

  // members need to be iupdated after.
  std::shared_ptr<AcsHandler> acsHandlerPtr = nullptr;
  std::shared_ptr<RtspHandler> rtspHandlerPtr = nullptr;
  std::shared_ptr<RtpHandler> rtpHandlerPtr = nullptr;

  std::vector<bool> playDone = {false, false};
  std::atomic<bool> interruptSending = false;

  std::string clientRemoteAddress = C::EMPTY_STRING;
  int64_t sessionInitTimeSecUtc = C::INVALID_OFFSET;
  int64_t sessionDestroyTimeSecUtc = C::INVALID_OFFSET;
  std::string deviceModelNo = C::EMPTY_STRING;
  std::string manufacturer = C::EMPTY_STRING;

  std::atomic<bool> isPaused = false;
  std::string contentTitle = C::EMPTY_STRING;
  int64_t playTimeMillis = C::INVALID_OFFSET;

  int kbpsCurrentBitrate = C::ZERO;
  std::unordered_map<int64_t, int> utcTimeSecBitSizeMap;
  std::atomic<int> sentBitsSize = C::ZERO;

  std::vector<RxBitrate> rxBitrateRecord{};
  std::vector<int> mbpsPossibleTypeList{};
  HybridMetaMapType hybridMeta;

  boost::asio::strand<boost::asio::io_context::executor_type> strand;
};

#endif //SESSION_H
