#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <memory>

class Logger {
public:
    static std::shared_ptr<Logger> getLogger(const std::string& className);

    explicit Logger(const std::string& className);
    void severe(const std::string& msg);
    void warning(const std::string& msg);
    void info(const std::string& msg);
    void info2(const std::string& msg);
    void info3(const std::string& msg);
    void debug(const std::string& msg);
    void setUserInputPrompt(bool prompt);

private:

    std::string makeString(const std::string& msg, const std::string& level, const std::string& color);
    void print(const std::string& msg);

    std::string className;
    bool showPrompt;

    static inline constexpr const char* ANSI_RESET = "\033[0m";
    static inline constexpr const char* ANSI_RED = "\033[31m";
    static inline constexpr const char* ANSI_GREEN = "\033[32m";
    static inline constexpr const char* ANSI_YELLOW = "\033[33m";
    static inline constexpr const char* ANSI_PURPLE = "\u001B[35m";
    static inline constexpr const char* ANSI_BLUE = "\033[34m";
    static inline constexpr const char* ANSI_CYAN = "\033[36m";
    static inline constexpr const char* ANSI_WHITE = "\033[37m";
};

#endif // LOGGER_H