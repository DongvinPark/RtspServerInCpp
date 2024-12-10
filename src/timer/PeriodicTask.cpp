#include "PeriodicTask.h"

PeriodicTask::PeriodicTask(
    boost::asio::io_context& io_context,
    std::chrono::milliseconds inputInterval,
    TaskCallback inputTask)
    : timer(io_context), interval(inputInterval), task(std::move(inputTask)) {}

void PeriodicTask::start() {
    scheduleTask();
}

void PeriodicTask::scheduleTask() {
    timer.expires_after(interval);
    timer.async_wait(
        [this](const boost::system::error_code& ec){
            if(!ec){
                if(task){
                    task();
                }
                scheduleTask();
            } else {
                std::cout << ec.what();
            }
        }
    );
}
