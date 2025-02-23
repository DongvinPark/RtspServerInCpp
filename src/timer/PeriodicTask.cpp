#include "../include/PeriodicTask.h"

#include <iostream>

#include "../constants/C.h"

PeriodicTask::PeriodicTask(
    boost::asio::io_context& io_context,
    boost::asio::strand<boost::asio::io_context::executor_type> inputStrand,
    std::chrono::milliseconds inputInterval
) : strand(std::move(inputStrand)),
    timer(io_context),
    interval(inputInterval),
    running(false),
    isTaskSet(false),
    logger(Logger::getLogger(C::PERIODIC_TASK)) {}

PeriodicTask::PeriodicTask(
    boost::asio::io_context& io_context,
    boost::asio::strand<boost::asio::io_context::executor_type> inputStrand,
    std::chrono::milliseconds inputInterval,
    TaskCallback inputTask
) : strand(std::move(inputStrand)),
    timer(io_context),
    interval(inputInterval),
    task(std::move(inputTask)),
    running(false),
    isTaskSet(true),
    logger(Logger::getLogger(C::PERIODIC_TASK)) {}

PeriodicTask::~PeriodicTask(){}

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
    if (running == false) return;
    logger->severe("task stop called!");
    running = false;
    boost::asio::post(strand, [this]{ timer.cancel(); });
}

void PeriodicTask::scheduleTask() {
    timer.expires_after(interval);
    timer.async_wait(boost::asio::bind_executor(
        strand,
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
    ));
}
