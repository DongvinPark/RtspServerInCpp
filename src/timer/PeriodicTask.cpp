#include "PeriodicTask.h"

PeriodicTask::PeriodicTask(
    boost::asio::io_context& io_context,
    std::chrono::milliseconds inputInterval,
    TaskCallback inputTask)
    : timer(io_context),
    interval(inputInterval),
    task(std::move(inputTask)),
    running(false),
    logger(Logger::getLogger("PeriodicTask")) {}

void PeriodicTask::start() {
    running = true;
    scheduleTask();
}

void PeriodicTask::stop() {
    logger->severe("task stop called!!");
    running = false;
}

void PeriodicTask::scheduleTask() {
    if(!running){
        timer.expires_from_now();
        timer.cancel();
        logger->severe("Timer stops.");
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
                    logger->severe("boost steady timer failed! error message : " + ec.what());
                }
            }
        );
    }
}
