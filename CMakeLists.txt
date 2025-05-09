cmake_minimum_required(VERSION 3.16)

# Project name and version
project(RtspServerInCpp VERSION 1.0)

# Set C++ standard to 17.
set(CMAKE_CXX_STANDARD 17)
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

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Find Boost
    find_package(Boost REQUIRED)

    # Link pthread library for Linux
    set(PLATFORM_LIBS pthread)
endif()

# Include directories
include_directories(${Boost_INCLUDE_DIRS} src)

# Add the executable
add_executable(RtspServerInCpp
        src/main.cpp
        include/Logger.h
        src/util/Logger.cpp
        include/Buffer.h
        src/util/Buffer.cpp
        include/ParsableByteArray.h
        src/util/ParsableByteArray.cpp
        include/RtpInfo.h
        src/util/RtpInfo.cpp
        include/RtpMetaInfo.h
        src/util/RtpMetaInfo.cpp
        include/Server.h
        src/server/Server.cpp
        constants/Util.h
        include/SntpRefTimeProvider.h
        src/server/SntpRefTimeProvider.cpp
        include/PeriodicTask.h
        src/timer/PeriodicTask.cpp
        include/RxBitrate.h
        src/util/RxBitrate.cpp
        include/ContentFileMeta.h
        src/server/file/ContentFileMeta.cpp
        include/AudioSampleInfo.h
        include/VideoSampleInfo.h
        src/server/file/access/AudioSampleInfo.cpp
        src/server/file/access/VideoSampleInfo.cpp
        include/AudioAccess.h
        include/VideoAccess.h
        src/server/file/access/AudioAccess.cpp
        src/server/file/access/VideoAccess.cpp
        include/HybridSampleMeta.h
        src/util/HybridSampleMeta.cpp
        include/Session.h
        src/service/Session.cpp
        include/ContentsStorage.h
        src/server/ContentsStorage.cpp
        include/RtspHandler.h
        src/service/RtspHandler.cpp
        include/RtpHandler.h
        src/service/RtpHandler.cpp
        include/StreamHandler.h
        src/service/StreamHandler.cpp
        include/ReadInfo.h
        src/service/ReadInfo.cpp
)

# Link Boost libraries and platform-specific libraries
target_link_libraries(RtspServerInCpp ${Boost_LIBRARIES} ${PLATFORM_LIBS})
