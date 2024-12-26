#include "../include/Server.h"

#include "../constants/Util.h"
#include "../constants/C.h"

using boost::asio::ip::tcp;

Server::Server(
  boost::asio::io_context& inputIoContext,
  ContentsStorage& inputContentsStorage,
  const std::string &inputStorage,
  SntpRefTimeProvider& inputSntpRefTimeProvider
) : logger(Logger::getLogger(C::SERVER)),
    io_context(inputIoContext),
    contentsStorage(inputContentsStorage),
    storage(inputStorage),
    sntpTimeProvider(inputSntpRefTimeProvider),
    connectionCnt(0) {}

Server::~Server() {
  shutdownServer();
}

void Server::start() {
  logger->info3("Dongvin C++ AlphaStreamer3.1 starts!!");
  sntpTimeProvider.start();

  tcp::acceptor acceptor(
    io_context, tcp::endpoint(tcp::v4(), C::RTSP_RTP_TCP_PORT)
  );

  while (true) {
    tcp::socket socket(io_context);
    acceptor.accept(socket);
    std::string sessionId = getSessionId();

    logger->warning(
      "Dongvin, new client arrives, id: " + sessionId
      + ", total number of clients: " + std::to_string(sessions.size())
    );

    // makes session and starts it.
    std::shared_ptr<Session> sessionPtr = std::make_shared<Session>(
      io_context, socket, sessionId,
      *this, contentsStorage, sntpTimeProvider
    );
    sessionPtr->start();

    // register session pointer at session map
    sessions.insert_or_assign(sessionId, sessionPtr);
  }
}

std::unordered_map<std::string, std::shared_ptr<Session>> & Server::getSessions() {
  return sessions;
}

ContentsStorage & Server::getContentsStorage() {
  return contentsStorage;
}

void Server::shutdownServer() {
  // shutdown all sessions.
  for (auto& kvPair : sessions) {
    kvPair.second->shutdown();
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
  return "client:" + std::to_string(connectionCnt) + ":"
  + Util::getRandomKey(C::SESSION_KEY_BIT_SIZE);
}
