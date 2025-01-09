#include "../include/RtspHandler.h"

/*
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
*/

RtspHandler::RtspHandler(
  std::string inputSessionId,
  std::weak_ptr<Session> inputParentSessionPtr,
  std::weak_ptr<AcsHandler> inputAcsHandlerPtr
) : logger(Logger::getLogger(C::RTSP_HANDLER)),
    parentSessionPtr(inputParentSessionPtr),
    acsHandlerPtr(inputAcsHandlerPtr),
    sessionId(inputSessionId) {}

RtspHandler::~RtspHandler() {

}

void RtspHandler::shutdown() {
}

std::unique_ptr<Buffer> RtspHandler::run(std::unique_ptr<Buffer> inputBufferPtr) {
}

std::unique_ptr<Buffer> RtspHandler::handleRtspRequest(std::string reqStr) {
}

bool RtspHandler::hasSessionId(std::vector<std::string> strings) {
}

void RtspHandler::respondOptions(Buffer &buffer) {
}

void RtspHandler::respondDescribe(Buffer &buffer, std::string mediaInfo, std::string content) {
}

void RtspHandler::respondSetup(
  Buffer &buffer,
  std::string transport,
  std::string sessionId,
  int64_t ssrc,
  int trackId,
  int refVideoSampleCnt,
  int camDirectoryCnt
) {
}

void RtspHandler::respondSetupForHybrid(
  Buffer &buffer,
  std::string sessionId,
  std::string hybridMode
) {
}

void RtspHandler::respondPlay(
  Buffer &buffer,
  std::vector<int> seq,
  std::vector<int64_t> rtpTime,
  std::vector<std::string> urls,
  std::string sessionId
) {
}

void RtspHandler::respondPlayAfterPause(std::string sessionId) {
}

void RtspHandler::respondSwitching(std::string sessionId) {
}

void RtspHandler::respondCameraChange(std::string sessionId) {
}

void RtspHandler::respondBitrateChange(std::string sessionId) {
}

void RtspHandler::respondTeardown() {
}

void RtspHandler::respondError(int error, std::string rtspMethod) {
}

void RtspHandler::respondPause(Buffer &buffer) {
}

std::string RtspHandler::findUserName(std::vector<std::string> strings) {
}

int RtspHandler::findCSeq(std::vector<std::string> strings) {
}

std::string RtspHandler::findContents(std::string line0) {
}

std::string RtspHandler::getMediaInfo(std::string fullCid) {
}

int RtspHandler::findTrackId(std::string line0) {
}

std::string RtspHandler::findTransport(std::vector<std::string> strings) {
}

std::string RtspHandler::findHybridMode(std::vector<std::string> strings) {
}

std::string RtspHandler::findNotTx(std::vector<std::string> strings) {
}

std::vector<int> RtspHandler::findChannels(std::string transport) {
}

std::string RtspHandler::findSessionId(std::vector<std::string> strings) {
}

std::vector<float> RtspHandler::findNormalPlayTime(std::vector<std::string> strings) {
}

std::string RtspHandler::findDeviceModelName(std::vector<std::string> strings) {
}

std::string RtspHandler::findManufacturer(std::vector<std::string> strings) {
}

int RtspHandler::findLatestReceivecSampleIdx(
  std::vector<std::string> strings,std::string filter
) {
}

bool RtspHandler::isSeekRequest(std::vector<std::string> strings) {
}

bool RtspHandler::isValidPlayTime(std::vector<float> ntpSec) {
}

std::string RtspHandler::getContentsTitle(std::vector<std::string> urls) {
}

std::string RtspHandler::getSupportingBitrateTypes(std::vector<int> bitrateTypes) {
}

bool RtspHandler::isContainingPlayInfoHeader(std::vector<std::string> strings) {
}

bool RtspHandler::isThereMonitoringInfoHeader(std::vector<std::string> strings) {
}

void RtspHandler::parseHybridVideoSampleMetaDataForDandS(std::string notTxIdListStr) {
}