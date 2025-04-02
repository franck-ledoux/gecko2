// Gmsh project created on Sun Sep  1 17:52:35 2024
SetFactory("OpenCASCADE");
//+
Box(1) = {0, 0, 0, 1, 1, 1};
//+
Cylinder(2) = {1, 0, 0, -1, 0, 0, 0.5, 2*Pi};
//+
BooleanDifference{ Volume{1}; Delete; }{ Volume{2}; Delete; }
