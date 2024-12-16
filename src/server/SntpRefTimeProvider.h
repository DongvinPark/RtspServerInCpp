#ifndef SNTP_REF_TIME_PROVIDER_H
#define SNTP_REF_TIME_PROVIDER_H

#include <boost/asio.hpp>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <cstdint> // For int64_t

#include "../src/util/Logger.h"

class SntpRefTimeProvider {
public:
    explicit SntpRefTimeProvider(boost::asio::io_context& inputIoContext);
    ~SntpRefTimeProvider();

    void start();
    int64_t getRefTimeMillisForCurrentTask();
    int64_t getRefTimeSecForCurrentTask();
    int64_t getRefTime(int64_t now);

private:
    void readTime();
    void writeTimeStamp(std::vector<uint8_t>& buffer, size_t offset, int64_t time);
    int64_t readTimeStamp(const std::vector<uint8_t>& buffer, size_t offset);
    int64_t read32(const std::vector<uint8_t>& buffer, size_t offset);
    void onReadSntpTime(int64_t ntpTimeMs);

    std::shared_ptr<Logger> logger;
    boost::asio::io_context& ioContext;
    int64_t originSntpTime; // Tracks the synchronized SNTP time
    std::atomic<int64_t> ntpTimeMs;
    std::atomic<int64_t> elapsedTimeNs;

    std::mutex lock;
    std::atomic<bool> running;
    std::thread timerThread;
};

#endif // SNTP_REF_TIME_PROVIDER_H