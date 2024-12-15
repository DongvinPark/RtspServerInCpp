#ifndef SNTP_REF_TIME_PROVIDER_H
#define SNTP_REF_TIME_PROVIDER_H

#include <boost/asio.hpp>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>

#include "../src/util/Logger.h"

class SntpRefTimeProvider {
public:
    explicit SntpRefTimeProvider(boost::asio::io_context& inputIoContext);
    ~SntpRefTimeProvider();

    void start();
    long long getRefTimeMillisForCurrentTask();
    long long getRefTimeSecForCurrentTask();
    long long getRefTime(long long now);

private:
    void readTime();
    void writeTimeStamp(std::vector<uint8_t>& buffer, size_t offset, long long time);
    long long readTimeStamp(const std::vector<uint8_t>& buffer, size_t offset);
    long long read32(const std::vector<uint8_t>& buffer, size_t offset);
    void onReadSntpTime(long long ntpTimeMs);

    std::shared_ptr<Logger> logger;
    boost::asio::io_context& ioContext;
    long long originSntpTime; // Tracks the synchronized SNTP time
    std::atomic<long long> ntpTimeMs;
    std::atomic<long long> elapsedTimeNs;

    std::mutex lock;
    std::atomic<bool> running;
    std::thread timerThread;
};

#endif // SNTP_REF_TIME_PROVIDER_H