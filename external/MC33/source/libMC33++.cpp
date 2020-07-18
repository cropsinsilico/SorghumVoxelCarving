/*
	File: libMC33++.cpp
	Programmed by: David Vega
	August 2019
	Only this file must be included in a project to compile the MC33 library
*/

#define compiling_libMC33
/********************************CUSTOMIZING**********************************/
//The following line can be only changed before compiling the library:
//#define MC_Normal_neg // the front and back surfaces are exchanged.
/*****************************************************************************/

#include "../MC33.h"

float invSqrt(float x);

#ifndef GRD_orthogonal
void _multA_bf(const double (*)[3], float *, float *, int);
extern void (*mult_Abf)(const double (*)[3], float *, float *, int);
#endif

#include "grid3d.cpp"
#include "surface.cpp"
#include "MC33.cpp"

