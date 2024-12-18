#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <memory>

// singleton design was used.
class Logger {
public:
    static std::shared_ptr<Logger> getLogger(const std::string& className);

    void severe(const std::string& msg);
    void warning(const std::string& msg);
    void info(const std::string& msg);
    void info2(const std::string& msg);
    void info3(const std::string& msg);
    void debug(const std::string& msg);
    void setUserInputPrompt(bool prompt);

private:
    explicit Logger(const std::string& className);

    std::string makeString(const std::string& msg, const std::string& level, const std::string& color);
    void print(const std::string& msg);

    std::string className;
    bool showPrompt;

    /*
    When writing a class to be used globally, such as a Logger or a constant value archive,
    the keywords static inline constexpr const can be helpful.

    static: Ensures the variable belongs to the class itself, not any instance.
    inline (C++17 or later): Allows the variable to appear in multiple translation units
    without violating the One Definition Rule (ODR).
    
    What is the ODR?
    The ODR (One Definition Rule) governs the use and definition of variables, functions,
    and types in C++. It consists of two main principles:
    A definition must appear only once across all translation units.
    Declarations can appear multiple times, but only one definition is allowed.
    
    What is a Translation Unit?
    A translation unit in C++ is the basic unit of compilation.
    It consists of a .cpp file along with all its included headers (via #include)
    and preprocessed code.

    Why Define Constants in Header Files?
    Defining constants in .cpp files instead of .h files can cause linker errors, such as:
    "Logger::Logger: cannot access private member."

    This error occurs because when the constants are defined in a .cpp file, the linker
    must resolve their references across the entire project.
    If only Logger.h is included in a translation unit (e.g., in main.cpp), the linker
    cannot find the definitions in Logger.cpp, causing the error.

    Solution:
    Always define global constants in the .h file like below!
    This ensures the constants are accessible across multiple translation
    units without violating the ODR.
    */
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