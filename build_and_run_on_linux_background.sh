#!/bin/bash

# Define variables
BUILD_DIR="build"
EXECUTABLE_NAME="AlphaStreamer3.1Cpp"  # Replace with your actual executable name

# Step 1: Delete the build directory if it exists
if [ -d "$BUILD_DIR" ]; then
    echo "Deleting existing build directory..."
    rm -rf "$BUILD_DIR"
fi

# Step 2: Make a new build directory
echo "Creating build directory..."
mkdir "$BUILD_DIR"

# Step 3: Change directory to the build directory
cd "$BUILD_DIR" || exit

# Step 4: Run CMake to generate the build files
echo "Running CMake..."
cmake ..

# Step 5: Compile the project using make
echo "Building the project..."
make

# Step 6: Run the executable
echo "Running the executable..."
nohup ./"$EXECUTABLE_NAME" &

