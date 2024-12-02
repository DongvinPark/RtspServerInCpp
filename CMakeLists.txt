# ������ CMakeLists.txt�� ������  WindowOS Visual Stuido IDE���̴�.
# Linux�� M Series Chip MacOS�� ���� CMakeLists.txt�� README.md�� ����� �ִ�.
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