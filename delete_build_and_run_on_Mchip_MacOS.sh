#!/bin/bash

# Exit on error
set -e

# Build directory
BUILD_DIR=build

# Remove the existing build directory and recreate it
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi
mkdir "$BUILD_DIR"

# Navigate to the build directory
cd "$BUILD_DIR"

# Run CMake and build the project
cmake ..
make

# Run the compiled executable
./RtspServerInCpp