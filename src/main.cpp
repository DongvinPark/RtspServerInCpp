#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <csignal> // for signal event handling
#include <iostream>
#include <vector>
#include <thread>
#include <future> // for std::promise and std::future
#include <filesystem>

// to find absolute path of project root dir
#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <libproc.h>
    #include <unistd.h>
#elif __linux__
    #include <unistd.h>
#endif

#include "../include/Logger.h"
#include "../constants/C.h"
#include "../include/SntpRefTimeProvider.h"
#include "../include/ContentFileMeta.h"
#include "../include/Session.h"
#include "../include/Server.h"

// Version History
// VER          Date            Changes
// 1.0.0        2024.12.27      Util, DTO, and FileRaader Initialized

using boost::asio::ip::tcp;

std::string getExecutablePath() {
    char path[1024];

#ifdef _WIN32
    GetModuleFileNameA(NULL, path, sizeof(path));
#elif __APPLE__
    proc_pidpath(getpid(), path, sizeof(path));
#elif __linux__
    ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
    if (count != -1) path[count] = '\0'; // Null-terminate
#endif

    return std::string(path);
}

// Function to find the project root directory
std::string getProjectRoot() {
    std::filesystem::path exePath = getExecutablePath();

    // Traverse up until we find the "RtspServer" directory
    while (exePath.has_parent_path()) {
        exePath = exePath.parent_path();
        if (exePath.filename() == "RtspServerInCpp") {
            return exePath.string();
        }
    }

    return ""; // If not found, return empty string
}

std::string getContentsRootPath() {
#ifdef __linux__
    // Check if running on WSL
    if (std::getenv("WSL_DISTRO_NAME")) {
        return "/mnt/c/dev/streaming-contents";
    } else {
        return "/home/dongvinpark/Documents/streaming-contents"; // for AWS EC2 Linux mounted with AWS Elastic File System.
    }
#elif _WIN32
    return "C:\\dev\\streaming-contents";
#elif __APPLE__
    return "/Users/dongvin99/Documents/for Mac Studio Dev Contents BackUP/streaming_contents_3.0";
#else
    throw std::runtime_error("Unsupported platform");
#endif
}

int main() {
    std::shared_ptr<Logger> logger = Logger::getLogger(C::MAIN);
    logger->warning("=================================================================");
    logger->warning("Dongvin, C++ AlphaStreamer3.1 RTSP Server STARTS. ver: " + std::string{C::VER});
    logger->warning("=================================================================");

    // make worker thread pool for boost.asio io_context
    boost::asio::io_context io_context;
    auto workGuard = boost::asio::make_work_guard(io_context);
    std::vector<std::thread> threadVec;
    auto threadCnt = std::thread::hardware_concurrency();
    for (auto i = 0; i < threadCnt; ++i) {
        threadVec.emplace_back(
            [&io_context]() {
                io_context.run();
            }
        );
    }
    logger->warning("Made io_cotext.run() worker thread pool with thread cnt: " + std::to_string(threadCnt));

    // used std::promise to synchronize the shutdown process
    std::promise<void> shutdownPromise;
    auto shutdownFuture = shutdownPromise.get_future();

    // handle exit signal using boost::asio::signal_set
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](const boost::system::error_code& ec, int signal) {
        try {
            if (!ec) {
                std::cout << "\n\t>>> Received signal: " << signal << ". Stopping server...\n";
                workGuard.reset();
                if (!io_context.stopped()) {
                    io_context.stop();
                    std::cout << "\t>>> io_context stopped.\n";
                }
                shutdownPromise.set_value();
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception during signal handling: " << e.what() << "\n";
            shutdownPromise.set_exception(std::make_exception_ptr(e));
        }
    });

    SntpRefTimeProvider sntpRefTimeProvider(io_context);

    std::string contentsRootPath = getContentsRootPath();

    ContentsStorage contentsStorage(contentsRootPath);
    contentsStorage.init();

    std::string projectRootDirPath = getProjectRoot();

    std::chrono::milliseconds inputIntervalMsForSessionRemoval(C::SHUTDOWN_SESSION_CLEAR_TASK_INTERVAL_MS);
    Server server(
        io_context,
        contentsStorage,
        contentsRootPath,
        sntpRefTimeProvider,
        projectRootDirPath,
        inputIntervalMsForSessionRemoval
    );
    // server.start(); is blocking function.
    // if server stop with uncaught exception, the following shutting down logic will never work.
    server.start();

    // Wait for shutdown to complete
    shutdownFuture.wait();

    // do cleaning before shutting down. shutdown all sessions and close boost asio io_context worker thread pool.
    // do not need to call server.shutdownServer();
    // destructor in Server class is called here automatically.

    for (auto& thread : threadVec) {
        if (thread.joinable()) {
            std::cout << "Joining io_context.run() worker thread " << thread.get_id() << "...\n";
            thread.join();
        } else {
            std::cout << "Cannot join thread : " << thread.get_id() << "\n";
        }
    }

    logger->warning("=================================================================");
    logger->warning("Dongvin, C++ AlphaStreamer3.1 RTSP Server SHUTS DOWN gracefully.");
    logger->warning("=================================================================");
    return 0;
}

/*
    test usage of VideoAccess in FileReader.

    std::cout <<"!!! FireReader test start !!!" << std::endl;
    FileReader& fileReader = contentsStorage.getReaders().at("enhypen-test-1cam-H");
    VideoAccess& videoAccess = fileReader.getVideoMetaWithLock().at("cam0");
    std::ifstream& fStream = videoAccess.getConstAccessList()[0];
    fStream.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(10);
    if (!fStream.read(reinterpret_cast<std::istream::char_type *>(buffer.data()), 10)) {
        throw std::ios_base::failure("Failed to read the file");
    }
    std::cout << "file read result : " << std::string(buffer.begin(), buffer.end()) << "\n";
*/

/*
    test usage to get a video and audio sample meta. this can be used when reading video sample from read file.

    const ContentFileMeta& fileReader = contentsStorage.getContentFileMetaMap().at("enhypen-test-1cam-H");

    const VideoAccess& videoAccess = fileReader.getConstVideoMeta().at("cam0");

    const std::vector<std::vector<VideoSampleInfo>>& videoMeta = videoAccess.getConstVideoSampleInfoList();
    std::cout << "enhypen-test-1cam-H cam 0 v1 video sample info !!!\n";
    for (const auto& sampleInfo : videoMeta.at(0)) {
        std::cout << "size : " << sampleInfo.getSize() << ", offset : " << sampleInfo.getOffset() << ", flag : " << sampleInfo.getFlag() << "\n";
    }

    std::cout << "enhypen-test-1cam-H cam 0 v2 video sample info !!!\n";
    for (const auto& sampleInfo : videoMeta.at(1)) {
        std::cout << "size : " << sampleInfo.getSize() << ", offset : " << sampleInfo.getOffset() << ", flag : " << sampleInfo.getFlag() << "\n";
    }

    const AudioAccess& audioAccess = fileReader.getConstAudioMeta();

    std::cout << "enhypen-test-1cam-H audio sample info !!!\n";
    for (const auto& audioSampleInfo : audioAccess.getConstMeta()) {
        std::cout << "size : " << audioSampleInfo.len << ", offset : " << audioSampleInfo.offset << "\n";
    }
*/

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
    std::vector<+unsigned char> audioData = {'A', 'u', 'd'};
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