# Parallel Island Model Genetic Algorithm for TSP

## Overview
This project implements a parallel island model genetic algorithm to solve the Traveling Salesman Problem (TSP). The algorithm uses OpenMP for parallelization and demonstrates the performance benefits of multi-threading in evolutionary algorithms.

## Problem
The Traveling Salesman Problem is a classic combinatorial optimization problem: given a list of cities and the distances between each pair, the goal is to find the shortest possible route that visits each city exactly once and returns to the origin city.

## Implementation Details

### Algorithm Components
- **Island Model**: The algorithm uses 12 islands with different mutation rates and selection pressures
- **Population**: Each island maintains a population of 200 individuals
- **Evolution**: Runs for 250 generations with periodic migration between islands
- **Crossover**: Implements Order Crossover (OX) to maintain valid routes
- **Mutation**: Random swap of cities with island-specific mutation rates
- **Selection**: Tournament selection with varying selection pressure per island
- **Migration**: Every 10 generations, individuals migrate between islands

### Parallelization Strategy
- OpenMP is used to parallelize the island evolution process
- Each island evolves independently in parallel
- Performance is measured across 1, 2, 4, 12, and 24 threads

### Analysis Tools
- A Python script (`graph.py`) is included to parse results and generate performance metrics:
  - Execution time vs. number of threads
  - Speedup vs. number of threads
  - Efficiency vs. number of threads

## Results
The implementation shows significant performance improvements with increased thread count, although with diminishing returns as thread count approaches the number of islands.

## Usage

### Compiling the C++ Program
```bash
g++ -fopenmp -o tsp_solver main.cpp -std=c++17
```

### Running the Algorithm
```bash
./tsp_solver
```

### Analyzing Results
```bash
python graph.py results.txt
```

## Technical Notes
- The algorithm is implemented in C++17 and uses modern C++ features
- Random city generation creates a problem instance with 200 cities
- The solution quality can vary between runs due to the stochastic nature of genetic algorithms
- The project demonstrates both algorithmic optimization and parallel computing techniques
