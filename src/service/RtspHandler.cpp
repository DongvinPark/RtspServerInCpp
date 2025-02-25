#include "../include/RtspHandler.h"

#include "../../constants/Util.h"
#include "../../include/Session.h"
#include "../include/HybridSampleMeta.h"
#include "../include/RtpHandler.h"

#include <cmath>
#include <iostream>

RtspHandler::RtspHandler(
  std::string inputSessionId,
  std::weak_ptr<Session> inputParentSessionPtr,
  std::weak_ptr<AcsHandler> inputAcsHandlerPtr
) : logger(Logger::getLogger(C::RTSP_HANDLER)),
    parentSessionPtr(inputParentSessionPtr),
    acsHandlerPtr(inputAcsHandlerPtr),
    sessionId(inputSessionId) {}

RtspHandler::~RtspHandler() = default;

void RtspHandler::run(Buffer& inputBuffer) {
  std::string req = inputBuffer.getString();
  logger->warning("Dongvin, " + sessionId + ", rtsp req: ");
  for (auto& reqLine : Util::splitToVecByStringForRtspMsg(req, C::CRLF)) {
    logger->info(reqLine);
  }
  std::cout << "\n";

  handleRtspRequest(req, inputBuffer);
}

void RtspHandler::handleRtspRequest(
  std::string reqStr, Buffer& inputBuffer
) {
  int idx = reqStr.find(' ');
  std::string method = reqStr.substr(0, idx);
  if (std::find(C::RTSP_METHOD_VECTOR.begin(), C::RTSP_METHOD_VECTOR.end(), method) == C::RTSP_METHOD_VECTOR.end()) {
    logger->severe("Dongvin, not implemented method! : " + method);
    respondError(inputBuffer, C::METHOD_NOT_ALLOWED, method);
    return;
  }

  std::vector<std::string> strings = Util::splitToVecByStringForRtspMsg(reqStr, C::CRLF);
  int _cSeq = findCSeq(strings);
  if (_cSeq == -1) {
    // invalid CSeq. bad req.
    logger->severe("Dongvin, failed to find CSeq header!");
    respondError(inputBuffer, C::BAD_REQUEST, method);
    return;
  }
  int next = cSeq + 1;
  if (next != _cSeq) {
    logger->severe("Dongvin, bad sequence number! server side/client side : " + std::to_string(next) + "/" + std::to_string(_cSeq));
    respondError(inputBuffer, C::BAD_REQUEST, method);
    return;
  }

  if (auto sessionPtr = parentSessionPtr.lock()) {
    if (auto ptrForAcsHandler = acsHandlerPtr.lock()) {
      // do not allowed proceeding without session id once session is set up.
      // Once session id is given to a client, all the following requests must
      // include the session id in the request.
      if (inSession && !hasSessionId(strings)) {
        wrongSessionIdRequestCnt++;
        if (wrongSessionIdRequestCnt >= C::WRONG_SESSION_ID_TOLERANCE_CNT) {
          // some kind of punks are sending wrong requests on the port assigned to
          // my client. I shut down this port to protect my client's QOE. Not answer to
          // the bad guy any longer so as not to give any hint of the correct session id.
          logger->warning(
            "Dongvin, illegal requests with invalid session id are given "
            + std::to_string(C::WRONG_SESSION_ID_TOLERANCE_CNT)+" times in total. "
            +"We believe there is something wrong. We shut down this connection.");

          // throw runtime exception
          throw std::runtime_error(
            "Dongvin, shutting down malicious session! remote addr : "
                + sessionPtr->getClientRemoteAddress()
          );
          return;
        }
        logger->severe("Dongvin, failed to find session id!");
        respondError(inputBuffer, C::BAD_REQUEST, method);
        return;
      }

      cSeq = _cSeq;

      if (method == "OPTIONS") {
        if (isThereMonitoringInfoHeader(strings)) {
          // t: NTP time at which OPTION method was sent. (ms)
          // sa: The amount of samples received by the client. (bitrate, utc time(ms))
          // sr: The rate at which the client receives samples. (rate, utc time(ms))
          // e.g. MonitoringInfo: t=1719479800907;sa=7731176,1719479798405,1099016,1719479799406;sr=32040696,1719479798387,25152971,1719479798487
          std::string info = strings[strings.size()-1];
          std::vector<std::string> words =
            Util::splitToVecBySingleChar(
              Util::trim( Util::splitToVecBySingleChar(info, ':')[1] ), ';'
            );

          for (std::string w : words) {
            if (w.rfind("rb", 0) == 0) {
              std::vector<std::string> bitrateAndTime =
                Util::splitToVecBySingleChar(
                  Util::splitToVecBySingleChar(w, '=')[1], ','
                );

              for (auto i=0; i < bitrateAndTime.size(); i+=2) {
                RxBitrate rxBitrate(
                  std::stoll(bitrateAndTime[i]), std::stoll(bitrateAndTime[i+1])
                );
                sessionPtr->addRxBitrate(rxBitrate);
              }

            }
          }
        }

        userName = findUserName(strings);
        std::string cid = findContents(strings[0]);
        if (sessionPtr->onCid(cid)) {
          respondOptions(inputBuffer);
          return;
        }
        logger->severe("Dongvin, server doesn't have content : " + cid);
        respondError(inputBuffer, C::BAD_REQUEST, method);
        return;
      } else if (method == "DESCRIBE") {
        // assume url and username are correct or same. Don't check again.
        std::string fullCid = Util::splitToVecBySingleChar(strings[0], ' ')[1];
        std::string mediaInfo = getMediaInfo(fullCid);
        respondDescribe(inputBuffer, mediaInfo);
      } else if (method == "SETUP") {
        // assume url and username are correct or same. Don't check again.
        std::string transport = findTransport(strings);
        std::string hybridMode = findHybridMode(strings);
        std::string notToTxList = findNotTx(strings);
        if (transport == C::EMPTY_STRING && hybridMode == C::EMPTY_STRING) {
          logger->severe("Dongvin, invalid SETUP header!");
          respondError(inputBuffer, C::BAD_REQUEST, method);
          return;
        }

        if (transport != C::EMPTY_STRING && hybridMode == C::EMPTY_STRING) {
          // process the setup req on track ids.
          int trackId = findTrackId(strings[0]);
          std::vector<int> channels = findChannels(transport);

          // don't make new thread for reading member videos.
          // the thread reading ref front video samples must read the member video samples too.
          if (trackId <= C::AUDIO_ID) {
            sessionPtr->onChannel(trackId, channels);
          }

          int ssrcIdx;
          if (trackId > C::AUDIO_ID) {
            ssrcIdx = C::VIDEO_ID;
          } else {
            ssrcIdx = trackId;
          }

          // dongvin : record content's title at Session object.
          if (sessionPtr->getContentTitle() == C::EMPTY_STRING) {
            std::string contentTitle = getContentsTitle(ptrForAcsHandler->getStreamUrls());
            if (contentTitle != C::EMPTY_STRING) {
              sessionPtr->updateContentTitleOfCurSession(contentTitle);
            }
          }

          std::vector<int64_t> ssrc = ptrForAcsHandler->getSsrc();
          respondSetup(
            inputBuffer, transport, sessionId, ssrc[ssrcIdx], trackId,
            sessionPtr->getRefVideoSampleCnt(), sessionPtr->getNumberOfCamDirectories()
          );
          return;
        } else {
          // process not tx sample numbers for hybrid D & S.
          parseHybridVideoSampleMetaDataForDandS(notToTxList);
          respondSetupForHybrid(inputBuffer, sessionId, hybridMode);
          return;
        }
      } else if (method == "PLAY") {
        if (isContainingPlayInfoHeader((strings))) {
          // dongvin : receiving play req to resume play from pause state

          if (sessionPtr->getPauseStatus()) {
            // if session is in pause state

            // 헤더를 파싱해서 클라이언트가 PAUSE 요청 날리기 직전까지 수신 완료한 가장 최근 샘플의 idx들을 알아낸다.
            // Jenny, sample index can be -1
            int receivedVideoSampleIdx = findLatestReceivedSampleIdx(strings, "videoIndex");
            int receivedAudioSampleIdx = findLatestReceivedSampleIdx(strings, "audioIndex");
            int receivedVideoRtpIdx = findLatestReceivedSampleIdx(strings, "videoRtpIndex");

            // video & audio readInfo 내의 curSampeNo를 초기화 한다.
            // received idx 들은 클라이언트가 이미 수신 완료한 것이므로, 이것 바로 다음 샘플부터 보내게 만든다.
            if(receivedVideoSampleIdx != -1) ptrForAcsHandler->updateCurSampleNo(
              C::VIDEO_ID, receivedVideoSampleIdx + 1
            );
            if(receivedAudioSampleIdx != -1) ptrForAcsHandler->updateCurSampleNo(
              C::AUDIO_ID, receivedAudioSampleIdx + 1
            );

            if (receivedVideoRtpIdx > 0) {
              ptrForAcsHandler->updateRtpRemoteCnt(receivedVideoRtpIdx);
            }
          }
          respondPlayAfterPause(inputBuffer);
          sessionPtr->updatePauseStatus(false);
          return;
        } else if (isSeekRequest(strings)) {
          // dongvin, play req for Seek operation
          sessionPtr->updatePauseStatus(false);
          sessionPtr->stopCurrentMediaReadingTasks(true);

          std::vector<float> npt = findNormalPlayTime(strings);
          if (npt.empty() || !isValidPlayTime(npt)) {
            std::string nptElem = "[";
            for (float f : npt) {
              nptElem += std::to_string(f);
              nptElem += ",";
            }
            nptElem += "]";
            logger->severe("Dongvin, invalid npt in play req for seek : " + nptElem);
            respondError(inputBuffer, C::BAD_REQUEST, method);
            return;
          }
          // valid npt
          sessionPtr->onUserRequestingPlayTime(npt);

          std::vector<int64_t> timestamp0 = ptrForAcsHandler->getTimestamp();
          respondPlay(inputBuffer, timestamp0, sessionId);

          Util::delayedExecutorAsyncByIoContext(
            sessionPtr->getIoContext(),
            C::DELAY_BEFORE_RTP_START_ON_SEEK,
            [sessionPtr](){sessionPtr->onPlayStart();}
          );
          return;
        } else {
          // dongvin, process initial play req.
          sessionPtr->updatePlayTimeDurationMillis(
            std::max(
              ptrForAcsHandler->getPlayTimeUs(C::VIDEO_ID)/1000,
              ptrForAcsHandler->getPlayTimeUs(C::AUDIO_ID)/1000
            )
          );

          std::vector<float> npt = findNormalPlayTime(strings);

          if (!isLookingSampleControInUse(strings)){
            sessionPtr->updatePFrameTxStatus(true);
          }

          if (sessionPtr->getDeviceModelNo() == C::EMPTY_STRING) {
            std::string deviceName = findDeviceModelName(strings);
            if (deviceName != C::EMPTY_STRING) {
              sessionPtr->updateDeviceModelNo(deviceName);
            }
          }

          if (sessionPtr->getManufacturer() == C::EMPTY_STRING) {
            std::string manufacturer = findManufacturer(strings);
            if (manufacturer != C::EMPTY_STRING) {
              sessionPtr->updateManufacturer(manufacturer);
            }
          }

          if (npt.empty() || !isValidPlayTime(npt)) {
            std::string nptElem = "[";
            for (float f : npt) {
              nptElem += std::to_string(f);
              nptElem += ",";
            }
            nptElem += "]";
            logger->severe("Dongvin, invalid npt in play req for initial play : " + nptElem);
            respondError(inputBuffer, C::BAD_REQUEST, method);
            return;
          } else {

            // open all video and audio std::ifstream.
            std::weak_ptr<RtpHandler> weakPtr = sessionPtr->getRtpHandlerPtr();
            if (auto rtpHandlePtr = weakPtr.lock()) {
              if (bool initResult = rtpHandlePtr->openAllFileStreamsForVideoAndAudio(); !initResult) {
                logger->severe("Dongvin, failed to open video/audio file stream! session id : " + sessionId);
                respondError(inputBuffer, C::BAD_REQUEST, method);
                return;
              }
            } else {
              logger->severe("Dongvin, failed to get RtpHandler ptr!");
              respondError(inputBuffer, C::BAD_REQUEST, method);
              return;
            }

            ptrForAcsHandler->setCamId(0);
            sessionPtr->onUserRequestingPlayTime(npt);

            std::vector<int64_t> timestamp0 = ptrForAcsHandler->getTimestamp();
            respondPlay(inputBuffer, timestamp0, sessionId);

            Util::delayedExecutorAsyncByIoContext(
              sessionPtr->getIoContext(),
              C::DELAY_BEFORE_RTP_START,
              [sessionPtr](){sessionPtr->onPlayStart();}
            );

            sessionPtr->updatePauseStatus(false);
            return;
          }
          inSession = true;
        }// end of else for initial play
      } else if (method == "PAUSE") {
        // dongvin, if pause req come at puase state, just return 200 OK response.
        if (sessionPtr->getPauseStatus()) {
          respondPause(inputBuffer);
          return;
        }
        sessionPtr->updatePauseStatus(true);
        respondPause(inputBuffer);
        return;
      } else if (method == "TEARDOWN") {
        respondTeardown(inputBuffer);
        inSession = false;
        wrongSessionIdRequestCnt = 0;
        return;
      } else if (method == "SET_PARAMETER") {
        // last string is the switching info.
        // for instance, SwitchingInfo: next=1;tv=1234567;ta=45678900
        // CameraInfo: cam=0;next=1;tv=1234567;ta=45678900

        const std::string& info = strings[strings.size() - 1];
        std::vector<std::string> words = Util::splitToVecBySingleChar(
          Util::trim(Util::splitToVecBySingleChar(info, ':')[1]), ';'
        );

        // tvIdx : video's sample time index in client side
        // taIdx : audio's sample time index in client side

        int64_t tvIdx = C::UNSET;
        int64_t taIdx = C::UNSET;
        int nextVid = C::UNSET;
        int cam = C::UNSET;
        int targetBitrate = C::UNSET;
        bool sampleLimitForPauseSwitching = false;
        std::string pFrameControlAction = C::EMPTY_STRING;
        for (std::string w : words) {
          if(w.rfind("tvIdx", 0) == 0) tvIdx = std::stoll(Util::splitToVecBySingleChar(w, '=')[1]);
          else if(w.rfind("taIdx", 0) == 0) taIdx = std::stoll(Util::splitToVecBySingleChar(w, '=')[1]);
          else if(w.rfind("next", 0) == 0) nextVid = std::stoi(Util::splitToVecBySingleChar(w, '=')[1]);
          else if(w.rfind("cam", 0) == 0) cam = std::stoi(Util::splitToVecBySingleChar(w, '=')[1]);
          else if(w.rfind("tBit", 0) == 0) targetBitrate = std::stoi(Util::splitToVecBySingleChar(w, '=')[1]);
          else if(w.rfind("limitSamples", 0) == 0) {
            std::string boolStr = Util::splitToVecBySingleChar(w, '=')[1];
            sampleLimitForPauseSwitching = boolStr == "true" ? true : false;
          }
          else if(w.find("-p") != std::string::npos ) pFrameControlAction = w;
        }
        std::vector<int64_t> switchingInfo = {tvIdx, taIdx};

        logger->info2(
          "Dongvin, id:"+sessionId+", switching request comes in!, " +
              "cam: "+ std::to_string(cam)+"vid: "+std::to_string(nextVid)+", time: "
              + "["+std::to_string(switchingInfo[0])+","+std::to_string(switchingInfo[1])+"] (us)"
        );

        bool isValid = false;
        if (info.find(C::CAM_CHANG_KEY) != std::string::npos) {
          sessionPtr->updateIsInCamSwitching(true);
          int maxCam = ptrForAcsHandler->getMaxCamNumber();
          int maxVnum = ptrForAcsHandler->getMainVideoNumber();
          isValid = cam < maxCam && nextVid < maxVnum;
          if (!isValid) {
            logger->info("Dongvin, invalid next id or video id for cam change!");
            sessionPtr->updateIsInCamSwitching(false);
            respondError(inputBuffer, C::BAD_REQUEST, method);
            return;
          }
          respondCameraChange(inputBuffer, cam);
          sessionPtr->onCameraChange(cam, nextVid, switchingInfo);
          Util::delayedExecutorAsyncByIoContext(
              sessionPtr->getIoContext(),
              C::UPDATE_CAM_SWITCHING_STATUS_DELAY_MILLIS,
              [sessionPtr](){sessionPtr->updateIsInCamSwitching(false);}
          );
          return;
        } else if (info.find(C::P_FRAME_KEY) != std::string::npos) {
          logger->info2(
            "Dongvin, id:" + sessionId+", looking sample control request comes in! : " + pFrameControlAction
          );
          bool pFrameTxMode = pFrameControlAction == C::SEND_P_FRAMES ? true : false;
          sessionPtr->updatePFrameTxStatus(pFrameTxMode);
          respondPFrameControl(inputBuffer, pFrameTxMode);
          return;
        } else {
          logger->info("Dongvin, invalid set_parameter header!");
          respondError(inputBuffer, C::BAD_REQUEST, method);
          return;
        }
      } else {
        logger->warning("Dongvin, invalid or not implemented method: " + method);
        respondError(inputBuffer, C::BAD_REQUEST, method);
        return;
      }

      if (!inputBuffer.getString().empty()) {
        logger->info("Dongvin, id : " + sessionId + ", rtsp response: \n" + inputBuffer.getString());
      }
      return;
    }
    logger->severe("Dongvin, failed to get weak acs handler ptr!");
    respondError(inputBuffer, C::INTERNAL_SERVER_ERROR, method);
    return;
  }
  logger->severe("Dongvin, failed to get weak session ptr!");
  respondError(inputBuffer, C::INTERNAL_SERVER_ERROR, method);
}// end of handleRtspRequest();

