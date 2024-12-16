#include "PeriodicTask.h"

PeriodicTask::PeriodicTask(
    boost::asio::io_context& io_context,
    std::chrono::milliseconds inputInterval
) : timer(io_context),
    interval(inputInterval),
    running(false),
    isTaskSet(false),
    logger(Logger::getLogger("PeriodicTask")) {}

PeriodicTask::PeriodicTask(
    boost::asio::io_context& io_context,
    std::chrono::milliseconds inputInterval,
    TaskCallback inputTask
)    : timer(io_context),
    interval(inputInterval),
    task(std::move(inputTask)),
    running(false),
    isTaskSet(true),
    logger(Logger::getLogger("PeriodicTask")) {}

void PeriodicTask::setTask(TaskCallback inputTask) {
    task = std::move(inputTask);
    isTaskSet = true;
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
}

void PeriodicTask::scheduleTask() {
    if(!running){
        timer.expires_from_now();
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
