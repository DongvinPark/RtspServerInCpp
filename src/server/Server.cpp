#include "../include/Server.h"

#include <sstream>

#include "../constants/Util.h"
#include "../constants/C.h"
#include "../include/AcsHandler.h"
#include "../include/RtspHandler.h"
#include "../include/RtpHandler.h"

using boost::asio::ip::tcp;

Server::Server(
  boost::asio::io_context& inputIoContext,
  ContentsStorage& inputContentsStorage,
  const std::string &inputStorage,
  SntpRefTimeProvider& inputSntpRefTimeProvider,
  std::string inputProjectRoot
) : logger(Logger::getLogger(C::SERVER)),
    io_context(inputIoContext),
    contentsStorage(inputContentsStorage),
    storage(inputStorage),
    sntpTimeProvider(inputSntpRefTimeProvider),
    connectionCnt(0),
    projectRootPath(inputProjectRoot){}

Server::~Server() {
  shutdownServer();
}

void Server::start() {
  logger->info3("Dongvin C++ AlphaStreamer3.1 starts!!");
  sntpTimeProvider.start();

  tcp::acceptor acceptor(
    io_context, tcp::endpoint(tcp::v4(), C::RTSP_RTP_TCP_PORT)
  );

  try {
    while (true) {
      auto socketPtr = std::make_shared<tcp::socket>(io_context);
      acceptor.accept(*socketPtr);
      std::string sessionId = getSessionId();

      // makes session and starts it.
      std::chrono::milliseconds zeroInterval(C::ZERO);
      std::shared_ptr<Session> sessionPtr = std::make_shared<Session>(
        io_context, socketPtr, sessionId,
        *this, contentsStorage, sntpTimeProvider, zeroInterval
      );

      // used weak pointer to break the circular dependencies
      auto inputAcsHandlerPtr = std::make_shared<AcsHandler>(sessionId, sessionPtr, contentsStorage);
      auto rtspHandlerPtr = std::make_shared<RtspHandler>(sessionId, sessionPtr, inputAcsHandlerPtr);
      auto rtpHandlerPtr = std::make_shared<RtpHandler>(sessionId, sessionPtr, inputAcsHandlerPtr);

      sessionPtr->setAcsHandlerPtr(inputAcsHandlerPtr);
      sessionPtr->setRtspHandlerPtr(rtspHandlerPtr);
      sessionPtr->setRtpHandlerPtr(rtpHandlerPtr);
      sessionPtr->start();

      // register session pointer at session map
      sessions.insert({sessionId, sessionPtr});
      logger->warning(
        "Dongvin, new client arrives, id: " + sessionId
        + ", total number of clients: " + std::to_string(sessions.size())
      );
    }
  } catch (const std::exception& e) {
    std::ostringstream oss;
    oss << "Server stops with exception : " << e.what();
    logger->severe(oss.str());
  }
}

std::unordered_map<std::string, std::shared_ptr<Session>> & Server::getSessions() {
  return sessions;
}

ContentsStorage & Server::getContentsStorage() {
  return contentsStorage;
}

std::string Server::getProjectRootPath(){
  return projectRootPath;
}

void Server::shutdownServer() {
  // shutdown all sessions.
  for (auto& kvPair : sessions) {
    kvPair.second->shutdownSession();
  }
  sessions.clear();
  contentsStorage.shutdown();
}

void Server::afterTerminatingSession(std::string sessionId) {
  std::shared_ptr<Session> sessionPtr = sessions.at(sessionId);

  // TODO : implement test record saving logic

  logger->info("test result successfully saved. session : " + sessionId);
  sessions.erase(sessionId);
  logger->info(
      "Dongvin, " + sessionId + " shuts down. Remaining session cnt : "
          + std::to_string(sessions.size())
    );
}

std::string Server::getSessionId() {
  connectionCnt++;
  return std::to_string(connectionCnt) + "_"
  + Util::getRandomKey(C::SESSION_KEY_BIT_SIZE);
}
