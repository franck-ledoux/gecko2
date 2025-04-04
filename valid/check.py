#!/usr/bin/env python3
import os
import sys
import subprocess

def run_mcts_program(executable_path, param, input):
    try:
        # Lancer le programme C++
        result = subprocess.run([executable_path, param, input, "result"], check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return result.returncode
    except subprocess.CalledProcessError as e:
        print(f"Eexcution error during program: {e}")
    except FileNotFoundError:
        print(f"The program {executable_path} does not exist!")


def list_vtk_files(dir):
    # Vérifie si le répertoire existe
    if not os.path.isdir(dir):
        print(f"The directory {dir} does not exist.")
        return []
    # Liste tous les fichiers dans le répertoire
    try:
        files = os.listdir(dir)
    except PermissionError:
        print(f"Unallowed access to directory {dir}.")
        return []

    # Filtre les fichiers pour ne garder que ceux avec l'extension .vtk
    vtk_files = [f for f in files if f.endswith('.vtk')]

    # Extrait les suffixes des fichiers .vtk
    file_names = [os.path.splitext(vtk_file)[0] for vtk_file in vtk_files]

    # Retourne la liste des suffixes triée alphabétiquement
    return sorted(file_names, key=lambda s: s.lower())



if __name__ == "__main__":

    if len(sys.argv) != 5:
        print("Usage: ./check.py <mcts program> <param.json> <test case directory> <out directory>")
    else:
        nb_cases = 0
        nb_wins = 0
        nb_errors = 0
        nb_lost = 0
        nb_draws = 0
        exe = os.getcwd()+'/'+sys.argv[1]
        input_dir = os.getcwd()+'/'+sys.argv[2]
        default_params =  os.getcwd()+'/'+sys.argv[3]
        output_dir = os.getcwd()+'/'+sys.argv[4]

        os.makedirs(output_dir, exist_ok=True)
        input_files= list_vtk_files(input_dir)
        if input_files:
            for f in input_files:
                os.chdir(output_dir)
                nb_cases += 1
                input_path = input_dir + "/" + f + ".vtk"
                # We initialize the parameter file with default value
                params = default_params
                # We check if a specific parameter file is provided
                if os.path.isfile(input_dir + "/" + f + ".json"):
                    params = input_dir + "/" + f + ".json"
                specific_output_dir = output_dir + "/" + f
                os.makedirs(specific_output_dir, exist_ok=True)
                os.chdir(specific_output_dir)
                r = run_mcts_program(exe,params,input_path)
                if r == 0:
                    nb_draws += 1
                    print(f"\033[38;5;214m D. \033[0m {f}")
                elif r == 1:
                    nb_lost += 1
                    print(f"\033[91m L. \033[0m {f}")
                elif r == 2:
                    nb_wins += 1
                    print(f"\033[92m W. \033[0m {f}")
                else:
                    nb_errors += 1
                    print(f"\033[91m E. \033[0m {f} ({r})")
            print("-----------------------------------------------------------------------")
            print(f"(total, win, lost, draw, errors) = ({nb_cases}, {nb_wins}, {nb_lost}, {nb_draws}, {nb_errors})")
            print("-----------------------------------------------------------------------")
        else:
            print("No acceptable files in the specified directory.")