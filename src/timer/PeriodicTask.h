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

    // constructor with no task
    explicit PeriodicTask(
        boost::asio::io_context& io_context,
        std::chrono::milliseconds interval
    );

    // constructor with task
    explicit PeriodicTask(
        boost::asio::io_context& io_context,
        std::chrono::milliseconds interval,
        TaskCallback task
    );
    void setTask(TaskCallback inputTask);
    void start();
    void stop();

private:
    void scheduleTask();

    std::shared_ptr<Logger> logger;
    boost::asio::steady_timer timer;
    std::chrono::milliseconds interval;
    TaskCallback task;
    bool running;
    bool isTaskSet;
};

#endif // PERIODICTASK_H