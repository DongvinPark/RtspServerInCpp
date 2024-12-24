#include "../include/Logger.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

Logger::Logger(const std::string& className)
    : className(className), showPrompt(false) {}

std::shared_ptr<Logger> Logger::getLogger(const std::string& className) {
    return std::shared_ptr<Logger>(new Logger(className));
}

void Logger::setUserInputPrompt(bool prompt) {
    showPrompt = prompt;
}

void Logger::severe(const std::string& msg) {
    print(makeString(msg, "SEVERE", ANSI_RED));
}

void Logger::warning(const std::string& msg) {
    print(makeString(msg, "WARNING", ANSI_YELLOW));
}

void Logger::info(const std::string& msg) {
    print(makeString(msg, "INFO", ANSI_WHITE));
}

void Logger::info2(const std::string& msg) {
    print(makeString(msg, "INFO", ANSI_YELLOW));
}

void Logger::info3(const std::string& msg) {
    print(makeString(msg, "INFO", ANSI_PURPLE));
}

void Logger::debug(const std::string& msg) {
    print(makeString(msg, "DEBUG", ANSI_GREEN));
}

std::string Logger::makeString(const std::string& msg, const std::string& level, const std::string& color) {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto local_time = *std::localtime(&time_t_now);

    std::ostringstream oss;
    oss << color
        << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S")
        << " [" << level << "] " << className << ": " << msg
        << ANSI_RESET;
    return oss.str();
}

void Logger::print(const std::string& msg) {
    std::cout << msg << "\n";
    if (showPrompt) {
        std::cout << ">> ";
    }
}
