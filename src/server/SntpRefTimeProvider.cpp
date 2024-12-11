#include "SntpRefTimeProvider.h"
#include "../src/util/Logger.h"

SntpRefTimeProvider::SntpRefTimeProvider(boost::asio::io_context& io_context)
	: 
	iOContext(io_context),
	logger(Logger::getLogger("SntpRefTimeProvider")),
	originSntpTime(0),
	ntpTimeMs(C::UNSET),
	elapsedTimeNanoSec(C::UNSET),
	running(false),
	timerTask(PeriodicTask(io_context, std::chrono::milliseconds(C::NTP_READ_PERIOD))){}

SntpRefTimeProvider::~SntpRefTimeProvider() {
	running = false;
	timerTask.stop();
}

void SntpRefTimeProvider::start() {
	// set task to timer and start the timer.
	auto readTimeTask = [this]() {
		this->readTime();
	};
	timerTask.setTask(readTimeTask);
	timerTask.start();
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