bool RtspHandler::hasSessionId(const std::vector<std::string>& strings) {
  std::string _sessionId = findSessionId(strings);
  logger->info("Dongvin, session in rtsp request " + _sessionId);
  return (!_sessionId.empty() && _sessionId == sessionId);
}

void RtspHandler::respondOptions(Buffer &buffer) {
  std::string rsp;

  rsp = "RTSP/1.0 200 OK" + std::string{C::CRLF};
  rsp += "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF};

  std::string rtspMethods = "Public: ";
  const auto loopCnt = C::RTSP_METHOD_VECTOR.size();
  for (int i=0; i<loopCnt; i++) {
    rtspMethods += C::RTSP_METHOD_VECTOR[i];
    if (i < (loopCnt-1)) rtspMethods += ",";
  }

  rsp += rtspMethods + std::string{C::CRLF};
  rsp += "Server: " + std::string{C::MY_NAME} + std::string{C::CRLF2};

  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
}

void RtspHandler::respondDescribe(Buffer &buffer, const std::string& mediaInfo) {
  std::string rsp = "RTSP/1.0 200 OK" + std::string{C::CRLF} +
                "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
                "Content-Base: " + C::DUMMY_CONTENT_BASE +"/"+ std::string{C::CRLF} +
                "Content-Length: " + std::to_string(mediaInfo.length()) + std::string{C::CRLF} +
                "Content-Type: application/sdp" + std::string{C::CRLF} +
                "Server: " +C::MY_NAME + std::string{C::CRLF2} +
                mediaInfo; // don't append CRLF according to AVPT 6.1 implementation
  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
  buffer.bodyLen = mediaInfo.length();
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
  logger->info("Dongvin, setup stream id : " + std::to_string(trackId));
  std::string rsp = "RTSP/1.0 200 OK" + std::string{C::CRLF} +
                "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
                "Server: "+ std::string{C::MY_NAME} + std::string{C::CRLF} +
                "Session: "+ sessionId + std::string{C::CRLF} +
                "RefVideoSampleCnt: " + std::to_string(refVideoSampleCnt) + std::string{C::CRLF} +
                "camDirectoryCnt: " + std::to_string(camDirectoryCnt) + std::string{C::CRLF} +
                "Transport: " + transport+";ssrc="+ std::to_string(ssrc) + std::string{C::CRLF2};
  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
}

