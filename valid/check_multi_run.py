#!/usr/bin/env python3
#Pour le run ./check_multi_run.py <mcts_program> <test_case_dir> <param.json> <out_dir>
import os
import sys
import subprocess
import time
import statistics
import matplotlib.pyplot as plt

def run_mcts_program(executable_path, param, input):
    try:
        result = subprocess.run(
            [executable_path, param, input, "result"],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        return result.returncode
    except subprocess.CalledProcessError as e:
        print(f"Execution error during program: {e}")
    except FileNotFoundError:
        print(f"The program {executable_path} does not exist!")

def list_vtk_files(dir):
    if not os.path.isdir(dir):
        print(f"The directory {dir} does not exist.")
        return []
    try:
        files = os.listdir(dir)
    except PermissionError:
        print(f"Unallowed access to directory {dir}.")
        return []

    vtk_files = [f for f in files if f.endswith('.vtk')]
    file_names = [os.path.splitext(vtk_file)[0] for vtk_file in vtk_files]
    return sorted(file_names, key=lambda s: s.lower())

if __name__ == "__main__":

    if len(sys.argv) != 5:
        print("Usage: ./check.py <mcts program> <test case directory> <param.json> <out directory>")
    else:
        exe = os.path.join(os.getcwd(), sys.argv[1])
        input_dir = os.path.join(os.getcwd(), sys.argv[2])
        default_params = os.path.join(os.getcwd(), sys.argv[3])
        output_dir = os.path.join(os.getcwd(), sys.argv[4])

        os.makedirs(output_dir, exist_ok=True)
        output_file_path = os.path.join(output_dir, "multiRun_save_results.txt")

        input_files = list_vtk_files(input_dir)
        all_stats = {}

        if input_files:
            total_start_time = time.time()

            with open(output_file_path, "w") as of:
                for f in input_files:
                    stats = {"win": 0, "lost": 0, "draw": 0, "error": 0, "times": []}
                    input_path = os.path.join(input_dir, f + ".vtk")
                    params = default_params
                    if os.path.isfile(os.path.join(input_dir, f + ".json")):
                        params = os.path.join(input_dir, f + ".json")

                    specific_output_dir = os.path.join(output_dir, f)
                    os.makedirs(specific_output_dir, exist_ok=True)

                    for _ in range(10):
                        os.chdir(specific_output_dir)
                        start_time = time.time()
                        r = run_mcts_program(exe, params, input_path)
                        duration = time.time() - start_time
                        stats["times"].append(duration)

                        if r == 0:
                            stats["draw"] += 1
                        elif r == 1:
                            stats["lost"] += 1
                        elif r == 2:
                            stats["win"] += 1
                        else:
                            stats["error"] += 1

                    avg_time = statistics.mean(stats["times"])
                    min_time = min(stats["times"])
                    max_time = max(stats["times"])

                    result_line = (
                        f"{f}: win={stats['win']}, lost={stats['lost']}, draw={stats['draw']}, error={stats['error']}, "
                        f"avg_time={avg_time:.2f}s, min_time={min_time:.2f}s, max_time={max_time:.2f}s\n"
                    )
                    print(result_line.strip())
                    of.write(result_line)
                    of.flush()

                    all_stats[f] = stats

                total_duration = time.time() - total_start_time
                of.write(f"Total execution time: {total_duration:.2f}s\n")

            # Plot results
            labels = list(all_stats.keys())
            wins = [all_stats[l]["win"] for l in labels]
            losses = [all_stats[l]["lost"] for l in labels]
            draws = [all_stats[l]["draw"] for l in labels]
            errors = [all_stats[l]["error"] for l in labels]
            avg_times = [statistics.mean(all_stats[l]["times"]) for l in labels]

            plt.figure(figsize=(12, 6))
            plt.bar(labels, wins, label="Wins", color="green")
            plt.bar(labels, losses, bottom=wins, label="Losses", color="red")
            plt.bar(labels, draws, bottom=[i+j for i,j in zip(wins, losses)], label="Draws", color="orange")
            plt.bar(labels, errors, bottom=[i+j+k for i,j,k in zip(wins, losses, draws)], label="Errors", color="gray")
            plt.xlabel("Shape")
            plt.ylabel("Count")
            plt.title("Results per Shape (10 runs each)")
            plt.xticks(rotation=45, ha="right")
            plt.legend()
            plt.tight_layout()
            plt.savefig(os.path.join(output_dir, "results_bar_chart.png"))

            plt.figure(figsize=(12, 6))
            plt.plot(labels, avg_times, marker='o')
            plt.xlabel("Shape")
            plt.ylabel("Average Execution Time (s)")
            plt.title("Average Time per Shape")
            plt.xticks(rotation=45, ha="right")
            plt.tight_layout()
            plt.savefig(os.path.join(output_dir, "average_times.png"))
        else:
            print("No acceptable files in the specified directory.")
