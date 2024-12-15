#include "SntpRefTimeProvider.h"
#include "../src/util/C.h"

#include <iostream>
#include <chrono>
#include <cstdlib>
#include <random>

SntpRefTimeProvider::SntpRefTimeProvider(boost::asio::io_context& inputIoContext)
    : logger(Logger::getLogger("SntpRefTimeProvider")),
    ioContext(inputIoContext),
    originSntpTime(0),
    ntpTimeMs(C::UNSET),
    elapsedTimeNs(C::UNSET),
    running(false) {}

SntpRefTimeProvider::~SntpRefTimeProvider() {
    running = false;
    if (timerThread.joinable()) {
        timerThread.join();
    }
}

void SntpRefTimeProvider::start() {
    running = true;
    timerThread = std::thread([this]() {
        while (running) {
            readTime();
            std::this_thread::sleep_for(std::chrono::milliseconds(C::NTP_READ_PERIOD));
        }
        });
}

long long SntpRefTimeProvider::getRefTimeMillisForCurrentTask() {
    return getRefTime(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );
}

long long SntpRefTimeProvider::getRefTimeSecForCurrentTask() {
    return getRefTimeMillisForCurrentTask() / 1000;
}

long long SntpRefTimeProvider::getRefTime(long long now) {
    std::lock_guard<std::mutex> guard(lock);
    long long _ntpTimeMs = ntpTimeMs.load();
    return (_ntpTimeMs == -1L) ? -1L
        : _ntpTimeMs + (now - elapsedTimeNs.load()) / 1000000;
}

void SntpRefTimeProvider::readTime() {
    try {
        boost::asio::ip::udp::resolver resolver(ioContext);
        boost::asio::ip::udp::endpoint serverEndpoint =
            *resolver.resolve(
                boost::asio::ip::udp::v4(), C::SNTP_SERVER_HOST, std::to_string(C::NTP_PORT)
            ).begin();
        boost::asio::ip::udp::socket socket(ioContext, boost::asio::ip::udp::v4());

        std::vector<uint8_t> buffer(C::NTP_PACKET_SIZE, 0);

        // set mode = 3 (client) and version = 3
        // mode is in low 3 bits of first byte
        // version is in bits 3-5 of first byte
        buffer[0] = C::NTP_MODE_CLIENT | (C::NTP_VERSION << 3);

        // get current time and write it to the request packet
        long requestTime = // UTC time 1970.1.1, ms, local UTC time
            std::chrono::system_clock::now().time_since_epoch().count();
        long requestTicks = //nano sec. device time from boot
            std::chrono::high_resolution_clock::now().time_since_epoch().count();

        writeTimeStamp(buffer, C::TRANSMIT_TIME_OFFSET, requestTime);

        socket.send_to(boost::asio::buffer(buffer), serverEndpoint);

        boost::asio::ip::udp::endpoint responseEndpoint;
        std::size_t bytesReceived = 0;

        // read the response
        bytesReceived = socket.receive_from(
            boost::asio::buffer(buffer),
            responseEndpoint);

        if (bytesReceived == 0) {
            logger->info("No data received : read again");
            return;
        }

        long responseTicks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        long responseTime = requestTime + (responseTicks - requestTicks) / 1000000;

        /*
        cancel comment to see the buffer
        std::cout << "!!! buffer inside : \n";
        for (auto& b : buffer) {
            std::cout << b << ",";
        }
        std::cout << "\n";
        */

        // extract the results
        // tasik, client send time = request time
        long long originateTime = readTimeStamp(buffer, C::ORIGINATE_TIME_OFFSET);
        // the time server receives the request.
        long long receiveTime = readTimeStamp(buffer, C::RECEIVE_TIME_OFFSET);
        // the time server sends the response.
        long long transmitTime = readTimeStamp(buffer, C::TRANSMIT_TIME_OFFSET);

        // round trip time in 2 ways.round trip time : the time consumed in media.
        // round trip time = total processing in client time  - server processing time.
        // Theoretically, we can't calculate the round trip time by
        // (receiveTime - originateTime) - (transmitTime - responseTime) because of the clock
        // skew between server and client devices. So we get the trip time only by 2 time difference
        // from each device, removing the common offset and short-time drift.
        long long serverProcessTime = transmitTime - receiveTime; // measured in server.
        long long totalProcessTime = responseTime - requestTime; // measured in client
        long long roundTripTime = totalProcessTime - serverProcessTime;

        // receiveTime = originateTime + transit + skew
        // responseTime = transmitTime + transit - skew
        // clockOffset = ((receiveTime - originateTime) + (transmitTime - responseTime))/2
        //             = ((originateTime + transit + skew - originateTime) +
        //                (transmitTime - (transmitTime + transit - skew)))/2
        //             = ((transit + skew) + (transmitTime - transmitTime - transit + skew))/2
        //             = (transit + skew - transit + skew)/2
        //             = (2 * skew)/2 = skew
        long long clockOffset = ((receiveTime - originateTime) + (transmitTime - responseTime)) / 2;

        // save our results - use the times on this side of the network latency
        // (response rather than request time)
        originSntpTime = responseTime + clockOffset; // UTC Time tuned by clock skew btw server and me.

        // we pass the time immediately to listener so as not to include the additional delay
        // by the internal sw works.
        onReadSntpTime(originSntpTime);

        socket.close();
    } catch (const std::exception& e) {
        std::cerr << "Error reading time: " << e.what() << std::endl;
    }
}


