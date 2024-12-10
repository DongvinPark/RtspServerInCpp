#include <boost/asio.hpp>
#include <iostream>

#include "util/Logger.h"
#include "util/C.h"
#include "util/Buffer.h"
#include "server/file/AudioSample.h"
#include "util/Util.h"
#include "util/AVSampleBuffer.h"
#include "timer/PeriodicTask.h"

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

    std::cout << (unsigned char)0x24 << "\n"; // this will print '$' in terminal console.

    // Trigger the afterTx callback
    if (buffer.afterTx) {
        buffer.afterTx();
    }

    std::cout << "buffer.toString() result : " << buffer.toString() << "\n";

    std::cout << "AudioSample Test Start!!\n";
    std::vector<unsigned char> audioData = {'A', 'u', 'd'};
    AudioSample aSample(audioData, audioData.size());
    std::cout << "AudioSample toString : " << aSample.toString() << "\n";

    std::cout << "Get random key for rtsp session Id \n";
    std::cout << Util::getRandomKey(64) << "\n";

    std::cout << "AVSampleBuffer test start!!\n";
    AVSampleBuffer avSample(C::VIDEO_ID);
    avSample.setKill();
    std::cout << avSample.toString();

    boost::asio::io_context io_context;

    std::cout << "\nPeriodic Timer Task start!\n";

    auto myTask = []() {
        std::cout << "Lambda-based periodic task executed at: "
                  << std::chrono::system_clock::now().time_since_epoch().count()
                  << "\n";
    };

    std::chrono::milliseconds interval(1000); // 1 second
    PeriodicTask task(io_context, interval, myTask);
    task.start();
    io_context.run();

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    task.stop();

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
