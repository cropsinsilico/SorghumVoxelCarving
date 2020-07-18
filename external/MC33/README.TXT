Feel free to use the MC33++ library.
********************************************************************************

INFO:
This library is a C++ version based on the MC33 library of the paper:
Vega, D., Abache, J., Coll, D., A Fast and Memory-Saving Marching Cubes 33
implementation with the correct interior test, Journal of Computer Graphics
Techniques (JCGT), vol. 8, no. 3, 1–18, 2019.

The MC33++ library is an open source software. The distribution and use rights
are under the terms of the MIT license, described in the file "LICENSE.txt".
********************************************************************************

FILES:

include/MC33.h (header file)
source/MC33.cpp (MC33 class code)
source/grid3D.cpp (grid3D class code)
source/surface.cpp (surface class code)
source/MC33_LookUpTable.h (Triangulation pattern for each MC33 case)
source/libMC33++.cpp (library source file)
Makefile (Linux or MinGW/msys GCC makefile)
example/TestMC33.cpp (Example of use. FLTK library is required)
example/makefileMinGW-w64.mak (MinGW/msys GCC makefile)
example/makefiledebian.mak (Debian GCC makefile)

The folder visualc contains the solution and project files to compile the
library with Visual Studio 2017 Community.

********************************************************************************

CUSTOMIZING:

There are 3 options that can be modified before compiling the library. You can
do it by editing the MC33.h and MC33.cpp files:

1) To change the data type of the grid (the default value is float) define
size_type_GRD and/or integer_GRD. For example:

#define size_type_GRD 8 // the data type is double

#define integer_GRD
#define size_type_GRD 4 // the data type is unsigned int

#define integer_GRD
#define size_type_GRD 2 // the data type is unsigned short int

#define integer_GRD
#define size_type_GRD 1 // the data type is unsigned char

2) If you do not use inclined grids, you can define GRD_orthogonal:
#define GRD_orthogonal

3) If you need to exchange the front and back surfaces, define MC_Normal_neg:
#define MC_Normal_neg

********************************************************************************

INSTALLING:

You can compile the libMC33++.cpp file as a static c++ library and copy the compiled
library from the local lib directory to the compiler lib directory. Copy the MC33.h
file from the local include directory to the compiler include directory.
Include the header file in your code:
#include <MC33.h>
and put in the linker options of your makefile: -lMC33++

Or you can inlude the files MC33.cpp, grid3D.cpp and surface.cpp in your code:
#include "..Path ../source/MC33.cpp"
#include "..Path ../source/grid3D.cpp"
#include "..Path ../source/surface.cpp"

COMPILING THE EXAMPLE:

In Debian terminal window, write:
make -f makefiledebian.mak

Or in a msys2 MinGW64 Shell (Windows), write:
make -f makefileMinGW-w64.mak

For any operating system you also can use the fltk-config script:
path/fltk-1.X.Y/fltk-config --compile TestMC33.cpp

The makefiles use the -Ofast optimization option and the script uses -O3
********************************************************************************

USAGE:

1) create a grid3D object, and read a data file (use the member functions read_grd,
read_grd_binary, read_scanfiles, read_raw_file or read_dat_file):

grid3D G;
G.read_dat_file("filename.dat");

// or as a pointer

grid3D *Z = new grid3D;
Z->read_dat_file("filename.dat");

2) create a MC33 object and assign it the grid3D

MC33 MC;
MC.set_grid3d(&G);


3) Calculate the isosurface(s) with the MC33 algorithm:

surface *S = MC.calculate_isosurface(isovalue);

See MC33.h file for the use of other functions.

********************************************************************************

To display the surface with OpenGl use the draw() member function of surface class:

S->draw();

You can modify the function codes surface::draw() and surface::drawdraft() in the
MC33.h file. You can also add functions to read other formats (from 3D grid data
files) to the grid3D class in this file.

********************************************************************************


mail to: dvega@uc.edu.ve
