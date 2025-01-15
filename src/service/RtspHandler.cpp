#include "../include/RtspHandler.h"

#include "../../constants/Util.h"
#include "../../include/Session.h"

#include <cmath>
#include <iostream>

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
  for (std::string elem : strings) {
    if (elem.find("User-Agent") != std::string::npos) {
      return Util::trim( Util::splitToVecBySingleChar(elem, ':')[1] );
    }
  }
  return C::EMPTY_STRING;
}

int RtspHandler::findCSeq(std::vector<std::string> strings) {
  for (std::string elem : strings) {
    if (elem.find("CSeq") != std::string::npos) {
      return std::stoi(
        Util::trim( Util::splitToVecBySingleChar(elem, ':')[1] )
      );
    }
  }
  return C::INVALID;
}

std::string RtspHandler::findContents(std::string line0) {
  std::string address = Util::splitToVecBySingleChar(line0, ' ')[1];
  std::vector<std::string> words = Util::splitToVecBySingleChar(address, ':');
  if (words.size() != C::URL_SPLIT_BY_SEMI_COLON_LENGTH) {
    logger->severe("Invalid Title!");
    return C::EMPTY_STRING;
  }
  return Util::trim(
    Util::splitToVecBySingleChar(words[2], '/')[1]
  );
}

std::string RtspHandler::getMediaInfo(std::string fullCid) {
  if (auto handlerPtr = acsHandlerPtr.lock()) {

    std::vector<int64_t> unitCnt = handlerPtr->getUnitFrameCount();
    std::vector<int64_t> gop = handlerPtr->getGop();

    std::string mediaInfo = handlerPtr->getMediaInfo();
    std::vector<std::string> lines = Util::splitToVecByString(mediaInfo, C::CRLF);

    for (int i = 0; i < lines.size(); ++i) {
      std::string line = lines[i];
      if (line.starts_with("c=")) {
        lines[i] = "c=IN IP4 0.0.0.0"; // don't need to know ip addr of client
      } else if (line.starts_with("a=control:")){
        int trackId = std::stoi(Util::splitToVecBySingleChar(line, '=')[2]);

        std::string track = "/trackID="+trackId;
        lines[i] = "a=control:"+C::DUMMY_CONTENT_BASE+track;
        if(trackId <= C::AUDIO_ID){
          handlerPtr->setStreamUrl(trackId, fullCid+track);
        }
      } else if (line.starts_with("AS")){ // application specific for bandwidth.
        lines[i] = C::EMPTY_STRING;
      } else if (line.starts_with("a=tool:")) {
        lines[i] = C::EMPTY_STRING;
      } else if (line.starts_with("a=fmtp:")) {
        if(Util::trim( Util::splitToVecBySingleChar(line, ':')[1] ).starts_with("97")){ // audio
          // refer to Table 9 (streamType Values) in ISO/IEC 14496-1 (coding of audio-visual objects)
          lines[i] +=";streamType=5";
          lines[i] += ";ucnt="+unitCnt[1];
        } else if (Util::trim( Util::splitToVecBySingleChar(line, ':')[1] ).starts_with("96")){ // video
          // refer to Table 9 (streamType Values) in ISO/IEC 14496-1 (coding of audio-visual objects)
          lines[i] +=";streamType=4";
          lines[i] += ";dGop="+gop[0];
          lines[i] += ";ucnt="+unitCnt[0];
        }
      } else if (line.starts_with("b=AS:")) {
        // dongvin : 최초 재생 시 사용하게 될 bitrate를 기록해 둔다.
        int kbps = std::stoi(Util::splitToVecBySingleChar(line, ':')[1] );
        if (auto sessionPtr = parentSessionPtr.lock()) {
          sessionPtr->add_kbpsBitrateValue(kbps);
        }
      }
    }//for

    std::string result = "";
    for (std::string line : lines) {
      if (line == C::EMPTY_STRING) {
        continue;
      }
      result += line + C::CRLF;
    }

    return result;
  } else {
    return C::EMPTY_STRING;
  }
}

int RtspHandler::findTrackId(std::string line0) {
  std::string addr = Util::splitToVecBySingleChar(line0, ' ')[1];
  return std::stoi( Util::splitToVecBySingleChar(addr, '=')[1] );
}

std::string RtspHandler::findTransport(std::vector<std::string> strings) {
  // required
  // refer to https://www.rfc-editor.org/rfc/rfc2326.html#section-12.39
  for (std::string elem : strings) {
    if ( elem.find("Transport:") != std::string::npos) {
      return Util::splitToVecBySingleChar(elem, ' ')[1];
    }
  }
  return C::EMPTY_STRING;
}

std::string RtspHandler::findHybridMode(std::vector<std::string> strings) {
  for (std::string elem : strings) {
    if ( elem.find("HybridMode") != std::string::npos) {
      std::string mode = Util::splitToVecBySingleChar(elem, ' ')[1];
      if (C::HYBRID_MODE_SET.contains(mode)) {
        return mode;
      } else {
        return C::EMPTY_STRING;
      }
    }
  }
  return C::EMPTY_STRING;
}

