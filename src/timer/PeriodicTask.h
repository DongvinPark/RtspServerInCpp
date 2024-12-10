#ifndef PERIODICTASK_H
#define PERIODICTASK_H

#include <boost/asio.hpp>
#include <iostream>
#include <functional>
#include <chrono>

class PeriodicTask {
public:
    using TaskCallback = std::function<void()>;
    explicit PeriodicTask(
        boost::asio::io_context& io_context,
        std::chrono::milliseconds interval,
        TaskCallback task
    );
    void start();

private:
    void scheduleTask();

    boost::asio::steady_timer timer;
    std::chrono::milliseconds interval;
    TaskCallback task;
};

#endif // PERIODICTASK_H