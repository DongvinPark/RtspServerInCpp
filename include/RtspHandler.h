#ifndef RTSPHANDLER_H
#define RTSPHANDLER_H

#include "../include/Logger.h"
#include "../constants/C.h"
#include "../include/Buffer.h"

class Session;
class AcsHandler;

class RtspHandler {
public:
  explicit RtspHandler(
    std::string sessionId, std::weak_ptr<Session> inputParentSessionPtr, std::weak_ptr<AcsHandler> inputAcsHandlerPtr
  );
  ~RtspHandler();

  void shutdown();
  std::unique_ptr<Buffer> run(std::unique_ptr<Buffer> inputBufferPtr);
  std::unique_ptr<Buffer> handleRtspRequest(std::string reqStr);

private:

  std::shared_ptr<Logger> logger;
  std::weak_ptr<Session> parentSessionPtr;
  std::weak_ptr<AcsHandler> acsHandlerPtr;

  std::string userName = C::EMPTY_STRING;
  std::string watingReq = C::EMPTY_STRING;

  int cSeq = C::UNSET;
  std::string sessionId = C::EMPTY_STRING;
  int wrongSessionIdRequestCnt = C::ZERO;

  std::string frontVideoTrackUrl = C::EMPTY_STRING;
  std::string audioTrackUrl = C::EMPTY_STRING;
  std::string rearVideoTrackUrl = C::EMPTY_STRING;
};

#endif //RTSPHANDLER_H
