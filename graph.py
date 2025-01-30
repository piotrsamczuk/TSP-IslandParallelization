import re
import matplotlib.pyplot as plt
import numpy as np
from statistics import mean
import sys

def parse_results(filename):
    results = []
    current_run = {}
    
    with open(filename, 'r', encoding='utf-8') as file:
        content = file.read()
        runs = content.split('Uruchamianie sekwencyjne...')
        
        for run in runs[1:]:  # Skip empty first split
            try:
                # Extract execution times
                times = {}
                
                sequential_time = re.search(r'Czas wykonania sekwencyjnego: (\d+\.\d+)', run)
                if sequential_time:
                    times[1] = float(sequential_time.group(1))
                    
                for thread_count in [2, 4, 12, 24]:
                    time_match = re.search(f'Czas wykonania dla {thread_count} wątków: (\d+\.\d+)', run)
                    if time_match:
                        times[thread_count] = float(time_match.group(1))
                
                # Extract final routes (from the last generation) for each thread count
                def get_final_routes(thread_count, text):
                    # Find all generations for this thread count
                    pattern = f'Pokolenie \\d+ \\(Threads: {thread_count}\\)(.*?)(?=Pokolenie|Czas|$)'
                    generations = re.finditer(pattern, text, re.DOTALL)
                    
                    # Get the last generation's data
                    last_gen = None
                    for gen in generations:
                        last_gen = gen.group(1)
                    
                    if last_gen:
                        routes = re.findall(r'Najlepsza trasa: (\d+\.\d+)', last_gen)
                        return [float(x) for x in routes]
                    return None
                
                routes = {}
                for thread_count in [1, 2, 4, 12, 24]:
                    final_routes = get_final_routes(thread_count, run)
                    if final_routes:
                        routes[thread_count] = final_routes
                
                # Only add the run if we have both times and routes
                if times and routes and len(times) == 5 and len(routes) == 5:
                    results.append({
                        'times': times,
                        'routes': routes
                    })
                    
            except Exception as e:
                print(f"Error processing run: {str(e)}")
                continue
    
    if not results:
        raise ValueError("No valid results were parsed from the file")
    
    print(f"Successfully parsed {len(results)} complete runs")
    return results

def create_performance_graphs(results):
    thread_counts = [1, 2, 4, 12, 24]
    
    # Calculate average times across all runs
    avg_times = []
    std_times = []
    for threads in thread_counts:
        times = [run['times'][threads] for run in results]
        avg_times.append(mean(times))
        std_times.append(np.std(times))
    
    # Calculate speedup and efficiency
    speedup = [avg_times[0] / time for time in avg_times]
    efficiency = [s / t for s, t in zip(speedup, thread_counts)]
    
    # Create figure with subplots
    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(15, 5))
    
    # Common x-axis settings
    for ax in [ax1, ax2, ax3]:
        ax.set_xticks(thread_counts)
        ax.set_xticklabels(thread_counts)
        ax.grid(True)
    
    # Execution time plot
    ax1.errorbar(thread_counts, avg_times, yerr=std_times, fmt='o-', capsize=5)
    ax1.set_xlabel('Liczba wątków')
    ax1.set_ylabel('Czas wykonania (s)')
    ax1.set_title('Czas wykonania vs Liczba wątków')
    max_time = max(avg_times) + max(std_times)
    ax1.set_ylim(0, max_time * 1.1)
    
    # Speedup plot
    ax2.plot(thread_counts, speedup, 'o-')
    ax2.set_xlabel('Liczba wątków')
    ax2.set_ylabel('Przyspieszenie')
    ax2.set_title('Przyspieszenie vs Liczba wątków')
    max_speedup = max(speedup)
    ax2.set_ylim(0, max_speedup * 1.1)
    
    # Efficiency plot
    ax3.plot(thread_counts, efficiency, 'o-')
    ax3.set_xlabel('Liczba wątków')
    ax3.set_ylabel('Efektywność')
    ax3.set_title('Efektywność vs Liczba wątków')
    ax3.set_ylim(0, 1.1)
    
    plt.tight_layout()
    plt.savefig('performance_analysis.png', dpi=300, bbox_inches='tight')
    
    # Print statistical summary
    print("\nPodsumowanie wydajności:")
    print("Liczba wątków | Średni czas (s) | Przyspieszenie | Efektywność")
    print("-" * 65)
    for i, threads in enumerate(thread_counts):
        print(f"{threads:^12d} | {avg_times[i]:^13.2f} | {speedup[i]:^13.2f} | {efficiency[i]:^10.2f}")

def main():
    if len(sys.argv) != 2:
        print("Użycie: python script.py <plik_wyników>")
        sys.exit(1)
        
    input_file = sys.argv[1]
    results = parse_results(input_file)
    create_performance_graphs(results)

if __name__ == "__main__":
    main()