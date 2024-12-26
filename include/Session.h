#ifndef SESSION_H
#define SESSION_H
#include <boost/asio.hpp>

#include "../include/Logger.h"
#include "../include/Server.h"
#include "../include/ContentsStorage.h"
#include "../include/SntpRefTimeProvider.h"

// forward declaration of Server, ContentsStorage, and SntpRefTimeProvider
// to prevent circular referencing
class Server;
class ContentsStorage;
class SntpRefTimeProvider;

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
};

#endif //SESSION_H
