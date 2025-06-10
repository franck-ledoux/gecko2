#!/usr/bin/env python3
import os
import sys
import subprocess
import time
import json
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

def list_vtk_files(directory):
    if not os.path.isdir(directory):
        print(f"The directory {directory} does not exist.")
        return []
    try:
        files = os.listdir(directory)
    except PermissionError:
        print(f"Unallowed access to directory {directory}.")
        return []

    return sorted([os.path.splitext(f)[0] for f in files if f.endswith('.vtk')], key=lambda s: s.lower())

def load_filter_list(json_path):
    if not os.path.isfile(json_path):
        print(f"No filter file found at {json_path}, running on all shapes.")
        return None
    try:
        with open(json_path, "r") as jf:
            return json.load(jf)
    except Exception as e:
        print(f"Could not read JSON filter list: {e}")
        return None

if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("Usage: ./check_multi_run_v2.py <program> <testcase_dir> <param.json> <out_dir> <filter.json>")
        sys.exit(1)

    exe = os.path.abspath(sys.argv[1])
    input_dir = os.path.abspath(sys.argv[2])
    default_params = os.path.abspath(sys.argv[3])
    output_dir = os.path.abspath(sys.argv[4])
    filter_json = os.path.abspath(sys.argv[5])
    runs_per_shape = 10

    os.makedirs(output_dir, exist_ok=True)
    output_file_path = os.path.join(output_dir, "multi_run_results.txt")

    input_files = list_vtk_files(input_dir)
    filter_list = load_filter_list(filter_json)
    if filter_list:
        input_files = [f for f in input_files if f in filter_list]

    if not input_files:
        print("No matching files to process.")
        sys.exit(0)

    results = {}
    total_start = time.time()

    with open(output_file_path, "w") as of:
        for f in input_files:
            shape_stats = {'W': 0, 'L': 0, 'D': 0, 'E': 0, 'times': []}

            vtk_path = os.path.join(input_dir, f + ".vtk")
            param_path = os.path.join(input_dir, f + ".json") if os.path.isfile(os.path.join(input_dir, f + ".json")) else default_params

            shape_output = os.path.join(output_dir, f)
            os.makedirs(shape_output, exist_ok=True)

            for i in range(runs_per_shape):
                os.chdir(shape_output)
                start = time.time()
                code = run_mcts_program(exe, param_path, vtk_path)
                duration = time.time() - start
                shape_stats['times'].append(duration)

                if code == 0:
                    shape_stats['D'] += 1
                elif code == 1:
                    shape_stats['L'] += 1
                elif code == 2:
                    shape_stats['W'] += 1
                else:
                    shape_stats['E'] += 1

            avg_time = sum(shape_stats['times']) / runs_per_shape
            min_time = min(shape_stats['times'])
            max_time = max(shape_stats['times'])

            results[f] = shape_stats

            line = f"{f}: W={shape_stats['W']}, L={shape_stats['L']}, D={shape_stats['D']}, E={shape_stats['E']}, " \
                   f"Avg={avg_time:.2f}s, Min={min_time:.2f}s, Max={max_time:.2f}s"
            print(line)
            of.write(line + "\n")

        total_time = time.time() - total_start
        of.write(f"\nTotal execution time: {total_time:.2f}s\n")

    # --- Plotting ---
    shapes = list(results.keys())
    Ws = [results[s]['W'] for s in shapes]
    Ls = [results[s]['L'] for s in shapes]
    Ds = [results[s]['D'] for s in shapes]

    x = range(len(shapes))
    plt.figure(figsize=(12, 6))
    plt.bar(x, Ws, label="Wins", color="green")
    plt.bar(x, Ls, bottom=Ws, label="Losses", color="red")
    bottom_draws = [Ws[i] + Ls[i] for i in x]
    plt.bar(x, Ds, bottom=bottom_draws, label="Draws", color="orange")
    plt.xticks(x, shapes, rotation=45, ha="right")
    plt.ylabel("Counts (over 10 runs)")
    plt.title("MCTS Results per Shape")
    plt.legend()
    plt.tight_layout()

    plot_path = os.path.join(output_dir, "multi_run_results_plot.png")
    plt.savefig(plot_path)
    print(f"\nPlot saved to {plot_path}")


#./check_multi_run_v2.py ./my_program ./shapes ./params.json ./results ./filter.json
#./valid/multiRun_check_list_Shapes.py cmake-build-release/gecko2 test_data/AxisAlign test_data/params.json OUT_TESTS test_data/AxisAlign/shortDataTest.json