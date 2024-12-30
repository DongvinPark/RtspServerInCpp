#ifndef RTPHANDLER_H
#define RTPHANDLER_H

#include "../include/Session.h"

class Session;
class AcsHandler;

class RtpHandler {
public:
  explicit RtpHandler(
    std::string sessionId, std::weak_ptr<Session> parentSessionPtr, std::weak_ptr<AcsHandler> acsHandlerPtr
  );
  ~RtpHandler();

private:
};

#endif //RTPHANDLER_H
