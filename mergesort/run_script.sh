#!/bin/bash

# Check if argument is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <array_size>"
    exit 1
fi

# Compile the program
make

# Run the program with the specified array size
./merge_sort "$1"

# Clean up after execution
make clean
