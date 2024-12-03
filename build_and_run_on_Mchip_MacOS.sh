#!/bin/bash

# Exit on error
set -e

# Build directory
BUILD_DIR=build

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir $BUILD_DIR
fi

# Navigate to the build directory
cd $BUILD_DIR

# Run CMake and build the project
cmake ..
make

# Run the compiled executable
./MyBoostProject