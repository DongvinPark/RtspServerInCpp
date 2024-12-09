#include "SntpRefTimeProvider.h"
#include "../src/util/Logger.h"
#include "../src/util/C.h"

/*
	Logger logger;

	long originSntpTime;
	std::atomic<long> ntpTimeMs;
	std::atomic<long> elapsedTimeNanoSec;

	std::mutex lock;
	std::atomic<bool> running;
	std::thread timerThread;
*/

SntpRefTimeProvider::SntpRefTimeProvider(boost::asio::io_context& io_context)
	: 
	iOContext(io_context),
	logger(Logger::getLogger("SntpRefTimeProvider")),
	originSntpTime(0),
	ntpTimeMs(C::UNSET),
	elapsedTimeNanoSec(C::UNSET),
	running(false) {}

SntpRefTimeProvider::~SntpRefTimeProvider() {
	running = false;
	// join the timer thread.
	if (timerThread.joinable()) {
		timerThread.join();
	}
}

void SntpRefTimeProvider::start() {
}

long SntpRefTimeProvider::getRefTimeMillisForCurrentTask() {
	return 0;
}

long SntpRefTimeProvider::getRefTimeSecForCurrentTask() {
	return 0;
}

long SntpRefTimeProvider::getRefTime(long now) {
	return 0;
}

void SntpRefTimeProvider::readTime() {
}

void SntpRefTimeProvider::onReadSntpTime(long ntpTimeMs_) {
}

void SntpRefTimeProvider::read32(const std::vector<uint8_t>& buffer, size_t offset) {
}

long SntpRefTimeProvider::readTimeStamp(const std::vector<uint8_t>& buffer, size_t offset) {
	return 0;
}

void SntpRefTimeProvider::writeTimeStamp(std::vector<uint8_t>& buffer, size_t offset, long time) {
}
