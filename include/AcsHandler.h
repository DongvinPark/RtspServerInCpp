#ifndef ACSHANDLER_H
#define ACSHANDLER_H

#include "../include/FileReader.h"
#include "../include/Session.h"
#include "../include/RtpInfo.h"
#include "../include/ReadInfo.h"

class Session;
class RtspHandler;
class RtpHandler;

class AcsHandler {
public:
  explicit AcsHandler(
    std::string sessionId,
    std::weak_ptr<Session> parentSessionPtr,
    ContentsStorage& parentContentsStorage
  );
  ~AcsHandler();

private:
  std::shared_ptr<Logger> logger;
  std::string sessionId;

  std::weak_ptr<FileReader> fileReaderPtr;

  // TODO : refer to comments in 'ReadInfo' class
  std::unordered_map<int, ReadInfo> sInfo{};
  RtpInfo rtpInfo;
  int camId = C::ZERO;

  int videoRtpRemoveCnt = C::ZERO;
};

#endif //ACSHANDLER_H
