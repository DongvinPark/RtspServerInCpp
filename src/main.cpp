#include <boost/asio.hpp>
#include <iostream>

#include "dto/Res.h"

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_service io_service;
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 8554));

        std::cout << "Server is running on port 8554..." << std::endl;

        while (true) {
            tcp::socket socket(io_service);
            acceptor.accept(socket);
	        std::cout << "client socket connected!!\n";

            Res resObject{ 9, "DongvinPark" };
            std::string message = "Server sent Object to Client >> key : "
            + std::to_string(resObject.getKey())
            + ", val : " + resObject.getVal() + "\n";

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