void RtspHandler::respondSetupForHybrid(
  Buffer &buffer,
  std::string sessionId,
  std::string hybridMode
) {
  std::string rsp = "RTSP/1.0 200 OK" + std::string{C::CRLF} +
                "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
                "Server: "+ std::string{C::MY_NAME} + std::string{C::CRLF} +
                "Session: "+ sessionId + std::string{C::CRLF} +
                "HybridMode: "+ hybridMode + std::string{C::CRLF2};
  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
}

void RtspHandler::respondPlay(
  Buffer &buffer,
  std::vector<int64_t> rtpTime,
  std::string sessionId
) {
  bool initResult = false;
  int lastVideoSampleNo = C::UNSET;
  int lastAudioSampleNo = C::UNSET;
  std::string supportingBitrateType = C::EMPTY_STRING;
  int numberOfCamDirectories = C::UNSET;
  if (auto acsHandler = acsHandlerPtr.lock()) {
    lastVideoSampleNo = acsHandler->getLastVideoSampleNumber();
    lastAudioSampleNo = acsHandler->getLastAudioSampleNumber();
    if (lastVideoSampleNo > C::ZERO && lastAudioSampleNo > C::ZERO) initResult = true;
    else {
      logger->severe("Dongvin, sample meta init failed!");
      respondError(buffer, C::INTERNAL_SERVER_ERROR, "PLAY");
      return;
    }
  }
  if (auto sessionPtr = parentSessionPtr.lock()) {
    supportingBitrateType = getSupportingBitrateTypes(sessionPtr->get_mbpsTypeList());
    numberOfCamDirectories = sessionPtr->getNumberOfCamDirectories();
    if (numberOfCamDirectories > C::ZERO) initResult = true;
    else {
      logger->severe("Dongvin, session meta init failed!");
      respondError(buffer, C::INTERNAL_SERVER_ERROR, "PLAY");
    }
  }

  std::string rsp;
  if (initResult) {
    rsp = "RTSP/1.0 200 OK" + std::string{C::CRLF} +
      "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
      "RTP-Info: " +

      "url=" + C::DUMMY_CONTENT_BASE + "/trackID=0" + // front video
      ";rtptime=" + std::to_string(rtpTime[0]) +
      ";lsnum=" + std::to_string(lastVideoSampleNo) +

      ",url=" + C::DUMMY_CONTENT_BASE + "/trackID=1" + // audio
      ";rtptime=" + std::to_string(rtpTime[1]) +
      ";lsnum=" + std::to_string(lastAudioSampleNo) +

      ",url=" + C::DUMMY_CONTENT_BASE + "/trackID=2" + // member videos(ex : rear video)
      ";rtptime=" + std::to_string(rtpTime[0]) +
      ";lsnum=" + std::to_string(lastVideoSampleNo) + std::string{C::CRLF} +

      "Server: " + std::string{C::MY_NAME} + std::string{C::CRLF} +
      "Session: " + sessionId + std::string{C::CRLF} +
      "SupportingBitrate: " + supportingBitrateType + std::string{C::CRLF} +
      "CamDirectoryCnt: " + std::to_string(numberOfCamDirectories) + std::string{C::CRLF2};
    const std::vector<unsigned char> response(rsp.begin(), rsp.end());
    buffer.updateBuf(response);
  } else {
    logger->severe("Dongvin, Failed to init Rtp-Info header for PLAY request");
    respondError(buffer, C::INTERNAL_SERVER_ERROR, "PLAY");
  }
}

