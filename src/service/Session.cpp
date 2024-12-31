#include "../include/Session.h"
#include "../constants/C.h"

#include <iostream>

Session::Session(
  boost::asio::io_context & inputIoContext,
  boost::asio::ip::tcp::socket & inputSocket,
  std::string inputSessionId,
  Server & inputServer,
  ContentsStorage & inputContentsStorage,
  SntpRefTimeProvider & inputSntpRefTimeProvider
)
  : logger(Logger::getLogger(C::SESSION)),
    io_context(inputIoContext),
    socket(inputSocket),
    sessionId(inputSessionId),
    parentServer(inputServer),
    contentsStorage(inputContentsStorage),
    sntpRefTimeProvider(inputSntpRefTimeProvider) {
  const int64_t sessionInitTime = sntpRefTimeProvider.getRefTimeSecForCurrentTask();
  sessionInitTimeSecUtc = sessionInitTime;

  auto clientIpAddressEndpoint = socket.local_endpoint();
  clientRemoteAddress = clientIpAddressEndpoint.address().to_string();
}

Session::~Session() {}

void Session::start() {

}

void Session::shutdownSession() {

}
