#ifndef SESSION_H
#define SESSION_H
#include <boost/asio.hpp>

#include "../include/Server.h"

class Session {
public:
  explicit Session();

private:
  boost::asio::io_context& io_context;
  std::string sessionId;
  Server& parentServer;
};

#endif //SESSION_H
