#ifndef SERVER_H
#define SERVER_H
#include <memory>
#include <unordered_map>
#include <boost/asio.hpp>
#include <cstdint>
#include <thread>

#include "../include/PeriodicTask.h"
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
    std::vector<std::shared_ptr<boost::asio::io_context>>& inputIoContextPool,
    ContentsStorage& inputContentsStorage,
    const std::string &inputStorage,
    SntpRefTimeProvider& inputSntpRefTimeProvider,
    std::string projectRoot,
    std::chrono::milliseconds inputIntervalMs
  );
  ~Server();

  void start();

  std::unordered_map<std::string, std::shared_ptr<Session>>& getSessions();
  ContentsStorage& getContentsStorage();
  std::string getProjectRootPath();

  void shutdownServer();
  void afterTerminatingSession(const std::string& sessionId);

private:
  std::string getSessionId();
  std::shared_ptr<boost::asio::io_context> getNextWorkerIoContextPtr();
  long ioContextIdx{C::INVALID};

  std::shared_ptr<Logger> logger;
  boost::asio::io_context& io_context;
  std::vector<std::shared_ptr<boost::asio::io_context>>& ioContextPool;
  std::string projectRootPath;
  // used shared_ptr for better ownership management
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions;
  ContentsStorage& contentsStorage;
  std::string storage;
  SntpRefTimeProvider& sntpTimeProvider;
  int connectionCnt;

  std::unordered_map<std::string, std::shared_ptr<Session>> shutdownSessions;
  PeriodicTask removeClosedSessionTask;
};

#endif // SERVER_H