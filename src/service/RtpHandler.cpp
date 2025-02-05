#include "../include/RtpHandler.h"

RtpHandler::RtpHandler(std::string sessionId,
  std::weak_ptr<Session> parentSessionPtr,
  std::weak_ptr<AcsHandler> acsHandlerPtr) {}

RtpHandler::~RtpHandler() {}

void RtpHandler::stopVideo() {}
void RtpHandler::stopAudio() {}