void RtspHandler::respondPlayAfterPause(Buffer& buffer) {
  std::string rsp = "RTSP/1.0 200 OK"+std::string{C::CRLF} +
            "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
            "Server: "+ std::string{C::MY_NAME} + std::string{C::CRLF}+
            "Session: "+ sessionId + std::string{C::CRLF2};
  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
}

void RtspHandler::respondSwitching(Buffer& buffer) {
  std::string rsp = "RTSP/1.0 200 OK"+std::string{C::CRLF}+
                "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
                "Session: "+ sessionId + std::string{C::CRLF} +
                "Server: "+ std::string{C::MY_NAME} + std::string{C::CRLF2};
  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
}

void RtspHandler::respondCameraChange(Buffer& buffer, int targetCamId) {
  std::string rsp = "RTSP/1.0 200 OK"+std::string{C::CRLF}+
                "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
                "Session: "+ sessionId + std::string{C::CRLF} +
                C::CAM_CHANG_KEY + ": " + std::to_string(targetCamId) + std::string{C::CRLF} +
                "Server: "+ std::string{C::MY_NAME} + std::string{C::CRLF2};
  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
}

void RtspHandler::respondPFrameControl(Buffer& buffer, bool needToTxPFrames){
  std::string rsp = "RTSP/1.0 200 OK"+std::string{C::CRLF}+
                "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
                "Server: "+ std::string{C::MY_NAME} + std::string{C::CRLF} +
                C::P_FRAME_KEY + ": " + std::to_string(needToTxPFrames) + std::string{C::CRLF2};
  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
}

