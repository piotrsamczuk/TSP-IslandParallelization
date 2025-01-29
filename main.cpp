#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <random>
#include <vector>

// Parametry algorytmu
const int num_cities = 200;
const int num_islands = 8;
const int population_size = 200;
const int num_generations = 400;
const int migration_interval = 10;
const int migration_size = 10;

// Parametry mutacji i selekcji dla każdej wyspy
const std::vector<std::pair<double, int>> island_params = {{0.1, 10}, {0.2, 15}, {0.05, 5}, {0.15, 20},
                                                           {0.1, 10}, {0.2, 15}, {0.05, 5}, {0.15, 20}};

// Losowa generacja miast (na siatce 100x100)
std::vector<std::pair<double, double>> cities;

// Funkcja obliczania długości trasy (koszt)
double calculate_distance(const std::vector<int>& route)
{
    double dist = 0;
    for (size_t i = 0; i < route.size(); ++i)
    {
        const auto& city1 = cities[route[i]];
        const auto& city2 = cities[route[(i + 1) % route.size()]];
        dist += std::hypot(city1.first - city2.first, city1.second - city2.second);
    }
    return dist;
}

// Inicjalizacja populacji
std::vector<std::vector<int>> initialize_population()
{
    std::vector<std::vector<int>> population;
    std::vector<int> base(num_cities);
    std::iota(base.begin(), base.end(), 0);
    for (int i = 0; i < population_size; ++i)
    {
        std::vector<int> individual = base;
        std::shuffle(individual.begin(), individual.end(), std::mt19937(std::random_device()()));
        population.push_back(individual);
    }
    return population;
}

// Order Crossover (krzyżowanie)
std::vector<int> order_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2)
{
    int size = parent1.size();
    std::vector<int> child(size, -1);
    int start = std::rand() % size;
    int end = std::rand() % size;
    if (start > end)
        std::swap(start, end);
    for (int i = start; i <= end; ++i)
    {
        child[i] = parent1[i];
    }
    int pointer = 0;
    for (int city : parent2)
    {
        if (std::find(child.begin(), child.end(), city) == child.end())
        {
            while (child[pointer] != -1)
                ++pointer;
            child[pointer] = city;
        }
    }
    return child;
}

// Mutacja - zamiana miejscami dwóch miast
void mutate(std::vector<int>& route, double mutation_rate)
{
    if (static_cast<double>(std::rand()) / RAND_MAX < mutation_rate)
    {
        int i = std::rand() % route.size();
        int j = std::rand() % route.size();
        std::swap(route[i], route[j]);
    }
}

// Ewolucja populacji na wyspie
std::vector<std::vector<int>> evolve_population(std::vector<std::vector<int>> population, double mutation_rate,
                                                int selection_pressure)
{
    std::sort(population.begin(), population.end(),
              [](const std::vector<int>& a, const std::vector<int>& b)
              { return calculate_distance(a) < calculate_distance(b); });
    std::vector<std::vector<int>> new_population = {population[0]}; // Elityzm
    while (new_population.size() < population_size)
    {
        int parent1_idx = std::rand() % selection_pressure;
        int parent2_idx = std::rand() % selection_pressure;
        std::vector<int> child = order_crossover(population[parent1_idx], population[parent2_idx]);
        mutate(child, mutation_rate);
        new_population.push_back(child);
    }
    return new_population;
}

// Klasa wyspy
class Island
{
  public:
    Island(int island_id, double mutation_rate, int selection_pressure)
        : id(island_id), mutation_rate(mutation_rate), selection_pressure(selection_pressure)
    {
        population = initialize_population();
    }

    std::pair<std::vector<int>, double> evolve()
    {
        population = evolve_population(population, mutation_rate, selection_pressure);
        return get_best_solution();
    }

    std::pair<std::vector<int>, double> get_best_solution()
    {
        auto best_route = *std::min_element(population.begin(), population.end(),
                                            [](const std::vector<int>& a, const std::vector<int>& b)
                                            { return calculate_distance(a) < calculate_distance(b); });
        return {best_route, calculate_distance(best_route)};
    }

    void receive_migrants(const std::vector<std::vector<int>>& migrants)
    {
        population.insert(population.end(), migrants.begin(), migrants.end());
    }

    std::vector<std::vector<int>> send_migrants()
    {
        std::vector<std::vector<int>> migrants;
        std::sample(population.begin(), population.end(), std::back_inserter(migrants), migration_size,
                    std::mt19937(std::random_device()()));
        return migrants;
    }

  private:
    int id;
    double mutation_rate;
    int selection_pressure;
    std::vector<std::vector<int>> population;
};

