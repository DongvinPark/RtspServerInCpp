#ifndef ACSHANDLER_H
#define ACSHANDLER_H

#include "../include/Session.h"

class Session;
class RtspHandler;
class RtpHandler;

class AcsHandler {
public:
  explicit AcsHandler(std::string sessionId, std::weak_ptr<Session> parentSessionPtr);
  ~AcsHandler();

private:
};

#endif //ACSHANDLER_H
