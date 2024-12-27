#include <boost/asio.hpp>

#include "../include/Logger.h"
#include "../constants/C.h"
#include "../include/SntpRefTimeProvider.h"
#include "../include/FileReader.h"
#include "../include/Session.h"
#include "../include/Server.h"

// Version History
// VER          Date            Changes
// 1.0.0        2024.12.27      Util, DTO, and FileRaader Initialized

using boost::asio::ip::tcp;

int main() {
    std::shared_ptr<Logger> logger = Logger::getLogger(C::MAIN);
    logger->warning("=================================================================");
    logger->warning("Dongvin, C++ AlphaStreamer3.1 RTSP Server STARTS. ver: " + C::VER);
    logger->warning("=================================================================");

    // make worker thread pool for boost.asio io_context
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

    //std::string contentsRootPath = "/mnt/c/dev/streaming-contents"; // for WSL
    std::string contentsRootPath = "C:\\dev\\streaming-contents"; // for native Windows

    ContentsStorage contentsStorage(contentsRootPath);
    contentsStorage.init();

    Server server(io_context, contentsStorage, contentsRootPath, sntpRefTimeProvider);
    server.start();

    return 0;
}
/*
*** for prev test ***

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
        std::cout << "Ward 'Hello' Transmission complete!" << "\n";
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

>>

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

std::mutex lock;

    auto myTask = [&]() {
        // osyncstream not works on MacOS. Used std::mutex.
        std::lock_guard<std::mutex> guard(lock);
         std::osyncstream (std::cout) << "sntp ref time millis : "
            //<< sntpRefTimeProvider.getRefTimeMillisForCurrentTask() << "\n";
    //}

std::cout << "AudioSampleInfo copy test!\n";

    AudioSampleInfo aSampleInfo1(10, 0);
    AudioSampleInfo aSampleInfo2 = aSampleInfo1;

    std::cout << "aSampleInfo1 : "  << aSampleInfo1.len << "/" << aSampleInfo1.offset << "\n";
    std::cout << "aSampleInfo2 : "  << aSampleInfo2.len << "/" << aSampleInfo2.offset << "\n";

    std::cout << "VideoSampleInfo copy test!\n";

    VideoSampleInfo vInfo1;
    vInfo1.setSize(2);
    vInfo1.setOffset(0);
    vInfo1.setFlag(C::KEY_FRAME_FLAG);

    VideoSampleInfo vInfo2 = vInfo1;
    vInfo2.setFlag(C::P_FRAME_FLAG);

    std::cout << "vInfo1 : " << vInfo1.getSize() << vInfo1.getOffset() << vInfo1.getFlag() << "\n";
    std::cout << "vInfo2 : " << vInfo2.getSize() << vInfo2.getOffset() << vInfo2.getFlag() << "\n";
*/