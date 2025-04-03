#ifndef SESSION_H
#define SESSION_H
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <atomic>
#include <cstdint> // For int64_t
#include <unordered_map>
#include <queue>
#include <mutex>

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

struct Sample {
  std::vector<unsigned char> buf;
  std::atomic<int> refCount;

  explicit Sample (std::ifstream& fileAccess, const long sampleLen)
    : refCount(0) {
    // seek is done outside of this class
    buf.resize(sampleLen);
    if (fileAccess.read(reinterpret_cast<std::ifstream::char_type *>(buf.data()), sampleLen)) {
    } else {
      refCount = C::INVALID;
    }
  }

  explicit Sample ()
    : refCount(0) {
    std::cout << "!!! sample destructed !!!\n";
  }

  ~Sample() {}
};

struct RtpPacketInfo {
  int flag; // 0 for video, 1 for audio
  std::shared_ptr<Sample> samplePtr;
  size_t offset;
  size_t length;
  bool isHybridMeta;
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

class Session : public std::enable_shared_from_this<Session> {
  public:
  explicit Session(
    boost::asio::io_context& inputIoContext,
    std::shared_ptr<boost::asio::io_context> inputWorkerIoContextPtr,
    std::shared_ptr<boost::asio::ip::tcp::socket> inputSocketPtr,
    std::string inputSessionId,
    Server& inputServer,
    ContentsStorage& inputContentsStorage,
    SntpRefTimeProvider& inputSntpRefTimeProvider,
    std::chrono::milliseconds inputZeroIntervalMs
  );
  ~Session();

  // Rule of five. Session object is not allowed to copy and move.
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;
  Session& operator=(Session&&) noexcept = delete;
  Session(Session&&) noexcept = delete;

  void start();

  boost::asio::io_context& getIoContext();

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

  void stopCurrentMediaReadingTasks(bool needToStopAudioReadingTask);

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

  void onCameraChange(int nextCam, int nextId, std::vector<int64_t> switchingTimeInfo);

  // play and teardown
  void onPlayStart();
  void startPlayForCamSwitching();
  void onTeardown();
  void recordBitrateTestResult();

  // looking sample control
  bool getPFrameTxStatus();
  void updatePFrameTxStatus(bool newState);
  bool getIsInCamSwitching();
  void updateIsInCamSwitching(bool newState);

  // rtp queue control
  void enqueueRtpInfo(RtpPacketInfo* rtpPacketInfoPtr);
  void enqueueRtpForMemoryMgmt(std::shared_ptr<RtpPacketInfo> rtpPacketPtr);
  void clearRtpQueue();
  void updateReadLastVideoSample();
  void updateReadLastAudioSample();

  // client aliveness check
  void updateOptionsReqTimeMillis(int64_t inputOptionsReqTimeMillis);

  private:
  void deleteDanglingRtps();
  void stopAllPeriodicTasks();
  void closeSocket();

  void transmitRtspRes(std::unique_ptr<Buffer> bufPtr);
  void transmitRtp();

  void asyncReceive();

  std::shared_ptr<Logger> logger;
  boost::asio::io_context& io_context;
  std::shared_ptr<boost::asio::io_context> workerIoContextPtr;
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

  // looking sample control
  bool needSendPFrames = false;
  bool isInCamSwitching = false;

  // used strand to reduce cache miss
  boost::asio::strand<boost::asio::io_context::executor_type> strand;

  // for rtsp msg rx/tx
  std::string rtspBuffer;

  // tx queue for rtp.
  std::unique_ptr<boost::lockfree::queue<RtpPacketInfo*>> rtpQueuePtr;
  std::queue<std::shared_ptr<RtpPacketInfo>> rtpMemoryQueue;

  std::vector<bool> readingEndSampleStatusVec = {false, false};

  // client alive check
  int64_t latestOptionsReqTimeMillis = C::UNSET;

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

  PeriodicTask rtpTransportTask;
  PeriodicTask bitrateRecodeTask;
  std::vector<std::shared_ptr<PeriodicTask>> videoReadingTaskVec;
  std::vector<std::shared_ptr<PeriodicTask>> audioReadingTaskVec;

  bool isRecordSaved = false;
};

#endif //SESSION_H
