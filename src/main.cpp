#include <boost/asio.hpp>
#include <iostream>

#include "util/Logger.h"
#include "util/C.h"
#include "util/Buffer.h"

using boost::asio::ip::tcp;

int main() {
    
    auto logger = Logger::getLogger("main");
    logger->severe("This is a severe message.");
    logger->warning("This is a warning message.");
    logger->info("This is an info message.");
    logger->info2("This is an info2 message.");
    logger->info3("This is an info3 message.");
    logger->debug("This is a debug message.");
    logger->setUserInputPrompt(false);
    logger->info("Input prompt disabled.");

    std::cout << "Use of global constants. Version : " << C::VERSION << "\n";
    std::cout << "Use of global constants. Hybrid Mode types : ";
    for(std::string mode : C::HYBRID_MODE_SET){
        std::cout << mode << ", ";
    }
    std::cout << "\n";

    std::vector<unsigned char> data = {'H', 'e', 'l', 'l', 'o'};
    Buffer buffer(data);

    // Assign a lambda function to afterTx
    buffer.afterTx = []() {
        std::cout << "Ward 'Hello' Transmission complete!" << std::endl;
    };

    // Trigger the afterTx callback
    if (buffer.afterTx) {
        buffer.afterTx();
    }

    std::cout << "buffer.toString() result : " << buffer.toString() << "\n";

    try {
        boost::asio::io_service io_service;
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 8554));

        std::cout << "Server is running on port 8554..." << std::endl;

        while (true) {
            tcp::socket socket(io_service);
            acceptor.accept(socket);
	        std::cout << "client socket connected!!\n";

            std::string message = "Server sent messsge!\n";

            std::cout << "sent message : " << message << "\n";

            boost::system::error_code ignored_error;
            boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
            std::cout << "wrote response!!\n";
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;

}
