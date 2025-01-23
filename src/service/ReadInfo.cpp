#include "../include/ReadInfo.h"

void ReadInfo::setCurSampleNo(int idx) {
    curSampleNo = idx;
}

bool ReadInfo::isDone() {
    return curSampleNo > endSampleNo;
}

std::string ReadInfo::toString() {
    return "start sample no: "+std::to_string(startSampleNo)+
            ", end sample no: "+std::to_string(endSampleNo)+
            ", current sample no: "+std::to_string(curSampleNo)+
            ", max sample no: "+std::to_string(maxSampleNo)+
            ", current vid: "+std::to_string(curVid)+
            ", current member vid: "+std::to_string(curMemberVid);
}