void RtspHandler::respondTeardown(Buffer& buffer) {
  std::string rsp = "RTSP/1.0 200 OK" + std::string{C::CRLF} +
            "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
            "Server: "+ std::string{C::MY_NAME} + std::string{C::CRLF} +
            "Teardown: true" + std::string{C::CRLF2};
  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
}

void RtspHandler::respondError(Buffer& buffer, int error, const std::string& rtspMethod) {
  std::string rsp = "RTSP/1.0 " + std::to_string(error) + " " + C::RTSP_STATUS_CODES_MAP.at(error) + std::string{C::CRLF} +
                "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
                "Server: " + std::string{C::MY_NAME} + std::string{C::CRLF} +
                "Error: " + std::string{C::MY_NAME} + std::string{C::CRLF2};
  logger->warning(
    "Dongvin, session id:"+ sessionId +
    ", send error (" + std::to_string(error) + ") response for " + rtspMethod
    );
  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
}

void RtspHandler::respondPause(Buffer &buffer) {
  std::string rsp = "RTSP/1.0 200 OK" + std::string{C::CRLF} +
            "CSeq: " + std::to_string(cSeq) + std::string{C::CRLF} +
            "Server: "+ std::string{C::MY_NAME} + std::string{C::CRLF2};
  const std::vector<unsigned char> response(rsp.begin(), rsp.end());
  buffer.updateBuf(response);
}

