#include "IOContextHolder.h"

IOContextHolder::IOContextHolder() 
    : ioContext(std::make_shared<boost::asio::io_context>()) {}

IOContextHolder::~IOContextHolder() {
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void IOContextHolder::start() {
    auto workGuard = boost::asio::make_work_guard(*ioContext);

    unsigned int num_threads = std::thread::hardware_concurrency();

    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this]() {
            ioContext->run(); // Run io_context on each thread
        });
    }
}

std::shared_ptr<boost::asio::io_context> IOContextHolder::getIOContext() {
    return ioContext;
}