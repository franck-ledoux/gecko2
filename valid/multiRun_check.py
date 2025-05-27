import os
import sys
import subprocess
import time
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

# Main logic for running and analyzing
def run_analysis(exe, input_dir, default_params, output_dir):
    os.makedirs(output_dir, exist_ok=True)
    output_file_path = os.path.join(output_dir, "multiRun_save_results.txt")
    input_files = list_vtk_files(input_dir)

    if not input_files:
        print("No acceptable files in the specified directory.")
        return

    total_start_time = time.time()
    results_summary = {}

    with open(output_file_path, "w") as of:
        for f in input_files:
            input_path = os.path.join(input_dir, f + ".vtk")
            params = default_params
            if os.path.isfile(os.path.join(input_dir, f + ".json")):
                params = os.path.join(input_dir, f + ".json")

            wins = draws = losses = errors = 0
            times = []

            for i in range(10):
                run_start = time.time()
                r = run_mcts_program(exe, params, input_path)
                run_duration = time.time() - run_start
                times.append(run_duration)

                if r == 0:
                    draws += 1
                elif r == 1:
                    losses += 1
                elif r == 2:
                    wins += 1
                else:
                    errors += 1

            avg_time = sum(times) / len(times)
            max_time = max(times)
            min_time = min(times)

            results_summary[f] = {
                "wins": wins,
                "losses": losses,
                "draws": draws,
                "errors": errors,
                "avg_time": avg_time,
                "max_time": max_time,
                "min_time": min_time
            }

            of.write(f"{f} - W: {wins}, L: {losses}, D: {draws}, E: {errors}, "
                     f"Avg: {avg_time:.2f}s, Max: {max_time:.2f}s, Min: {min_time:.2f}s\n")
            of.flush()

        total_duration = time.time() - total_start_time
        of.write(f"\nTotal script duration: {total_duration:.2f}s\n")

    return results_summary, output_dir

# Visualization
def plot_results(results_summary, output_dir):
    forms = list(results_summary.keys())
    wins = [results_summary[f]["wins"] for f in forms]
    losses = [results_summary[f]["losses"] for f in forms]
    draws = [results_summary[f]["draws"] for f in forms]
    errors = [results_summary[f]["errors"] for f in forms]
    avg_times = [results_summary[f]["avg_time"] for f in forms]

    x = range(len(forms))

    plt.figure(figsize=(14, 6))
    plt.bar(x, wins, label='Wins', color='green')
    plt.bar(x, losses, bottom=wins, label='Losses', color='red')
    plt.bar(x, draws, bottom=[i+j for i,j in zip(wins, losses)], label='Draws', color='orange')
    plt.bar(x, errors, bottom=[i+j+k for i,j,k in zip(wins, losses, draws)], label='Errors', color='grey')
    plt.xticks(x, forms, rotation='vertical')
    plt.ylabel('Run Count')
    plt.title('MCTS Outcome per Shape')
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, "results_bar_chart.png"))
    plt.close()

    plt.figure(figsize=(14, 6))
    plt.plot(forms, avg_times, marker='o', linestyle='-', color='blue', label='Average Time (s)')
    plt.xticks(rotation='vertical')
    plt.ylabel('Time (s)')
    plt.title('Average Execution Time per Shape')
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, "average_times.png"))
    plt.close()

# Entry point
def main():
    if len(sys.argv) != 5:
        print("Usage: ./check.py <mcts program> <test case directory> <param.json> <out directory>")
        return

    exe = os.path.join(os.getcwd(), sys.argv[1])
    input_dir = os.path.join(os.getcwd(), sys.argv[2])
    default_params = os.path.join(os.getcwd(), sys.argv[3])
    output_dir = os.path.join(os.getcwd(), sys.argv[4])

    results_summary, output_dir = run_analysis(exe, input_dir, default_params, output_dir)
    plot_results(results_summary, output_dir)

main()