std::string RtspHandler::findUserName(const std::vector<std::string>& strings) {
  for (const std::string& elem : strings) {
    if (elem.find("User-Agent") != std::string::npos) {
      return Util::trim( Util::splitToVecBySingleChar(elem, ':')[1] );
    }
  }
  return C::EMPTY_STRING;
}

int RtspHandler::findCSeq(const std::vector<std::string>& strings) {
  for (const std::string& elem : strings) {
    if (elem.find("CSeq") != std::string::npos) {
      return std::stoi(
        Util::trim( Util::splitToVecBySingleChar(elem, ':')[1] )
      );
    }
  }
  return C::INVALID;
}

std::string RtspHandler::findContents(const std::string& line0) {
  const std::string address = Util::splitToVecBySingleChar(line0, ' ')[1];
  const std::vector<std::string> words = Util::splitToVecBySingleChar(address, ':');
  if (words.size() != C::URL_SPLIT_BY_SEMI_COLON_LENGTH) {
    logger->severe("Invalid Title!");
    return C::EMPTY_STRING;
  }
  return Util::trim(
    Util::splitToVecBySingleChar(words[2], '/')[1]
  );
}

std::string RtspHandler::getMediaInfo(const std::string& fullCid) {
  if (auto handlerPtr = acsHandlerPtr.lock()) {

    const std::vector<int64_t> unitCnt = handlerPtr->getUnitFrameCount();
    const std::vector<int64_t> gop = handlerPtr->getGop();

    std::string mediaInfo = handlerPtr->getMediaInfo();
    std::vector<std::string> lines = Util::splitToVecByString(mediaInfo, C::CRLF);

    for (auto i = 0; i < lines.size(); ++i) {
      std::string line = lines[i];
      if (line == C::EMPTY_STRING) continue;

      if (line.rfind("c=", 0) == 0) {
        lines[i] = "c=IN IP4 0.0.0.0"; // don't need to know ip addr of client
      } else if (line.rfind("a=control:", 0) == 0){
        int trackId = std::stoi(Util::splitToVecBySingleChar(line, '=')[2]);
        std::string track = "/trackID="+std::to_string(trackId);
        lines[i] = "a=control:"+std::string{C::DUMMY_CONTENT_BASE}+track;
        if(trackId <= C::AUDIO_ID){
          handlerPtr->setStreamUrl(trackId, fullCid+track);
        }
      } else if (line.rfind("AS", 0) == 0){ // application specific for bandwidth.
        lines[i] = C::EMPTY_STRING;
      } else if (line.rfind("a=tool:", 0) == 0) {
        lines[i] = C::EMPTY_STRING;
      } else if (line.rfind("a=fmtp:", 0) == 0) {
        if(Util::trim( Util::splitToVecBySingleChar(line, ':')[1] ).rfind("97", 0) == 0){ // audio
          // refer to Table 9 (streamType Values) in ISO/IEC 14496-1 (coding of audio-visual objects)
          lines[i] +=";streamType=5";
          lines[i] += ";ucnt="+std::to_string(unitCnt[1]);
        } else if (Util::trim( Util::splitToVecBySingleChar(line, ':')[1] ).rfind("96", 0) == 0){ // video
          // refer to Table 9 (streamType Values) in ISO/IEC 14496-1 (coding of audio-visual objects)
          lines[i] +=";streamType=4";
          lines[i] += ";dGop="+std::to_string(gop[0]);
          lines[i] += ";ucnt="+std::to_string(unitCnt[0]);
        }
      } else if (line.rfind("b=AS:", 0) == 0) {
        // dongvin : 최초 재생 시 사용하게 될 bitrate를 기록해 둔다.
        int kbps = std::stoi(Util::splitToVecBySingleChar(line, ':')[1] );
        if (auto sessionPtr = parentSessionPtr.lock()) {
          sessionPtr->add_kbpsBitrateValue(kbps);
        }
      }
    }//for

    std::string result = C::EMPTY_STRING;
    for (const std::string& line : lines) {
      if (line == C::EMPTY_STRING) {
        continue;
      }
      result += (line + std::string{C::CRLF});
    }

    return result;
  }
  logger->severe("Dongvin, failed to get AcsHandler weak ptr lock!");
  return C::EMPTY_STRING;
}

