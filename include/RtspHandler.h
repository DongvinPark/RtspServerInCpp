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

  void run(Buffer& inputBuffer);
  void handleRtspRequest(
    std::string reqStr, Buffer& inputBuffer
  );

private:
  bool hasSessionId(const std::vector<std::string>& strings);
  void respondOptions(Buffer& buffer);
  void respondDescribe(Buffer& buffer, const std::string& mediaInfo);
  void respondSetup(
    Buffer& buffer, std::string transport, std::string sessionId, int64_t ssrc,
    int trackId, int refVideoSampleCnt, int camDirectoryCnt
  );
  void respondSetupForHybrid(Buffer& buffer, std::string sessionId, std::string hybridMode);
  void respondPlay(
    Buffer& buffer, std::vector<int64_t> rtpTime, std::string sessionId
  );
  void respondPlayAfterPause(Buffer& buffer);
  void respondSwitching(Buffer& buffer);
  void respondCameraChange(Buffer& buffer, int targetCamId);
  void respondPFrameControl(Buffer& buffer, bool needToTxPFrames);
  void respondTeardown(Buffer& buffer);
  void respondError(Buffer& buffer, int error, const std::string& rtspMethod);
  void respondPause(Buffer& buffer);

  std::string findUserName(const std::vector<std::string>& strings);
  int findCSeq(const std::vector<std::string>& strings);
  std::string findContents(const std::string& line0);
  std::string getMediaInfo(const std::string& fullCid);
  int findTrackId(const std::string& line0);
  std::string findTransport(const std::vector<std::string>& strings);
  std::string findHybridMode(const std::vector<std::string>& strings);
  std::string findNotTx(const std::vector<std::string>& strings);
  std::vector<int> findChannels(const std::string& transport);
  std::string findSessionId(const std::vector<std::string>& strings);
  std::vector<float> findNormalPlayTime(const std::vector<std::string>& strings);
  std::string findDeviceModelName(const std::vector<std::string>& strings);
  std::string findManufacturer(const std::vector<std::string>& strings);
  bool isLookingSampleControInUse(const std::vector<std::string>& strings);
  int findLatestReceivedSampleIdx(const std::vector<std::string>& strings, const std::string& filter);
  bool isSeekRequest(const std::vector<std::string>& strings);
  bool isValidPlayTime(const std::vector<float>& ntpSec);
  std::string getContentsTitle(const std::vector<std::string>& urls);
  std::string getSupportingBitrateTypes(std::vector<int> bitrateTypes);
  bool isContainingPlayInfoHeader(const std::vector<std::string>& strings);
  bool isThereMonitoringInfoHeader(const std::vector<std::string>& strings);
  void parseHybridVideoSampleMetaDataForDandS(const std::string& notTxIdListStr);

  std::shared_ptr<Logger> logger;
  std::weak_ptr<Session> parentSessionPtr;
  std::weak_ptr<AcsHandler> acsHandlerPtr;

  std::string userName = C::EMPTY_STRING;
  std::string watingReq = C::EMPTY_STRING;

  int cSeq = C::UNSET;
  std::string sessionId = C::EMPTY_STRING;
  bool inSession = false;
  int wrongSessionIdRequestCnt = C::ZERO;

  std::string frontVideoTrackUrl = C::EMPTY_STRING;
  std::string audioTrackUrl = C::EMPTY_STRING;
  std::string rearVideoTrackUrl = C::EMPTY_STRING;
};

#endif //RTSPHANDLER_H
