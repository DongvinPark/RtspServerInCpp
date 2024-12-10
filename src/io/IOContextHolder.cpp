#include "IOContextHolder.h"

/*
	boost::asio::io_context& IOContext;
	std::vector<std::thread> threadVector;
	WorkGuardType workGuard;
*/

IOContextHolder::IOContextHolder(boost::asio::io_context& iOContext)
    : IOContext(iOContext), threadVector(), workGuard(boost::asio::make_work_guard(iOContext)) {
    unsigned int num_threads = std::thread::hardware_concurrency();

    // Launch threads to run the io_context
    for (unsigned int i = 0; i < num_threads; ++i) {
        threadVector.emplace_back([&iOContext]() {
            iOContext.run();
            });
    }
}

IOContextHolder::~IOContextHolder() {
    for (auto& t : threadVector) {
        t.join();
    }
    workGuard.reset();
}

void IOContextHolder::start() {
}

boost::asio::io_context& IOContextHolder::getIOContext() {
    return IOContext;
}
