#ifndef SERVER_H
#define SERVER_H
#include <memory>
#include <unordered_map>
#include <boost/asio.hpp>

#include "../include/SntpRefTimeProvider.h"
#include "../include/ContentsStorage.h"
#include "../include/Session.h"
#include "../include/Logger.h"

// forward declaration of Session
class Session;
class ContentsStorage;
class SntpRefTimeProvider;

class Server {
public:
  explicit Server(
    boost::asio::io_context& inputIoContext,
    ContentsStorage& inputContentsStorage,
    const std::string &inputStorage,
    SntpRefTimeProvider& inputSntpRefTimeProvider
  );
  ~Server();

  void start();

  std::unordered_map<std::string, std::shared_ptr<Session>>& getSessions();
  ContentsStorage& getContentsStorage();

  void shutdownServer();
  void afterTerminatingSession(std::string sessionId);

private:
  std::string getSessionId();

  std::shared_ptr<Logger> logger;
  boost::asio::io_context& io_context;
  // used shared_ptr for better ownership management
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions;
  ContentsStorage& contentsStorage;
  std::string storage;
  SntpRefTimeProvider& sntpTimeProvider;
  int connectionCnt;
};

#endif // SERVER_H