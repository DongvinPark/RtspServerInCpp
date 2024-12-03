# 아래의 CMakeLists.txt 파일은  WindowOS Visual Stuido IDE를 위한 것이다..
# Linux 및 M Series Chip MacOS를 위한 CMakeLists.txt은 README.md에 별도로 기재하였다.
cmake_minimum_required(VERSION 3.29)
project(AlphaStreamer3.1Cpp)

set(CMAKE_CXX_STANDARD 20)

# Add Vcpkg toolchain file if using Vcpkg
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

find_package(Boost REQUIRED COMPONENTS system)

add_executable(
	AlphaStreamer3.1Cpp src/main.cpp
	"src/util/Logger.cpp"
	"src/util/Logger.h")

# Link Boost.System and Winsock (ws2_32)
target_link_libraries(AlphaStreamer3.1Cpp Boost::system ws2_32)