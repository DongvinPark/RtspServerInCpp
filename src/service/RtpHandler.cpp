#include "../include/RtpHandler.h"

#include <iostream>

RtpHandler::RtpHandler(std::string sessionId,
                       std::weak_ptr<Session> parentSessionPtr,
                       std::weak_ptr<AcsHandler> acsHandlerPtr) {}

RtpHandler::~RtpHandler(){
  std::cout << "!!! RtpHandler destructor called" << std::endl;
}

void RtpHandler::stopVideo() {}
void RtpHandler::stopAudio() {}
