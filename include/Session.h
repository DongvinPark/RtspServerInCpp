#ifndef SESSION_H
#define SESSION_H
#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>
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
#include "../include/PeriodicTask.h"

// forward declaration of Server, ContentsStorage, and SntpRefTimeProvider
// to prevent circular referencing
class Server;
class ContentsStorage;
class SntpRefTimeProvider;

class AcsHandler;
class RtspHandler;
class RtpHandler;

struct VideoSampleRtps {
  unsigned char data[5 * 1024 * 1024]; // 5MB
  size_t size;
};

struct AudioSampleRtp {
  unsigned char data[1500]; // MTU of rtp packet is 1472 bytes
  size_t length;
};

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
    SntpRefTimeProvider& inputSntpRefTimeProvider,
    std::chrono::milliseconds inputIntervalMs
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
  int64_t getSessionInitTimeSecUtc();
  int64_t getSessionDestroyTimeSecUtc();

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

  void stopCurrentMediaReadingTasks();

  float get_mbpsCurBitrate() const;
  void set_kbpsBitrate(int input_kbps);
  void add_kbpsBitrateValue(int input_kbps);
  int get_kbpsCurBitrate();
  std::unordered_map<int64_t, int>& getUtiTimeSecBitSizeMap();
  void addRxBitrate(RxBitrate& record);
  std::vector<int> get_mbpsTypeList();
  void set_mbpsTypeList(std::vector<int> input_mbpsTypeList);

  int getNumberOfCamDirectories();
  int getRefVideoSampleCnt();
  std::shared_ptr<RtpHandler> getRtpHandlerPtr();

  std::string getContentRootPath() const;
  HybridMetaMapType& getHybridMetaMap();

  const ContentsStorage& getContentsStorage() const;

  void shutdownSession();

  // for rtsp messages
  void handleRtspRequest(Buffer& buf);
  bool onCid(std::string inputCid);
  void onChannel(int trackId, std::vector<int> channels);
  void onUserRequestingPlayTime(std::vector<float> playTimeSec);

  void onSwitching(int nextId, std::vector<int64_t> switchingTimeInfo, std::unique_ptr<Buffer> switchingRspPtr, bool neetToLimitSample);
  void onCameraChange(int nextCam, int nextId, std::vector<int64_t> switchingTimeInfo, Buffer& camChangeRspPtr);

  void onPlayStart();
  void onTeardown();

  void onPlayDone(int streamId);

  void recordBitrateTestResult();

private:
  void stopAllTimerTasks();
  void closeHandlersAndSocket();

  bool isPlayDone(int streamId);
  void transmit(std::unique_ptr<Buffer> bufPtr);

  std::unique_ptr<Buffer> receive(boost::asio::ip::tcp::socket& socket);

  std::shared_ptr<Logger> logger;
  boost::asio::io_context& io_context;
  std::shared_ptr<boost::asio::ip::tcp::socket> socketPtr;
  std::string sessionId;
  Server& parentServer;
  ContentsStorage& contentsStorage;
  SntpRefTimeProvider& sntpRefTimeProvider;
  std::string cid = C::EMPTY_STRING;

  std::shared_ptr<AcsHandler> acsHandlerPtr = nullptr;
  std::shared_ptr<RtspHandler> rtspHandlerPtr = nullptr;
  std::shared_ptr<RtpHandler> rtpHandlerPtr = nullptr;

  std::vector<bool> playDone = {false, false};
  std::atomic<bool> interruptSending = false;

  // memory pool for Video and Audio Sample to prevent head memory fragmentation.
  boost::object_pool<AudioSampleRtp> audioRtpPool;
  boost::object_pool<VideoSampleRtps> videoRtpPool;

  std::string clientRemoteAddress = C::EMPTY_STRING;
  int64_t sessionInitTimeSecUtc = C::INVALID_OFFSET;
  int64_t sessionDestroyTimeSecUtc = C::INVALID_OFFSET;
  std::string deviceModelNo = C::EMPTY_STRING;
  std::string manufacturer = C::EMPTY_STRING;

  std::atomic<bool> isPaused = false;
  std::string contentTitle = C::EMPTY_STRING;
  int64_t playTimeMillis = C::INVALID_OFFSET;

  int kbpsCurrentBitrate = C::ZERO;
  std::unordered_map<int64_t, int32_t> utcTimeSecBitSizeMap;
  std::atomic<int> sentBitsSize = C::ZERO;

  std::vector<RxBitrate> rxBitrateRecord{};
  std::vector<int> mbpsPossibleTypeList{};
  HybridMetaMapType hybridMeta;

  PeriodicTask rtspTask;
  PeriodicTask bitrateRecodeTask;
  std::vector<std::shared_ptr<PeriodicTask>> videoReadingTaskVec;
  std::vector<std::shared_ptr<PeriodicTask>> audioReadingTaskVec;

  bool isShutdown = false;
  bool isRecordSaved = false;
};

#endif //SESSION_H
