#!/bin/bash

# This script deletes all .txt files in the current directory except CMakeLists.txt.

echo "Deleting all .txt files except CMakeLists.txt in the current directory..."

# Loop through all .txt files in the current directory
for file in *.txt; do
    # Check if the file exists and is not "CMakeLists.txt"
    if [[ -f "$file" && "$file" != "CMakeLists.txt" ]]; then
        rm "$file"
        echo "Deleted: $file"
    fi
done

echo "Txt record logs cleanup complete."
