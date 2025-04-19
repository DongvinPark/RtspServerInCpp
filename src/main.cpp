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
    #include <pthread.h>
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
// 1.0.0        2025.02.25.     PLAY, PAUSE, SEEK, Paused-SEEK, Cam Switching, Hybrid D&S, Looking Sample Control
// 1.1.0        2025.04.4.      Performance test done. RTP packet sending and memory management architecture were updated.

using boost::asio::ip::tcp;

std::string getExecutablePath() {
    char path[1024];

#ifdef _WIN32
    GetModuleFileNameA(NULL, path, sizeof(path));
#elif __APPLE__
    proc_pidpath(getpid(), path, sizeof(path));
#elif __linux__
    ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
    if (count != -1) path[count] = '\0';
#endif

    return std::string(path);
}

std::string getProjectRoot() {
    std::filesystem::path exePath = getExecutablePath();

    // Traverse up until we find the "RtspServer" directory
    while (exePath.has_parent_path()) {
        exePath = exePath.parent_path();
        if (exePath.filename() == "RtspServerInCpp") {
            return exePath.string();
        }
    }

    return "";
}

std::string getContentsRootPath() {
#ifdef __linux__
    // Check if running on WSL
    if (std::getenv("WSL_DISTRO_NAME")) {
        return "/mnt/c/dev/streaming-contents";
    } else {
        return "/home/ec2-user/streaming_contents"; // for AWS EC2 Linux mounted with AWS Elastic File System.
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

    // make worker thread pool for main boost.asio io_context
    boost::asio::io_context io_context;
    auto workGuard = boost::asio::make_work_guard(io_context);
    std::vector<std::thread> threadVec;
    int cpuCoreCnt = static_cast<int>(std::thread::hardware_concurrency());
    for (auto i = 0; i < cpuCoreCnt; ++i) {
        threadVec.emplace_back(
            [&io_context]() {
                Util::set_thread_priority();
                io_context.run();
            }
        );
    }

    // make threads pool for worker io_context pool
    std::vector<std::shared_ptr<boost::asio::io_context>> ioContextPool;
    std::vector<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> workGuardVec;
    for (int i = 0; i < cpuCoreCnt; ++i) {
        auto workerIoContextPtr = std::make_shared<boost::asio::io_context>();
        ioContextPool.emplace_back(workerIoContextPtr);

        // create a work guard to prevent io_context from stopping
        workGuardVec.emplace_back(boost::asio::make_work_guard(*workerIoContextPtr));

        // create worker threads for this io_context
        for (int j = 0; j < C::THREAD_CNT_PER_WORKER_IO_CONTEXT; ++j) {
            threadVec.emplace_back([workerIoContextPtr]() {
                Util::set_thread_priority();
                workerIoContextPtr->run();
            });
        }
    }

    logger->warning(
        "Made io_cotext.run() worker thread pool with thread cnt: "
        + std::to_string(cpuCoreCnt + (cpuCoreCnt*C::THREAD_CNT_PER_WORKER_IO_CONTEXT))
    );

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

                for (int i = 0; i < cpuCoreCnt; ++i){
                    workGuardVec[i].reset();
                }

                if (!io_context.stopped()) {
                    io_context.stop();
                }

                for (int i = 0; i < cpuCoreCnt; ++i){
                    if (!ioContextPool[i]->stopped()){
                        ioContextPool[i]->stop();
                    }
                }

                std::cout << "\t>>> all io_context stopped.\n";
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

    const ContentFileMeta& fileReader = contentsStorage.getContentFileMetaMap().at("enhypen-test-1cam-H");

    const VideoAccess& videoAccess = fileReader.getConstVideoMeta().at("cam0");

    std::string projectRootDirPath = getProjectRoot();

    std::chrono::milliseconds inputIntervalMsForSessionRemoval(C::SHUTDOWN_SESSION_CLEAR_TASK_INTERVAL_MS);
    Server server(
        io_context,
        ioContextPool,
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

    // do cleaning before shutting down.

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