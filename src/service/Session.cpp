#include "../include/Session.h"
#include "../constants/C.h"

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
    sntpRefTimeProvider(inputSntpRefTimeProvider){}

Session::~Session() {
}

void Session::start() {

}

void Session::shutdown() {

}