int RtspHandler::findTrackId(const std::string& line0) {
  const std::string addr = Util::splitToVecBySingleChar(line0, ' ')[1];
  return std::stoi( Util::splitToVecBySingleChar(addr, '=')[1] );
}

std::string RtspHandler::findTransport(const std::vector<std::string>& strings) {
  // required
  // refer to https://www.rfc-editor.org/rfc/rfc2326.html#section-12.39
  for (const std::string& elem : strings) {
    if ( elem.find("Transport:") != std::string::npos) {
      return Util::splitToVecBySingleChar(elem, ' ')[1];
    }
  }
  return C::EMPTY_STRING;
}

std::string RtspHandler::findHybridMode(const std::vector<std::string>& strings) {
  for (const std::string& elem : strings) {
    if ( elem.find("HybridMode") != std::string::npos) {
      std::string mode = Util::splitToVecBySingleChar(elem, ' ')[1];
      if (C::HYBRID_MODE_SET.find(mode) != C::HYBRID_MODE_SET.end()) {
        return mode;
      } else {
        return C::EMPTY_STRING;
      }
    }
  }
  return C::EMPTY_STRING;
}

std::string RtspHandler::findNotTx(const std::vector<std::string>& strings) {
  for (const std::string& elem : strings) {
    if ( elem.find("NotTx") != std::string::npos) {
      return Util::splitToVecBySingleChar(elem, ' ')[1];
    }
  }
  return C::EMPTY_STRING;
}

std::vector<int> RtspHandler::findChannels(const std::string& transport) {
  std::string range = Util::splitToVecBySingleChar(transport, '=')[1];
  std::vector<std::string> channels = Util::splitToVecBySingleChar(range, '-');
  std::vector<int> channelVec;
  channelVec.push_back(std::stoi(channels[0]));
  channelVec.push_back(std::stoi(channels[1]));
  return channelVec;
}

std::string RtspHandler::findSessionId(const std::vector<std::string>& strings) {
  for (const std::string& elem : strings) {
    if (elem.find("Session:") != std::string::npos) {
      return Util::trim( Util::splitToVecBySingleChar(elem, ':')[1] );
    }
  }
  return C::EMPTY_STRING;
}

std::vector<float> RtspHandler::findNormalPlayTime(const std::vector<std::string>& strings) {
  // optional.
  // refer to https://www.rfc-editor.org/rfc/rfc2326.html#section-3.6 chapter
  for (const std::string& line : strings) {
    if (line.find("npt") != std::string::npos) {
      const std::vector<std::string> rangePart = Util::splitToVecBySingleChar(line, '=');
      const std::vector<std::string> range = Util::splitToVecBySingleChar(rangePart[1], '-');
      std::vector<float> nptVector;
      nptVector.push_back(std::stof(range[0]));
      nptVector.push_back(range.size() == 2 ? std::stof(range[1]) : -1);
      return nptVector;
    }
  }
  return {};
}

std::string RtspHandler::findDeviceModelName(const std::vector<std::string>& strings) {
  for (const std::string& elem : strings) {
    if ( elem.find("ModelNo") != std::string::npos) {
      return Util::splitToVecBySingleChar(elem, ' ')[1];
    }
  }
  return C::EMPTY_STRING;
}

std::string RtspHandler::findManufacturer(const std::vector<std::string>& strings) {
  for (const std::string& elem : strings) {
    if ( elem.find("Manufacturer") != std::string::npos) {
      return Util::splitToVecBySingleChar(elem, ' ')[1];
    }
  }
  return C::EMPTY_STRING;
}

bool RtspHandler::isLookingSampleControInUse(const std::vector<std::string>& strings){
  for (const std::string& elem : strings) {
    if ( elem.find(C::USE_P_FRAME_CONTROL) != std::string::npos) {
      if (Util::splitToVecBySingleChar(elem, ' ')[1] == "true") {
        return true;
      }
    }
  }
  return false;
}

// for play req right after pause
int RtspHandler::findLatestReceivedSampleIdx(
  const std::vector<std::string>& strings, const std::string& filter
) {
  // PLAY req header example
  // PlayInfo: videoIndex=144;videoRtpIndex=8;audioIndex=228;
  for (const std::string& headerLine: strings) {
    if (headerLine.rfind("PlayInfo", 0) == 0) {
      std::string HeaderValue = Util::splitToVecByString(headerLine, " ")[1];
      for (const std::string& value: Util::splitToVecBySingleChar(headerLine, ';')) {
        if (value.find(filter) != std::string::npos) {
          return std::stoi(Util::splitToVecBySingleChar(value, '=')[1]);
        }
      }//inner for
    }
  }//outer for
  return C::INVALID;
}

