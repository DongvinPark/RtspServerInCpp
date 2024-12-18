#ifndef RXBITRATE_H
#define RXBITRATE_H

#include <cstdint> // for int64_t

class RxBitrate {
public:
    explicit RxBitrate(int64_t bitrate, int64_t utcTimeMillis);

    int64_t getBitrate() const;
    int64_t getUtcTimeMillis() const;

private:
    const int64_t bitrate;
    const int64_t utcTimeMillis;
};

#endif // RXBITRATE_H
