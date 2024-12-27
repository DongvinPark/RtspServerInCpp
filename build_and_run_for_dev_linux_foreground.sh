#!/bin/bash

# Define variables
BUILD_DIR="build"
EXECUTABLE_NAME="AlphaStreamer3.1Cpp"  # Replace with your actual executable name

# Step 1: Change directory to the build directory
cd "$BUILD_DIR" || exit

# Step 2: Run CMake to generate the build files
echo "Running CMake..."
cmake ..

# Step 3: Compile the project using make
echo "Building the project..."
make

# Step 4: Run the executable
echo "Running the executable..."
./"$EXECUTABLE_NAME"