bool RtspHandler::isSeekRequest(const std::vector<std::string>& strings) {
  for (const std::string& elem : strings) {
    if ( elem.find("SeekInfo") != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool RtspHandler::isValidPlayTime(const std::vector<float>& ntpSec) {
  if (auto acsHandler = acsHandlerPtr.lock()) {
    std::vector<int64_t> playTimeUs = acsHandler->getPlayTimeUs();
    int mediaPlayTimeMs = static_cast<int>(std::max(playTimeUs[0], playTimeUs[1])/1000);

    return (static_cast<int>(ntpSec[0]*1000) < mediaPlayTimeMs) &&
      (ntpSec[1] == -1  || ntpSec[1]*1000 < mediaPlayTimeMs);
  } else {
    return false;
  }
}

std::string RtspHandler::getContentsTitle(const std::vector<std::string>& urls) {
  if (urls.empty()) {
    return C::EMPTY_STRING;
  }
  std::vector<std::string> infos = Util::splitToVecBySingleChar(urls[0], '/');
  for (auto i = infos.size() - 1; i >= 0; i--) {
    std::string elem = infos[i];
    if (elem != "trackID=0") {
      return elem;
    }
  }
  return C::EMPTY_STRING;
}

std::string RtspHandler::getSupportingBitrateTypes(std::vector<int> bitrateTypes) {
  if (auto sessionPtr = parentSessionPtr.lock()) {
    if (bitrateTypes.empty()) {
      return std::to_string(sessionPtr->get_mbpsCurBitrate());
    } else {
      std::string val = C::EMPTY_STRING;
      for (auto i = 0; i < bitrateTypes.size(); i++) {
        val += std::to_string(bitrateTypes[i]);
        if (i < bitrateTypes.size() - 1) {
          val += ",";
        }
      }
      return val;
    }
  } else {
    logger->severe("RtspHandler: parentSessionPtr is null");
    throw std::runtime_error("RtspHandler: parentSessionPtr is null");
  }
}

bool RtspHandler::isContainingPlayInfoHeader(const std::vector<std::string>& strings) {
  for (const std::string& elem : strings) {
    if ( elem.find("PlayInfo") != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool RtspHandler::isThereMonitoringInfoHeader(const std::vector<std::string>& strings) {
  for (const std::string& elem : strings) {
    if ( elem.find("MonitoringInfo") != std::string::npos) {
      return true;
    }
  }
  return false;
}

void RtspHandler::parseHybridVideoSampleMetaDataForDandS(const std::string& notTxIdListStr) {
  // dongvin : for hybrid streaming
/*
hybridMetaMap inside : camId >> view number & frame type >> sampleNo & sampleMetaData
Map : hybridMeta<camId, val>
                         |---> Map<view number + frame type string , val>
                                                                       |---> Map<sampleNo, val>
                                                                                            |---> sample meta data
*/
  using HybridMetaMapType
    = std::unordered_map<int, std::unordered_map<std::string, std::unordered_map<int, HybridSampleMeta>>>;
  if (auto sessionPtr = parentSessionPtr.lock()) {
    if (auto handlerPtr = acsHandlerPtr.lock()) {
      HybridMetaMapType& hybridMetaMap = sessionPtr->getHybridMetaMap();
      int gop = handlerPtr->getGop()[0];

      std::vector<std::string> infoArr = Util::splitToVecBySingleChar(notTxIdListStr, ',');
      int camId = std::stoi(infoArr[0]);
      int viewNum = std::stoi(infoArr[1]);
      for (auto i=2; i<infoArr.size(); i++) {
        int sampleIdx = std::stoi(infoArr[i]);
        std::string frameType = sampleIdx % gop == 0 ? C::KEY_FRAME_TYPE : C::P_FRAME_TYPE;

        if (hybridMetaMap.find(camId) == hybridMetaMap.end()) {
          // put empty map if key 'camId' was not found.
          hybridMetaMap[camId] = {};
        }

        auto& viewNumberAndFrameTypeMap = hybridMetaMap[camId];
        std::string viewNumberFrameKey = std::to_string(viewNum) + frameType;
        if (viewNumberAndFrameTypeMap.find(viewNumberFrameKey) == viewNumberAndFrameTypeMap.end()) {
          viewNumberAndFrameTypeMap[viewNumberFrameKey] = {};
        }

        auto& sampleMetaMap = viewNumberAndFrameTypeMap[viewNumberFrameKey];

        const HybridSampleMeta hybridSampleMeta(
          sampleIdx, C::INVALID_OFFSET, C::INVALID_OFFSET, C::INVALID_OFFSET
        );

        if (sampleMetaMap.find(sampleIdx) == sampleMetaMap.end()) {
          sampleMetaMap[sampleIdx] = hybridSampleMeta;
        }

      }//for
    } else logger->severe("Dongvin, RtspHandler: failed to get weak AcsHandlerPtr!");
  } else logger->severe("Dongvin, RtspHandler: failed to get weak SessionPtr!");
}