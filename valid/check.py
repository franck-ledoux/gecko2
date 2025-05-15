#!/usr/bin/env python3
import os
import sys
import subprocess
import time

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
        print("Usage: ./check.py <mcts program> <param.json> <test case directory> <out directory>")
    else:
        nb_cases = nb_wins = nb_errors = nb_lost = nb_draws = 0
        exe = os.path.join(os.getcwd(), sys.argv[1])
        input_dir = os.path.join(os.getcwd(), sys.argv[2])
        default_params = os.path.join(os.getcwd(), sys.argv[3])
        output_dir = os.path.join(os.getcwd(), sys.argv[4])

        os.makedirs(output_dir, exist_ok=True)
        output_file_path = os.path.join(output_dir, "saveResults.txt")

        input_files = list_vtk_files(input_dir)

        if input_files:
            total_start_time = time.time()

            with open(output_file_path, "w") as of:
                for f in input_files:
                    start_time = time.time()
                    nb_cases += 1
                    input_path = os.path.join(input_dir, f + ".vtk")
                    params = default_params
                    if os.path.isfile(os.path.join(input_dir, f + ".json")):
                        params = os.path.join(input_dir, f + ".json")

                    specific_output_dir = os.path.join(output_dir, f)
                    os.makedirs(specific_output_dir, exist_ok=True)
                    os.chdir(specific_output_dir)

                    r = run_mcts_program(exe, params, input_path)
                    duration = time.time() - start_time
                    duration_str = f"{duration:.2f}s"

                    if r == 0:
                        nb_draws += 1
                        print(f"\033[38;5;214m D. \033[0m {f} - {duration_str}")
                        of.write(f"D. {f} - {duration_str}\n")
                    elif r == 1:
                        nb_lost += 1
                        print(f"\033[91m L. \033[0m {f} - {duration_str}")
                        of.write(f"L. {f} - {duration_str}\n")
                        of.flush()
                    elif r == 2:
                        nb_wins += 1
                        print(f"\033[92m W. \033[0m {f} - {duration_str}")
                        of.write(f"W. {f} - {duration_str}\n")
                        of.flush()
                    else:
                        nb_errors += 1
                        print(f"\033[91m E. \033[0m {f} ({r}) - {duration_str}")
                        of.write(f"E. {f} ({r}) - {duration_str}\n")
                        of.flush()

                total_duration = time.time() - total_start_time
                total_str = f"{total_duration:.2f}s"
                summary = (
                    "-----------------------------------------------------------------------\n"
                    f"(total, win, lost, draw, errors) = ({nb_cases}, {nb_wins}, {nb_lost}, {nb_draws}, {nb_errors})\n"
                    f"Total execution time: {total_str}\n"
                    "-----------------------------------------------------------------------\n"
                )
                print(summary)
                of.write(summary)
        else:
            print("No acceptable files in the specified directory.")