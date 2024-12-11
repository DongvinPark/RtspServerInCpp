#ifndef IOCONTEXTHOLDER_H
#define IOCONTEXTHOLDER_H

#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <memory>

class IOContextHolder {
public:
    IOContextHolder();
    ~IOContextHolder();

    void start(); 
    std::shared_ptr<boost::asio::io_context> getIOContext();

private:
    std::shared_ptr<boost::asio::io_context> ioContext;
    std::vector<std::thread> threads;
};

#endif // IOCONTEXTHOLDER_H