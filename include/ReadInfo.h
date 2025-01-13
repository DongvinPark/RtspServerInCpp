#ifndef READINFO_H
#define READINFO_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "../include/AudioSampleInfo.h"
#include "../include/VideoAccess.h"
#include "../constants/C.h"

class ReadInfo {
public:
  void setCurSampleNo(int idx);
  bool isDone();
  std::string toString();

  int startSampleNo = C::INVALID;
  int endSampleNo = C::INVALID;
  int curSampleNo = C::INVALID;

  int refSeq0 = C::INVALID;
  std::vector<int> channel{};
  int64_t unitTimeUs = C::INVALID_OFFSET;
  int maxSampleNo = C::INVALID;

  int64_t timestamp = C::INVALID_OFFSET;
  int unitFrameCount = C::INVALID;

  int curVid = C::INVALID;
  int curMemberVid = C::INVALID;

  int64_t curPresentationTimeUs = C::INVALID_OFFSET;

  // TODO : AudioSampleInfo and VideoAccess are non-copyable
  // TODO : Should refactor these fields later.
  //std::vector<AudioSampleInfo> audioMeta{};
  //std::unordered_map<std::string, VideoAccess> videoMeta{};
};

#endif //READINFO_H
