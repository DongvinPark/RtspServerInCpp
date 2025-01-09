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
  bool hasSessionId(std::vector<std::string> strings);
  void respondOptions(Buffer& buffer);
  void respondDescribe(Buffer& buffer, std::string mediaInfo, std::string content);
  void respondSetup(
    Buffer& buffer, std::string transport, std::string sessionId, int64_t ssrc,
    int trackId, int refVideoSampleCnt, int camDirectoryCnt
  );
  void respondSetupForHybrid(Buffer& buffer, std::string sessionId, std::string hybridMode);
  void respondPlay(
    Buffer& buffer, std::vector<int> seq, std::vector<int64_t> rtpTime,
    std::vector<std::string> urls, std::string sessionId
  );
  void respondPlayAfterPause(std::string sessionId);
  void respondSwitching(std::string sessionId);
  void respondCameraChange(std::string sessionId);
  void respondBitrateChange(std::string sessionId);
  void respondTeardown();
  void respondError(int error, std::string rtspMethod);
  void respondPause(Buffer& buffer);

  std::string findUserName(std::vector<std::string> strings);
  int findCSeq(std::vector<std::string> strings);
  std::string findContents(std::string line0);
  std::string getMediaInfo(std::string fullCid);
  int findTrackId(std::string line0);
  std::string findTransport(std::vector<std::string> strings);
  std::string findHybridMode(std::vector<std::string> strings);
  std::string findNotTx(std::vector<std::string> strings);
  std::vector<int> findChannels(std::string transport);
  std::string findSessionId(std::vector<std::string> strings);
  std::vector<float> findNormalPlayTime(std::vector<std::string> strings);
  std::string findDeviceModelName(std::vector<std::string> strings);
  std::string findManufacturer(std::vector<std::string> strings);
  int findLatestReceivecSampleIdx(std::vector<std::string> strings, std::string filter);
  bool isSeekRequest(std::vector<std::string> strings);
  bool isValidPlayTime(std::vector<float> ntpSec);
  std::string getContentsTitle(std::vector<std::string> urls);
  std::string getSupportingBitrateTypes(std::vector<int> bitrateTypes);
  bool isContainingPlayInfoHeader(std::vector<std::string> strings);
  bool isThereMonitoringInfoHeader(std::vector<std::string> strings);
  void parseHybridVideoSampleMetaDataForDandS(std::string notTxIdListStr);

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
