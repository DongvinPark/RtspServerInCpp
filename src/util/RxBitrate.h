#ifndef RXBITRATE_H
#define RXBITRATE_H

class RxBitrate {
public:
    explicit RxBitrate(long bitrate, long utcTimeMillis);

    long getBitrate() const;
    long getUtcTimeMillis() const;

private:
    const long bitrate;
    const long utcTimeMillis;
};

#endif // RXBITRATE_H
