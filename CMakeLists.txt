cmake_minimum_required(VERSION 3.16)

# Project name and version
project(AlphaStreamer3.1Cpp VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add platform-specific settings
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Set toolchain for vcpkg (Windows only)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

    # Find Boost using vcpkg
    find_package(Boost REQUIRED COMPONENTS system)

    # Additional libraries required on Windows
    set(PLATFORM_LIBS ws2_32)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Find Boost using Homebrew (macOS only)
    find_package(Boost REQUIRED)

    # No additional libraries required for macOS
    set(PLATFORM_LIBS "")
endif()

# Include directories
include_directories(${Boost_INCLUDE_DIRS} src)

# Add the executable
add_executable(AlphaStreamer3.1Cpp
    src/main.cpp
	src/util/Logger.h
    src/util/Logger.cpp
	src/util/Buffer.h
	src/util/Buffer.cpp
    src/util/ParsableByteArray.h
    src/util/ParsableByteArray.cpp
    src/util/RtpInfo.h
    src/util/RtpInfo.cpp
    src/util/RtpMetaInfo.h
    src/util/RtpMetaInfo.cpp
    src/server/Server.h
    src/server/Server.cpp
    src/server/file/AudioSample.h
    src/server/file/AudioSample.cpp)

# Link Boost libraries and platform-specific libraries only for Windows
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(AlphaStreamer3.1Cpp Boost::system ${PLATFORM_LIBS})
endif()