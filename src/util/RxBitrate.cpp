#include "../include/RxBitrate.h"

RxBitrate::RxBitrate(int64_t bitrate, int64_t utcTimeMillis)
    : bitrate(bitrate), utcTimeMillis(utcTimeMillis) {}

int64_t RxBitrate::getBitrate() const {
    return bitrate;
}

int64_t RxBitrate::getUtcTimeMillis() const {
    return utcTimeMillis;
}
