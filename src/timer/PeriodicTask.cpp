#include "../include/PeriodicTask.h"
#include "../constants/C.h"

PeriodicTask::PeriodicTask(
    boost::asio::io_context& io_context,
    std::chrono::milliseconds inputInterval
) : timer(io_context),
    interval(inputInterval),
    running(false),
    isTaskSet(false),
    logger(Logger::getLogger(C::PERIODIC_TASK)) {}

PeriodicTask::PeriodicTask(
    boost::asio::io_context& io_context,
    std::chrono::milliseconds inputInterval,
    TaskCallback inputTask
)    : timer(io_context),
    interval(inputInterval),
    task(std::move(inputTask)),
    running(false),
    isTaskSet(true),
    logger(Logger::getLogger(C::PERIODIC_TASK)) {}

void PeriodicTask::setTask(TaskCallback inputTask) {
    task = std::move(inputTask);
    isTaskSet = true;
}

void PeriodicTask::setInterval(std::chrono::milliseconds inputInterval) {
    interval = inputInterval;
}

void PeriodicTask::start() {
    if (!isTaskSet) {
        logger->severe("no task to run!");
    } else {
        running = true;
        scheduleTask();
    }
}

void PeriodicTask::stop() {
    logger->severe("task stop called!");
    running = false;
    timer.cancel();
}

void PeriodicTask::scheduleTask() {
    if(!running){
        timer.expires_after(std::chrono::milliseconds(C::ZERO));
        timer.cancel();
    } else {
        timer.expires_after(interval);
        timer.async_wait(
            [this](const boost::system::error_code& ec){
                if(!ec){
                    if(task){
                        task();
                    }
                    if(running){
                        scheduleTask();
                    }
                } else {
                    // used ec.message() instead of ec.what() for boost backward comparability
                    logger->severe("boost steady timer failed! error message : " + ec.message());
                }
            }
        );
    }
}
