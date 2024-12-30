#ifndef RTSPHANDLER_H
#define RTSPHANDLER_H

#include "../include/Session.h"

class Session;
class AcsHandler;

class RtspHandler {
public:
  explicit RtspHandler(
    std::string sessionId, std::weak_ptr<Session> parentSessionPtr, std::weak_ptr<AcsHandler> acsHandlerPtr
  );
  ~RtspHandler();

private:
};

#endif //RTSPHANDLER_H
