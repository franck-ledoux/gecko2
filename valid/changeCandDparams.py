#!/usr/bin/env python3
import os
import sys
import subprocess
import time
import json
import csv
from statistics import mean

def run_mcts_program(executable_path, param_file, input_file):
    try:
        result = subprocess.run(
            [executable_path, param_file, input_file, "result"],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        return result.returncode
    except subprocess.CalledProcessError as e:
        print(f"Execution error: {e}")
    except FileNotFoundError:
        print(f"The program {executable_path} does not exist!")

def list_vtk_files(directory):
    if not os.path.isdir(directory):
        print(f"The directory {directory} does not exist.")
        return []
    return sorted([
        os.path.splitext(f)[0] for f in os.listdir(directory)
        if f.endswith(".vtk")
    ])

def modify_json_param(base_json_path, output_json_path, C_value, D_value):
    with open(base_json_path, 'r') as f:
        data = json.load(f)
    data['uct_C'] = C_value
    data['utc_D'] = D_value
    with open(output_json_path, 'w') as f:
        json.dump(data, f, indent=2)

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: ./check.py <mcts program> <test case directory> <param.json> <out directory>")
        sys.exit(1)

    exe = os.path.abspath(sys.argv[1])
    input_dir = os.path.abspath(sys.argv[2])
    base_params = os.path.abspath(sys.argv[3])
    output_dir = os.path.abspath(sys.argv[4])
    os.makedirs(output_dir, exist_ok=True)

    # Create/open the global CSV summary file
    csv_path = os.path.join(output_dir, "results_summary_CandD.csv")
    csv_file = open(csv_path, "w", newline="")
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(["Shape", "C", "D", "Wins", "Losses", "Draws", "Errors", "AvgTime (s)"])


    input_files = []
    if os.path.isdir(input_dir):
     input_files = list_vtk_files(input_dir)
    elif input_dir.endswith('.vtk') and os.path.isfile(input_dir):
        file_name = os.path.splitext(os.path.basename(input_dir))[0]
        input_files = [file_name]
        input_dir = os.path.dirname(input_dir)
    else:
        print(f"Invalid input path: {input_dir}")
        sys.exit(1)
    #output_file_path = os.path.join(output_dir, "saveResultsCandD.txt")
    # Create one result file per shape
    def get_output_file_path(form_name):
        return os.path.join(output_dir, f"saveResultsCandD_{form_name}.txt")

    # Param ranges to test
    C_values = [0.5, 1.42, 3.0, 6.0]
    D_values = [100, 1000, 100000, 1000000]

    for file_name in input_files:
        output_file_path = get_output_file_path(file_name)
        with open(output_file_path, "w") as log:
            for file_name in input_files:
                input_path = os.path.join(input_dir, file_name + ".vtk")

                for C in C_values:
                    for D in D_values:
                        print(f"\n=== Testing {file_name} | C={C}, D={D} ===")
                        log.write(f"\n=== {file_name} | C={C}, D={D} ===\n")

                        # Prepare param file
                        modified_param_path = os.path.join(output_dir, f"params_C{C}_D{D}.json")
                        modify_json_param(base_params, modified_param_path, C, D)


                        results = []
                        durations = []

                        for i in range(10):
                            run_start = time.time()
                            result_code = run_mcts_program(exe, modified_param_path, input_path)
                            run_time = time.time() - run_start
                            durations.append(run_time)

                            if result_code == 2:
                                results.append("W")
                            elif result_code == 1:
                                results.append("L")
                            elif result_code == 0:
                                results.append("D")
                            else:
                                results.append("E")

                            print(f"Run {i+1}: {results[-1]} - {run_time:.2f}s")
                            log.write(f"Run {i+1}: {results[-1]} - {run_time:.2f}s\n")

                        # Summary
                        summary = {
                            'W': results.count("W"),
                            'L': results.count("L"),
                            'D': results.count("D"),
                            'E': results.count("E"),
                            'avg_time': mean(durations)
                        }

                        log.write(f"Summary: W={summary['W']}, L={summary['L']}, D={summary['D']}, E={summary['E']}, AvgTime={summary['avg_time']:.2f}s\n")
                        log.flush()
                        csv_writer.writerow([
                            file_name,
                            C,
                            D,
                            summary['W'],
                            summary['L'],
                            summary['D'],
                            summary['E'],
                            round(summary['avg_time'], 2)
                        ])


csv_file.close()