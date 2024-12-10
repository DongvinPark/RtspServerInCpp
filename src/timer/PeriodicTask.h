#ifndef PERIODICTASK_H
#define PERIODICTASK_H

#include <boost/asio.hpp>
#include <iostream>
#include <functional>
#include <chrono>
#include <memory>

#include "../src/util/Logger.h"

class PeriodicTask {
public:
    using TaskCallback = std::function<void()>;
    explicit PeriodicTask(
        boost::asio::io_context& io_context,
        std::chrono::milliseconds interval,
        TaskCallback task
    );
    void start();
    void stop();

private:
    void scheduleTask();

    std::shared_ptr<Logger> logger;
    boost::asio::steady_timer timer;
    std::chrono::milliseconds interval;
    TaskCallback task;
    bool running;
};

#endif // PERIODICTASK_H