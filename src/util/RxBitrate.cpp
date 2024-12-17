#include "../include/RxBitrate.h"

RxBitrate::RxBitrate(long bitrate, long utcTimeMillis)
    : bitrate(bitrate), utcTimeMillis(utcTimeMillis) {}

long RxBitrate::getBitrate() const {
    return bitrate;
}

long RxBitrate::getUtcTimeMillis() const {
    return utcTimeMillis;
}
