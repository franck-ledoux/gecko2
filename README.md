# gecko2
Version de travail pour la thèse de Paul. 

# Comment compiler

Ce projet repose sur l'utilisation de **CMake** pour la compilation et **Catch2** pour les tests.

Toutes les dépendances nécessaires au projet à l'exception de **CGAL** et ses dépendances sont fournies dans le 
sous-répertoire `external`. 

*Remarque :* Le projet a été compilé et exécuté sous Ubuntu et MacOS.
Dans les deux cas, la version 6.0.1 de CGAL est utilisée.
- **Ubuntu** - Installation manuelle et ajout au `CMAKE_PREFIX_PATH` des répertoires
d'installation de **CGAL** et **Boost**.
- **MacOS** - Installation avec *Homebrew*: `brew install cgal`. L'installation se faisant alors dans des répertoires usuels, il n'est pas 
- nécessaire de les rajouter au  `CMAKE_PREFIX_PATH`.

# Exécution 

## Lancement des tests 
Tous les tests catch2 peuvent être exécutés conjointement en utilisant 'CTest'. C'est détecté automatiquement
dans l'IDE *CLION* que nous utilisons.

Pour une utilisation baique en ligne de commandes, on peut valider que les tests
fonctionnent comme suit:

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

Le programme `gecko2` est produit à partir de la fonction prinicpale `main` présente
dans le fichier [main_blocker.cpp](main_blocker.cpp) présent à la racine du projet.

Pour exécuter ce programme, il faut lui passer en paramètres :
- Un fichier au format `json` contenant les informations nécessaires au MCTS
- Un fichier au format `vtk` qui contienne un maillage tétraédrique de la forme géométrique
à discrétiser.
- Un nom de fichier de sortie.

Vous pouvez par exemple tester les fichiers présents dans `gecko2/test_data` comme suit:

```shell
./gecko2 ../gecko2/test_data/params.json ../gecko2/test_data/cube_minus_corner.vtk out_blocks.vtk
```