std::string RtspHandler::findNotTx(std::vector<std::string> strings) {
  for (std::string elem : strings) {
    if ( elem.find("NotTx") != std::string::npos) {
      return Util::splitToVecBySingleChar(elem, ' ')[1];
    }
  }
  return C::EMPTY_STRING;
}

std::vector<int> RtspHandler::findChannels(std::string transport) {
  std::string range = Util::splitToVecBySingleChar(transport, '=')[1];
  std::vector<std::string> channels = Util::splitToVecBySingleChar(range, '-');
  std::vector<int> channelVec;
  channelVec.push_back(std::stoi(channels[0]));
  channelVec.push_back(std::stoi(channels[1]));
  return channelVec;
}

std::string RtspHandler::findSessionId(std::vector<std::string> strings) {
  for (std::string elem : strings) {
    if (elem.find("Session:") != std::string::npos) {
      return Util::trim( Util::splitToVecBySingleChar(elem, ':')[1] );
    }
  }
  return C::EMPTY_STRING;
}

std::vector<float> RtspHandler::findNormalPlayTime(std::vector<std::string> strings) {
  // optional.
  // refer to https://www.rfc-editor.org/rfc/rfc2326.html#section-3.6 chapter
  for (std::string line : strings) {
    if (line.find("npt") != std::string::npos) {
      const std::vector<std::string> rangePart = Util::splitToVecBySingleChar(line, '=');
      const std::vector<std::string> range = Util::splitToVecBySingleChar(rangePart[1], '-');
      std::vector<float> nptVector;
      nptVector.push_back(std::stof(range[0]));
      nptVector.push_back(range.size() == 2 ? std::stof(range[1]) : -1);
      return nptVector;
    }
  }
  return std::vector<float>();
}

std::string RtspHandler::findDeviceModelName(std::vector<std::string> strings) {
  for (std::string elem : strings) {
    if ( elem.find("ModelNo") != std::string::npos) {
      return Util::splitToVecBySingleChar(elem, ' ')[1];
    }
  }
  return C::EMPTY_STRING;
}

std::string RtspHandler::findManufacturer(std::vector<std::string> strings) {
  for (std::string elem : strings) {
    if ( elem.find("Manufacturer") != std::string::npos) {
      return Util::splitToVecBySingleChar(elem, ' ')[1];
    }
  }
  return C::EMPTY_STRING;
}

// for play req right after pause
int RtspHandler::findLatestReceivedSampleIdx(
  std::vector<std::string> strings,std::string filter
) {
  // PLAY req header example
  // PlayInfo: videoIndex=144;videoRtpIndex=8;audioIndex=228;
  for (std::string headerLine: strings) {
    if (headerLine.starts_with("PlayInfo")) {
      std::string HeaderValue = Util::splitToVecByString(headerLine, " ")[1];
      for (std::string value: Util::splitToVecBySingleChar(headerLine, ';')) {
        if (value.find(filter) != std::string::npos) {
          return std::stoi(Util::splitToVecBySingleChar(value, '=')[1]);
        }
      }//inner for
    }
  }//outer for
  return C::INVALID;
}

bool RtspHandler::isSeekRequest(std::vector<std::string> strings) {
  for (std::string elem : strings) {
    if ( elem.find("SeekInfo") != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool RtspHandler::isValidPlayTime(std::vector<float> ntpSec) {
  if (auto acsHandler = acsHandlerPtr.lock()) {
    std::vector<int64_t> playTimeUs = acsHandler->getPlayTimeUs();
    int mediaPlayTimeMs = static_cast<int>(std::max(playTimeUs[0], playTimeUs[1])/1000);

    return (static_cast<int>(ntpSec[0]*1000) < mediaPlayTimeMs) &&
      (ntpSec[1] == -1  || ntpSec[1]*1000 < mediaPlayTimeMs);
  } else {
    return false;
  }
}

std::string RtspHandler::getContentsTitle(std::vector<std::string> urls) {
  if (urls.empty()) {
    return C::EMPTY_STRING;
  }
  std::vector<std::string> infos = Util::splitToVecBySingleChar(urls[0], '/');
  for (int i = infos.size() - 1; i >= 0; i--) {
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
      for (int i = 0; i < bitrateTypes.size(); i++) {
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

bool RtspHandler::isContainingPlayInfoHeader(std::vector<std::string> strings) {
  for (std::string elem : strings) {
    if ( elem.find("PlayInfo") != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool RtspHandler::isThereMonitoringInfoHeader(std::vector<std::string> strings) {
  for (std::string elem : strings) {
    if ( elem.find("MonitoringInfo") != std::string::npos) {
      return true;
    }
  }
  return false;
}

void RtspHandler::parseHybridVideoSampleMetaDataForDandS(std::string notTxIdListStr) {
}