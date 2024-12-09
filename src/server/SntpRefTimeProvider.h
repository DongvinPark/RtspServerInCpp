#ifndef SNTP_REF_TIME_PROVIDER_H
#define SNTP_REF_TIME_PROVIDER_H

#include <boost/asio.hpp>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include "../src/util/C.h"
#include "../src/util/Logger.h"

class SntpRefTimeProvider {
public:
	SntpRefTimeProvider(boost::asio::io_context& io_context);
	~SntpRefTimeProvider();

	void start();
	long getRefTimeMillisForCurrentTask();
	long getRefTimeSecForCurrentTask();
	long getRefTime(long now);

private:
	void readTime();
	void onReadSntpTime(long ntpTimeMs_);
	void read32(const std::vector<uint8_t>& buffer, size_t offset);
	long readTimeStamp(const std::vector<uint8_t>& buffer, size_t offset);
	void writeTimeStamp(std::vector<uint8_t>& buffer, size_t offset, long time);

	boost::asio::io_context& iOContext;

	std::shared_ptr<Logger> logger;

	long originSntpTime;
	std::atomic<long> ntpTimeMs;
	std::atomic<long> elapsedTimeNanoSec;

	std::mutex lock;
	std::atomic<bool> running;
	std::thread timerThread;

};

#endif //SNTP_REF_TIME_PROVIDER_H