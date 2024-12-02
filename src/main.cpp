#include <boost/asio.hpp>
#include <iostream>

#include "util/Logger.h"

using boost::asio::ip::tcp;

int main() {
    
    auto logger = Logger::getLogger("main");
    logger->severe("This is a severe message.");
    logger->warning("This is a warning message.");
    logger->info("This is an info message.");
    logger->debug("This is a debug message.");
    logger->setUserInputPrompt(false);
    logger->info("Input prompt disabled.");
    return 0;

}
