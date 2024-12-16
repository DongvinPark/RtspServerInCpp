#include <boost/asio.hpp>
#include <iostream>
#include <syncstream>

#include "util/Logger.h"
#include "util/C.h"
#include "util/Buffer.h"
#include "server/file/AudioSample.h"
#include "util/Util.h"
#include "util/AVSampleBuffer.h"
#include "timer/PeriodicTask.h"
#include "server/SntpRefTimeProvider.h"

using boost::asio::ip::tcp;

boost::asio::io_context& returnIOContext(
    boost::asio::io_context& ioContext
) {
    return ioContext;
}

int main() {
    
    auto logger = Logger::getLogger(C::MAIN);
    logger->severe("This is a severe message.");
    logger->warning("This is a warning message.");
    logger->info("This is an info message.");
    logger->info2("This is an info2 message.");
    logger->info3("This is an info3 message.");
    logger->debug("This is a debug message.");
    logger->setUserInputPrompt(false);
    logger->info("Input prompt disabled.");

    std::cout << "Use of global constants. Version : " << C::VERSION << "\n";
    std::cout << "Use of global constants. Hybrid Mode types : ";
    for(std::string mode : C::HYBRID_MODE_SET){
        std::cout << mode << ", ";
    }
    std::cout << "\n";

    std::vector<unsigned char> data = {'H', 'e', 'l', 'l', 'o'};
    Buffer buffer(data);

    buffer.afterTx = []() {
        std::cout << "Ward 'Hello' Transmission complete!" << std::endl;
    };

    std::cout << (unsigned char)0x24 << "\n";

    if (buffer.afterTx) {
        buffer.afterTx();
    }

    std::cout << "buffer.toString() result : " << buffer.toString() << "\n";

    std::cout << "AudioSample Test Start!!\n";
    std::vector<unsigned char> audioData = {'A', 'u', 'd'};
    AudioSample aSample(audioData, audioData.size());
    std::cout << "AudioSample toString : " << aSample.toString() << "\n";

    std::cout << "Get random key for rtsp session Id \n";
    std::cout << Util::getRandomKey(64) << "\n";

    std::cout << "AVSampleBuffer test start!!\n";
    AVSampleBuffer avSample(C::VIDEO_ID);
    avSample.setKill();
    std::cout << avSample.toString();

    std::cout << "\nPeriodic Timer Task start!\n";

    boost::asio::io_context io_context;
    auto workGuard = boost::asio::make_work_guard(io_context);
    std::vector<std::thread> threadVec;
    unsigned int threadCnt = std::thread::hardware_concurrency();
    for (int i = 0; i < threadCnt; ++i) {
        threadVec.emplace_back(
            [&io_context]() {io_context.run(); }
        );
    }

    SntpRefTimeProvider sntpRefTimeProvider(io_context);
    sntpRefTimeProvider.start();

    std::mutex lock;

    auto myTask = [&]() {
        // osyncstream not works on MacOS. Used std::mutex.
        std::lock_guard<std::mutex> guard(lock);
        /* std::osyncstream */(std::cout) << "sntp ref time millis : "
            << sntpRefTimeProvider.getRefTimeMillisForCurrentTask() << "\n";
    };

    std::chrono::milliseconds interval(1000); // 1 second
    PeriodicTask task(returnIOContext(io_context), interval, myTask);
    task.start();

    //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    task.stop();

    PeriodicTask noTask(io_context, std::chrono::milliseconds(1000));
    noTask.setTask(myTask);
    noTask.start();

    //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    noTask.stop();

    workGuard.reset();
    
    for (auto& thread : threadVec) {
        thread.join();
    }

    return 0;
}