// Funkcja do zapisywania wyników do pliku
void save_results_to_file(const std::vector<std::pair<std::vector<int>, double>>& best_solutions, int generation,
                          int num_threads)
{
    std::ofstream outfile("results.txt", std::ios_base::app);
    outfile << "Pokolenie " << generation + 1 << " (Threads: " << num_threads << ")" << std::endl;
    for (size_t i = 0; i < best_solutions.size(); ++i)
    {
        auto [route, dist] = best_solutions[i];
        outfile << "  Wyspa " << i + 1 << ", Najlepsza trasa: " << dist << std::endl;
    }
    outfile.close();
}

// Funkcja do uruchomienia algorytmu dla określonej liczby wątków
void run_algorithm(int num_threads)
{
    omp_set_num_threads(num_threads); // Ustawienie liczby wątków

    // Inicjalizacja wysp
    std::vector<Island> islands;
    for (int i = 0; i < num_islands; ++i)
    {
        islands.emplace_back(i, island_params[i].first, island_params[i].second);
    }

    // Główna pętla algorytmu
    std::vector<std::pair<std::vector<int>, double>> final_best_solutions;
    for (int generation = 0; generation < num_generations; ++generation)
    {
        std::vector<std::pair<std::vector<int>, double>> best_solutions(num_islands);

#pragma omp parallel for
        for (int i = 0; i < num_islands; ++i)
        {
            best_solutions[i] = islands[i].evolve();
        }

        // Zapisywanie wyników do pliku
        save_results_to_file(best_solutions, generation, num_threads);

        // Migracja co migration_interval pokoleń
        if ((generation + 1) % migration_interval == 0)
        {
            std::vector<std::vector<std::vector<int>>> migrants(num_islands);

#pragma omp parallel for
            for (int i = 0; i < num_islands; ++i)
            {
                migrants[i] = islands[i].send_migrants();
            }

            for (int i = 0; i < num_islands; ++i)
            {
                int target_island = (i + 1) % num_islands;
                islands[target_island].receive_migrants(migrants[i]);
            }
        }

        // Zapisywanie wyników ostatniej iteracji
        if (generation == num_generations - 1)
        {
            final_best_solutions = best_solutions;
        }
    }

    // Wyświetlenie wyników ostatniej iteracji
    std::cout << "Ostatnie pokolenie (Threads: " << num_threads << ")" << std::endl;
    for (size_t i = 0; i < final_best_solutions.size(); ++i)
    {
        auto [route, dist] = final_best_solutions[i];
        std::cout << "  Wyspa " << i + 1 << ", Najlepsza trasa: " << dist << std::endl;
    }
}

int main()
{
    // Inicjalizacja miast
    std::srand(std::time(nullptr));
    for (int i = 0; i < num_cities; ++i)
    {
        cities.push_back(
            {static_cast<double>(std::rand()) / RAND_MAX * 100, static_cast<double>(std::rand()) / RAND_MAX * 100});
    }

    // Uruchomienie algorytmu sekwencyjnie
    std::cout << "Uruchamianie sekwencyjne..." << std::endl;
    auto start_seq = std::chrono::high_resolution_clock::now();
    run_algorithm(1); // 1 wątek = sekwencyjnie
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seq = end_seq - start_seq;
    std::cout << "Czas wykonania sekwencyjnego: " << elapsed_seq.count() << " s" << std::endl;

    // Uruchomienie algorytmu dla 2 wątków
    std::cout << "Uruchamianie dla 2 wątków..." << std::endl;
    auto start_2 = std::chrono::high_resolution_clock::now();
    run_algorithm(2);
    auto end_2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_2 = end_2 - start_2;
    std::cout << "Czas wykonania dla 2 wątków: " << elapsed_2.count() << " s" << std::endl;

    // Uruchomienie algorytmu dla 4 wątków
    std::cout << "Uruchamianie dla 4 wątków..." << std::endl;
    auto start_4 = std::chrono::high_resolution_clock::now();
    run_algorithm(4);
    auto end_4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_4 = end_4 - start_4;
    std::cout << "Czas wykonania dla 4 wątków: " << elapsed_4.count() << " s" << std::endl;

    // Uruchomienie algorytmu dla 12 wątków
    std::cout << "Uruchamianie dla 12 wątków..." << std::endl;
    auto start_12 = std::chrono::high_resolution_clock::now();
    run_algorithm(12);
    auto end_12 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_12 = end_12 - start_12;
    std::cout << "Czas wykonania dla 12 wątków: " << elapsed_12.count() << " s" << std::endl;

    // Uruchomienie algorytmu dla 24 wątków
    std::cout << "Uruchamianie dla 24 wątków..." << std::endl;
    auto start_24 = std::chrono::high_resolution_clock::now();
    run_algorithm(24);
    auto end_24 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_24 = end_24 - start_24;
    std::cout << "Czas wykonania dla 24 wątków: " << elapsed_24.count() << " s" << std::endl;

    return 0;
}