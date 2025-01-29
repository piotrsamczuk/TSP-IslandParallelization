#!/bin/bash

# Compile the C++ program (if not already compiled)
echo "Compiling the C++ program..."
g++ -o tsp_parallel main.cpp -fopenmp

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi

echo "Compilation successful. Running the program 10 times..."

# Clear the results file if it exists
> results.txt

# Run the program 10 times
for i in {1..10}
do
    echo "Run $i/10"
    ./tsp_parallel >> results.txt
done

echo "All 100 runs completed. Processing results with Python..."

# Run the Python script to process the results
python3 graph.py

echo "Results processed. Check the output of the Python script."