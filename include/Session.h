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
#include "../include/BlockingQueue.h"
#include "../include/Buffer.h"
#include "../constants/C.h"
#include "../include/AcsHandler.h"
#include "../include/RtspHandler.h"
#include "../include/RtpHandler.h"

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
    boost::asio::ip::tcp::socket& inputSocket,
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

  void shutdown();

private:
  std::shared_ptr<Logger> logger;
  boost::asio::io_context& io_context;
  boost::asio::ip::tcp::socket& socket;
  std::string sessionId;
  Server& parentServer;
  ContentsStorage& contentsStorage;
  SntpRefTimeProvider& sntpRefTimeProvider;
  std::string cid = C::EMPTY_STRING;
  BlockingQueue<Buffer> txQ;

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
  BlockingQueue<Buffer> rxBitrateQueue;
  std::atomic<int> sentBitsSize = C::ZERO;

  std::vector<int> mbpsPossibleTypeList{};
  HybridMetaMapType hybridMeta;
};

#endif //SESSION_H
