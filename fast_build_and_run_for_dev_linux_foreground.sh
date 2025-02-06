#!/bin/bash

# Define variables
BUILD_DIR="build"
EXECUTABLE_NAME="AlphaStreamer3.1Cpp"  # Replace with your actual executable name

# Step 1: Ensure the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

# Step 2: Change directory to the build directory
cd "$BUILD_DIR" || exit

# Step 3: Run CMake to generate the build files
echo "Running CMake..."
cmake ..

# Step 4: Compile the project using make
echo "Building the project..."
make

# Step 5: Run the executable if build succeeds
if [ -f "$EXECUTABLE_NAME" ]; then
    echo "Running the executable..."
    ./"$EXECUTABLE_NAME"
else
    echo "Executable not found, build may have failed."
fi