void SntpRefTimeProvider::writeTimeStamp(std::vector<uint8_t>& buffer, size_t offset, long long time) {
    long seconds = time / 1000LL + C::OFFSET_1900_TO_1970;
    long milliseconds = time % 1000LL;

    buffer[offset++] = (seconds >> 24) & 0xFF;
    buffer[offset++] = (seconds >> 16) & 0xFF;
    buffer[offset++] = (seconds >> 8) & 0xFF;
    buffer[offset++] = seconds & 0xFF;

    long fraction = (milliseconds * 0x100000000LL) / 1000LL;
    buffer[offset++] = (fraction >> 24) & 0xFF;
    buffer[offset++] = (fraction >> 16) & 0xFF;
    buffer[offset++] = (fraction >> 8) & 0xFF;

    std::random_device rd; // Seed
    std::mt19937 randomEngine(rd()); // Random engine (Mersenne Twister)
    // low order bits should be random data
    buffer[offset++] = static_cast<uint8_t>(randomEngine() % 256);
}

long long SntpRefTimeProvider::readTimeStamp(
    const std::vector<uint8_t>& buffer, size_t offset
) {
    long long seconds = read32(buffer, offset);
    long long fraction = read32(buffer, offset + 4);
    return ((static_cast<long long>(seconds) - C::OFFSET_1900_TO_1970) * 1000)
        + ((fraction * 1000LL) / 0x100000000LL);
}

long long SntpRefTimeProvider::read32(
    const std::vector<uint8_t>& buffer, size_t offset
) {
    long byte1 = static_cast<long>(buffer[offset]) << 24;
    long byte2 = static_cast<long>(buffer[offset + 1]) << 16;
    long byte3 = static_cast<long>(buffer[offset + 2]) << 8;
    long byte4 = static_cast<long>(buffer[offset + 3]);

    return byte1 | byte2 | byte3 | byte4;
}


void SntpRefTimeProvider::onReadSntpTime(long long ntpTimeMs_) {
    long long curRefTime = getRefTime(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );

    long long diff = 0LL;
    // the noise on clock can make this abrupt change, actually AWGN.
    // so we don't follow the update but watch this difference down to keep UTC synced nicely.

    // we don't update this spiky update.
    // the device abrupt noise makes a bad effect on it or the context thread
    // will make the time difference by running other tasks.
    if (curRefTime != C::UNSET) {
        diff = ntpTimeMs_ - curRefTime;

        if (std::abs(diff) > 35) {
            return;
        }

        // filter the diff and update
        ntpTimeMs_ = static_cast<long long>(ntpTimeMs_ - (0.5 * diff));
    }

    std::lock_guard<std::mutex> guard(lock);
    ntpTimeMs.store(ntpTimeMs_);
    elapsedTimeNs.store(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );
}
