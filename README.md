# gecko2
Version de travail pour la thèse de Paul. 

# Comment compiler

Ce projet repose sur l'utilisation de **CMake** pour la compilation et **Catch2** pour les tests.

Toutes les dépendances nécessaires au projet à l'exception de **CGAL** et ses dépendances sont fournies dans le 
sous-répertoire `external`. 

*Remarque :* Le projet a été compilé et exécuté sous Ubuntu et MacOS.
Dans les deux cas, la version 6.0.1 de CGAL est utilisée.
- **Ubuntu** - Installation manuelle et ajout au `CMAKE_PREFIX_PATH` des répertoires
d'installation de **CGAL** et **Boost**. Je n'utilise pas `GMP` en spécifiant `-DCGAL_DISABLE_GMP=True`. 
- **MacOS** - Installation avec *Homebrew*: `brew install cgal`. L'installation se faisant alors dans des répertoires 
usuels, il n'est pas nécessaire de les rajouter au `CMAKE_PREFIX_PATH`.

# Exécution 

## Lancement des tests 
Tous les tests Catch2 peuvent être exécutés conjointement en utilisant `CTest`. C'est détecté automatiquement
dans l'IDE **CLion** que nous utilisons.

Pour une utilisation basique en ligne de commandes, on peut valider que les tests
fonctionnent comme suit :

```shell
git clone git@github.com:franck-ledoux/gecko2.git
mkdir build
cd build
cmake ../gecko2
make 
make test
```
Vous devez obtenir 64 tests passés avec succès.

## Programme principal

Le programme `gecko2` est produit à partir de la fonction principale `main` disponible
dans le fichier [main_blocker.cpp](main_blocker.cpp) présent à la racine du projet.

Pour exécuter ce programme, il faut lui passer en paramètres :
- Un fichier au format `json` contenant les informations nécessaires au MCTS
- Un fichier au format `vtk` qui contienne un maillage tétraédrique de la forme géométrique
à discrétiser.
- Un nom de fichier de sortie.

Vous pouvez par exemple tester les fichiers présents dans `gecko2/test_data` comme suit :

```shell   
.gecko2 ../gecko2/test_data/params.json ..gecko2/test_data/AxisAlign/cube_minus_corner.vtk  result
```

# Test systèmes en Python

Le répertoire `valid` contient un unique script Python qui permet de lancer une version du programme
`gecko2`sur un ensemble de modèles situés dans le même répertoire. Il fonctionne comme suit sous réserve que vous
ayez **Python3** installé (pas d'environnement spécifique pour l'instant) :

```shell
 ./valid/check.py <gecko2_path> <input_dir> <default_params.json> <output_dir>
```
Les paramètres suivants sont utilisés :
1. `gecko2_path` est le chemin relatif vers une version du programme `gecko2`. Dans le cadre de l'IDE CLion, ce sera
usuellement `./cmake-build-release/gecko2` ou `./cmake-build-debug/gecko2`.
2. `ìnput_dir` est le chemin relatif vers le répertoire contenant l'ensemble des modèles sur lesquels vous voulez
exécuter `gecko2_path`. Pour rappel un modèle est un fichier au format `vtk` conternant une discrétisation tétraédrique
du modèle géométrique.
3. `default_params.json` est le fichier de paramètre utilisé pour configurer l'exécution du MCTS interne à 
`gecko2_path`. Si pour un modèle `M.vtk` présent dans `ìnput_dir`, on veut définir des paramètres spécifiques, il suffit 
de positionner le fichier de paramètres `model.json` dans  `ìnput_dir`.
4. `output_dir` est le chemin relatif vers le répertoire contenant l'ensemble des sorties produites par chaque modèle.
Pour chaque modèle `M` un répertoire `output_dir`sera créé et contiendra les fichiers de sortie de l'exécution 
de `gecko2_path`.

A titre d'exemple, vous pouvez exécuter à la racine du projet la ligne de commande suivante si vous avez compilé le projet
en mode *Release* à l'aide de l'IDE CLion :
```shell
./valid/check.py cmake-build-release/gecko2 test_data/AxisAlign test_data/params.json OUT_TESTS
```
Au fur et à mesure de l'exécution du programme `cmake-build-release/gecko2` sur les modèles du répertoire
`test_data/AxisAlign`, vous obtiendrez les résultats suivants. Chaque ligne indique le résultat obtenu sur le modèle 
sous la forme `<W|L|D|E>. nom_du_modele [(code d'erreur si E)]`. La lettre `W` indique une victoire,  la lettre `L` 
indique une défaite, la lettre `D` indique une égalité, et la lettre `E` indique que le programme 
`cmake-build-release/gecko2` a quitté en erreur avec le code d'erreur ajouté à la fin.

```
 E.  cross_3D (-11)
 W.  cube
 W.  cube_minus_corner
 D.  cube_minus_edge
 W.  cube_minus_opp_edges
 D.  cube_minus_two_edges
 E.  cube_with_holes (-11)
 E.  cube_with_notch (-11)
 E.  cube_without_two_cubes (-11)
 E.  encoches (-11)
 D.  hole
 E.  polycube (-11)
 E.  polycubeRefine (-11)
 W.  simple_cube
 D.  T_shape
 E.  teeth (-11)
 W.  tet_in_box
 E.  triangulated_quad (-6)
 D.  U_shape
 E.  XXYZ (-11)
 W.  XYZ
-----------------------------------------------------------------------
(total, win, lost, draw, errors) = (21, 6, 0, 5, 10)
-----------------------------------------------------------------------
```