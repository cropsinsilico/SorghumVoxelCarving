/*
	File: MC33.cpp
	Programmed by: David Vega - dvega@uc.edu.ve
	August 2019
	February 2020
*/


#ifndef MC33_cpp
#define MC33_cpp

#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include "MC33_LookUpTable.h"
#include "../MC33.h"

using namespace std;

#ifndef incDimTVN
#define incDimTVN (1<<9)
#endif

MC33::MC33() : F(0)
{
	memcpy(table, MC33_all_tables, sizeof(MC33_all_tables));
}

MC33::~MC33()
{
	clear_temp_isosurface();
}

void MC33::set_default_surface_color(unsigned char *color)
{
	DefaultColor = *(reinterpret_cast<int*>(color));
}

/******************************************************************
Vertices:           Faces:
    3 __________2        ___________
   /|          /|      /|          /|
  / |         / |     / |   2     / |
7/__________6/  |    /  |     4  /  |
|   |       |   |   |¯¯¯¯¯¯¯¯¯¯¯| 1 |     z
|   0_______|___1   | 3 |_______|___|     |
|  /        |  /    |  /  5     |  /      |____y
| /         | /     | /     0   | /      /
4/__________5/      |/__________|/      x


This function returns a vector with all six test face results (face[6]). Each
result value is 1 if the positive face vertices are joined, -1 if the negative
vertices are joined, and 0 (unchanged) if the test should not be applied. The
return value of this function is the the sum of all six results.*/
int MC33::face_tests(int *face, int i)
{
	if (i&0x80)//vertex 0
	{
		face[0] = ((i&0xCC) == 0x84? (v[0]*v[5] < v[1]*v[4]? -1: 1): 0);//0x84 = 10000100, vertices 0 and 5
		face[3] = ((i&0x99) == 0x81? (v[0]*v[7] < v[3]*v[4]? -1: 1): 0);//0x81 = 10000001, vertices 0 and 7
		face[4] = ((i&0xF0) == 0xA0? (v[0]*v[2] < v[1]*v[3]? -1: 1): 0);//0xA0 = 10100000, vertices 0 and 2
	}
	else
	{
		face[0] = ((i&0xCC) == 0x48? (v[0]*v[5] < v[1]*v[4]? 1: -1): 0);//0x48 = 01001000, vertices 1 and 4
		face[3] = ((i&0x99) == 0x18? (v[0]*v[7] < v[3]*v[4]? 1: -1): 0);//0x18 = 00011000, vertices 3 and 4
		face[4] = ((i&0xF0) == 0x50? (v[0]*v[2] < v[1]*v[3]? 1: -1): 0);//0x50 = 01010000, vertices 1 and 3
	}
	if (i&0x02)//vertex 6
	{
		face[1] = ((i&0x66) == 0x42? (v[1]*v[6] < v[2]*v[5]? -1: 1): 0);//0x42 = 01000010, vertices 1 and 6
		face[2] = ((i&0x33) == 0x12? (v[3]*v[6] < v[2]*v[7]? -1: 1): 0);//0x12 = 00010010, vertices 3 and 6
		face[5] = ((i&0x0F) == 0x0A? (v[4]*v[6] < v[5]*v[7]? -1: 1): 0);//0x0A = 00001010, vertices 4 and 6
	}
	else
	{
		face[1] = ((i&0x66) == 0x24? (v[1]*v[6] < v[2]*v[5]? 1: -1): 0);//0x24 = 00100100, vertices 2 and 5
		face[2] = ((i&0x33) == 0x21? (v[3]*v[6] < v[2]*v[7]? 1: -1): 0);//0x21 = 00100001, vertices 2 and 7
		face[5] = ((i&0x0F) == 0x05? (v[4]*v[6] < v[5]*v[7]? 1: -1): 0);//0x05 = 00000101, vertices 5 and 7
	}
	return face[0] + face[1] + face[2] + face[3] + face[4] + face[5];
}
/* Faster function for the face test, the test is applied to only one face
(int face). This function is only used for the cases 3 and 6 of MC33*/
int MC33::face_test1(int face)
{
	switch (face)
	{
	case 0:
		return (v[0]*v[5] < v[1]*v[4]? 0x48: 0x84);
	case 1:
		return (v[1]*v[6] < v[2]*v[5]? 0x24: 0x42);
	case 2:
		return (v[3]*v[6] < v[2]*v[7]? 0x21: 0x12);
	case 3:
		return (v[0]*v[7] < v[3]*v[4]? 0x18: 0x81);
	case 4:
		return (v[0]*v[2] < v[1]*v[3]? 0x50: 0xA0);
	default:
		return (v[4]*v[6] < v[5]*v[7]? 0x05: 0x0A);
	}
}
// signbit(float) is somewhat slower than __signbitf compiling with GCC ???
#ifdef __GNUC__
#define signbf __signbitf
#elif _MSC_VER
// signbit(float) is slower compiling with Visual C
#define signbf(x) (*(reinterpret_cast<unsigned int*>(&x))>>31)
#else
#define signbf signbit
#endif

/******************************************************************
Interior test function. If the test is positive, the function returns a value
different from 0. The integer i must be 0 to test if the vertices 0 and 6 are
joined. 1 for vertices 1 and 7, 2 for vertices 2 and 4, and 3 for 3 and 5.
For case 13, the integer flag13 must be 1, and the function returns 2 if one
of the vertices 0, 1, 2 or 3 is joined to the center point of the cube (case
13.5.2), returns 1 if one of the vertices 4, 5, 6 or 7 is joined to the
center point of the cube (case 13.5.2 too), and it returns 0 if the vertices
are not joined (case 13.5.1)*/
int MC33::interior_test(int i, int flag13)
{
	//Signs of cube vertices were changed to use signbit function in calc_isosurface
	//A0 = -v[0], B0 = -v[1], C0 = -v[2], D0 = -v[3]
	//A1 = -v[4], B1 = -v[5], C1 = -v[6], D1 = -v[7]
	//But the function still works
	float At = v[4] - v[0], Bt = v[5] - v[1],
				Ct = v[6] - v[2], Dt = v[7] - v[3];
	float t = At*Ct - Bt*Dt;//the "a" value.
	if (signbf(t))
	{
		if (i&0x01) return 0;
	}
	else
	{
		if (!(i&0x01) || t == 0.0f) return 0;
	}
	t = 0.5f*(v[3]*Bt + v[1]*Dt - v[2]*At - v[0]*Ct)/t;//t = -b/2a
	if (t <= 0.0f || t >= 1.0f)
		return 0;

	At = v[0] + At*t;
	Bt = v[1] + Bt*t;
	Ct = v[2] + Ct*t;
	Dt = v[3] + Dt*t;
	if (i&0x01)
	{
		if (At*Ct < Bt*Dt && signbf(Bt) == signbf(Dt))
			return (signbf(Bt) == signbf(v[i])) + flag13;
	}
	else
	{
		if (At*Ct > Bt*Dt && signbf(At) == signbf(Ct))
			return (signbf(At) == signbf(v[i])) + flag13;
	}
	return 0;
}

/******************************************************************
Assign memory for the vertex r[3], normal (r + 3)[3]. The return value is
the new vertex label.
*/
int MC33::store_point_normal(float *r)
{
	float t, *p;
	unsigned int nv = MC_S->nV++;
	if (!(nv&0x0FFF))
	{
		try
		{
			MC_S->V.resize(nv + 0x1000);
			MC_S->N.resize(nv + 0x1000);
			MC_S->color.resize(nv + 0x1000);
		}
		catch (...)
		{
			memoryfault = 1;
			MC_S->nV = (nv? 1: 0);
			return 0;
		}
	}
	MC_S->color[nv] = DefaultColor;
	p = MC_S->V[nv].v;
	for (int i = 0; i != 3; ++i)
		p[i] = r[i]*MC_D[i] + MC_O[i];
	r += 3; // now r points to normal coordinates
#ifndef GRD_orthogonal
	if (nonortho)
	{
		mult_Abf(_A,p,p,0);
		mult_Abf(A_,r,r,1);
	}
#endif
#ifndef MC_Normal_neg
//	t = 1.0f/sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
	t = invSqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
#else //MC_Normal_neg reverses the direction of the normal
//t = -1.0f/sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
	t = -invSqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
#endif
	p = MC_S->N[nv].v;
	for (int i = 0; i != 3; ++i)
		p[i] = t*r[i];
	return nv;
}

/******************************************************************
Store the triangle, an array of three vertex indices (integers).
*/
/*void MC33::store_triangle(int *t)
{
	int nt = MC_S->nT++;
	if (!(nt&0x0FFF))
		MC_S->T.resize(nt + 0x1000);
	memcpy(MC_S->T[nt].v,t,3*sizeof(int));
}*/

/******************************************************************
Auxiliary function that calculates the normal if a vertex
of the cube lies on the isosurface.
*/
int MC33::surfint(int x, int y, int z, float *r)
{
	r[0] = float(x); r[1] = float(y); r[2] = float(z);
	if (x == 0)
		r[3] = F[z][y][0] - F[z][y][1];
	else if (x == nx)
		r[3] = F[z][y][x - 1] - F[z][y][x];
	else
		r[3] = 0.5f*(F[z][y][x - 1] - F[z][y][x + 1]);
	if (y == 0)
		r[4] = F[z][0][x] - F[z][1][x];
	else if (y == ny)
		r[4] = F[z][y - 1][x] - F[z][y][x];
	else
		r[4] = 0.5f*(F[z][y - 1][x] - F[z][y + 1][x]);
	if (z == 0)
		r[5] = F[0][y][x] - F[1][y][x];
	else if (z == nz)
		r[5] = F[z - 1][y][x] - F[z][y][x];
	else
		r[5] = 0.5f*(F[z - 1][y][x] - F[z + 1][y][x]);
	return store_point_normal(r);
}

/******************************************************************
This function find the MC33 case (using the index i, and the face and interior
tests). The correct triangle pattern is obtained from the arrays contained in
the MC33_LookUpTable.h file. The necessary vertices (intersection points) are
also calculated here (using trilinear interpolation).
       _____2_____
     /|          /|
   11 |<-3     10 |
   /____6_____ /  1     z
  |   |       |   |     |
  |   |_____0_|___|     |____y
  7  /        5  /     /
  | 8         | 9     x
  |/____4_____|/

The temporary matrices: Ox, Oy, Nx and Ny, and vectors: OL and NL are filled
and used here.*/

void MC33::find_case(int x, int y, int z, int i)
{
	int p[13] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	union { // memory saving
		int f[6];//for the face tests
		float r[6];//for intercept and normal coordinates
	};
	int ti[3];//for vertex indices of a triangle
	float t;
	int c, k, m;
	const unsigned short int *pcase = table;
	c = pcase[i&0x80? i^0xFF: i];
	m = (i^c)&0x80;
	k = c&0x7F;
	switch (c>>8)//find the MC33 case
	{
	case 1://********************************************
		pcase += k + 127;
		break;
	case 2://********************************************
		pcase += k + 135;
		break;
	case 3://********************************************
		pcase += ((m? i: i^0xFF)&face_test1(k>>2)? 183 + (k<<1): 159 + k);
		break;
	case 4://********************************************
		pcase += (interior_test(k,0)? 239 + 6*k: 231 + (k<<1));
		break;
	case 5://********************************************
		pcase += k + 263;
		break;
	case 6://********************************************
		if ((m? i: i^0xFF)&face_test1(k%6))
			pcase += 575 + 5*k; //6.2
		else
			pcase += (interior_test(k/6,0)? 407 + 7*k: 335 + 3*k); //6.1
		break;
	case 7://********************************************
		switch (face_tests(f,(m? i: i^0xFF)))
		{
		case -3:
			pcase += 695 + 3*k; //7.1
			break;
		case -1: //7.2
			pcase += (f[4] + f[5] < 0? (f[0] + f[2] < 0? 759: 799): 719) + 5*k;
			break;
		case 1: //7.3
			pcase += (f[4] + f[5] < 0? 983: (f[0] + f[2] < 0? 839: 911)) + 9*k;
			break;
		default: //7.4
			pcase += (interior_test(k>>1,0)? 1095 + 9*k: 1055 + 5*k);
		}
		break;
	case 8://********************************************
		pcase += k + 1167;
		break;
	case 9://********************************************
		pcase += k + 1173;
		break;
	case 10://********************************************
		switch (face_tests(f,(m? i: i^0xFF)))
		{
		case -2:
			if (k&2? interior_test(0,0): interior_test(0,0)||interior_test(k? 1: 3,0))
				pcase += 1213 + (k<<3); //10.1.2
			else
				pcase += 1189 + (k<<2); //10.1.1
			break;
		case 0: //10.2
			pcase += (f[2 + k] < 0? 1261: 1285) + (k<<3);
			break;
		default:
			if (k&2? interior_test(1,0): interior_test(2,0)||interior_test(k? 3: 1,0))
				pcase += 1237 + (k<<3); //10.1.2
			else
				pcase += 1201 + (k<<2); //10.1.1
		}
		break;
	case 11://********************************************
		pcase += k + 1309;
		break;
	case 12://********************************************
		switch (face_tests(f,(m? i: i^0xFF)))
		{
		case -2: //12.1
			pcase += (interior_test((0xDA010C>>(k<<1))&3,0)? 1429 + (k<<3): 1333 + (k<<2));
			break;
		case 0: //12.2
			pcase += (f[k>>1] < 0? 1621: 1717) + (k<<3);
			break;
		default: //12.1
			pcase += (interior_test((0xA7B7E5>>(k<<1))&3,0)? 1525 + (k<<3): 1381 + (k<<2));
		}
		break;
	case 13://********************************************
		switch (abs(face_tests(f,(m? 90: 165))))
		{
		case 0:
			k = ((f[1] < 0)<<1)|(f[5] < 0);
			if (f[0]*f[1] == f[5]) //13.4
				pcase += 2133 + 12*k;
			else
				switch (interior_test(k,1))
				{
				case 0: //13.5.1
					pcase += 2181 + 6*k;
					break;
				case 1: //13.5.2
					pcase += 2245 + 10*k;
					break;
				default: //13.5.2
					pcase += 2205 + 10*k;
				}
			break;
		case 2: //13.3
			pcase += 1893 + 10*((f[0] < 0? f[2] > 0: 12 + (f[2] < 0)) + (f[1] < 0? f[3] < 0: 6 + (f[3] > 0)));
			if (f[4] > 0)
				pcase += 30;
			break;
		case 4: //13.2
			k = 21 + 11*f[0] + 4*f[1] + 3*f[2] + 2*f[3] + f[4];
			if (k >> 4)
				k -= (k&32? 20: 10);
			pcase += 1821 + 3*k;
			break;
		default: //13.1
			pcase += 1813;
			if (f[0] < 0)
				pcase += 4;
		}
		break;
	default://********************************************
		pcase += k + 2285;
	}
	while (i)
	{
		i = *(++pcase);
		k = -1;
		while (k != 2)
		{
			c = i&0x0F;
			if (p[c] < 0)
			{
				switch (c)//the vertices r[3] and normals (r + 3)[3] are calculated here
				{
				case 0:
					if (z || x)
						p[0] = Oy[y][x];
					else
					{
						if (v[0] == 0.0f)
						{
							p[0] = surfint(0,y,0,r);
							if (signbf(v[3])) p[3] = p[0];
							if (signbf(v[4])) p[8] = p[0];
						}
						else if (v[1] == 0.0f)
						{
							p[0] = surfint(0,y + 1,0,r);
							if (signbf(v[2])) NL[0] = p[1] = p[0];
							if (signbf(v[5])) Ox[y + 1][0] = p[9] = p[0];
						}
						else
						{
							t = v[0]/(v[0] - v[1]);
							r[0] = r[2] = 0.0f;
							r[1] = y + t;
							r[3] = (v[4] - v[0])*(1.0f - t) + (v[5] - v[1])*t;
							r[4] = v[1] - v[0];
							r[5] = (v[3] - v[0])*(1.0f - t) + (v[2] - v[1])*t;
							p[0] = store_point_normal(r);
						}
					}
					break;
				case 1:
					if (x)
						p[1] = NL[x];
					else
					{
						if (v[1] == 0.0f)
						{
							NL[0] = p[1] = surfint(0,y + 1,z,r);
							if (signbf(v[0])) p[0] = p[1];
							if (signbf(v[5]))
							{
								p[9] = p[1];
								if (z == 0) Ox[y + 1][0] = p[1];
							}
						}
						else if (v[2] == 0.0f)
						{
							NL[0] = p[1] = surfint(0,y + 1,z + 1,r);
							if (signbf(v[3])) Ny[y][0] = p[2] = p[1];
							if (signbf(v[6])) Nx[y + 1][0] = p[10] = p[1];
						}
						else
						{
							t = v[1]/(v[1] - v[2]);
							r[0] = 0.0f; r[1] = float(y + 1);
							r[2] = z + t;
							r[3] = (v[5] - v[1])*(1.0f - t) + (v[6] - v[2])*t;
							r[4] = (y + 1 < ny? 0.5f*((F[z][y][0] - F[z][y + 2][0])*(1.0f - t)
										+ (F[z + 1][y][0] - F[z + 1][y + 2][0])*t):
										(v[1] - v[0])*(1.0f - t) + (v[2] - v[3])*t);
							r[5] = v[2] - v[1];
							NL[0] = p[1] = store_point_normal(r);
						}
					}
					break;
				case 2:
					if (x)
						p[2] = Ny[y][x];
					else
					{
						if (v[3] == 0.0f)
						{
							Ny[y][0] = p[2] = surfint(0,y,z + 1,r);
							if (signbf(v[0])) p[3] = p[2];
							if (signbf(v[7]))
							{
								p[11] = p[2];
								if (y == 0) Nx[0][0] = p[2];
							}
						}
						else if (v[2] == 0.0f)
						{
							Ny[y][0] = p[2] = surfint(0,y + 1,z + 1,r);
							if (signbf(v[1])) NL[0] = p[1] = p[2];
							if (signbf(v[6])) Nx[y + 1][0] = p[10] = p[2];
						}
						else
						{
							t = v[3]/(v[3] - v[2]);
							r[0] = 0.0f; r[2] = float(z + 1);
							r[1] = y + t;
							r[3] = (v[7] - v[3])*(1.0f - t) + (v[6] - v[2])*t;
							r[4] = v[2] - v[3];
							r[5] = (z + 1 < nz? 0.5f*((F[z][y][0] - F[z + 2][y][0])*(1.0f - t)
										+ (F[z][y + 1][0] - F[z + 2][y + 1][0])*t):
										(v[3] - v[0])*(1.0f - t) + (v[2] - v[1])*t);
							Ny[y][0] = p[2] = store_point_normal(r);
						}
					}
					break;
				case 3:
					if (y || x)
						p[3] = OL[x];
					else
					{
						if (v[0] == 0.0f)
						{
							p[3] = surfint(0,0,z,r);
							if (signbf(v[1])) p[0] = p[3];
							if (signbf(v[4])) p[8] = p[3];
						}
						else if (v[3] == 0.0f)
						{
							p[3] = surfint(0,0,z + 1,r);
							if (signbf(v[2])) Ny[0][0] = p[2] = p[3];
							if (signbf(v[7])) Nx[0][0] = p[11] = p[3];
						}
						else
						{
							t = v[0]/(v[0] - v[3]);
							r[0] = r[1] = 0.0f;
							r[2] = z + t;
							r[3] = (v[4] - v[0])*(1.0f - t) + (v[7] - v[3])*t;
							r[4] = (v[1] - v[0])*(1.0f - t) + (v[2] - v[3])*t;
							r[5] = v[3] - v[0];
							p[3] = store_point_normal(r);
						}
					}
					break;
				case 4:
					if (z)
						p[4] = Oy[y][x + 1];
					else
					{
						if (v[4] == 0.0f)
						{
							Oy[y][x + 1] = p[4] = surfint(x + 1,y,0,r);
							if (signbf(v[7])) p[7] = p[4];
							if (signbf(v[0])) p[8] = p[4];
							if (y == 0)
								OL[x + 1] = p[4];
						}
						else if (v[5] == 0.0f)
						{
							Oy[y][x + 1] = p[4] = surfint(x + 1,y + 1,0,r);
							if (signbf(v[6])) NL[x + 1] = p[5] = p[4];
							if (signbf(v[1])) Ox[y + 1][x] = p[9] = p[4];
						}
						else
						{
							t = v[4]/(v[4] - v[5]);
							r[0] = float(x + 1); r[2] = 0.0f;
							r[1] = y + t;
							r[3] = (x + 1 < nx? 0.5f*((F[0][y][x] - F[0][y][x + 2])*(1.0f - t)
										+ (F[0][y + 1][x] - F[0][y + 1][x + 2])*t):
										(v[4] - v[0])*(1.0f - t) + (v[5] - v[1])*t);
							r[4] = v[5] - v[4];
							r[5] = (v[7] - v[4])*(1.0f - t) + (v[6] - v[5])*t;
							Oy[y][x + 1] = p[4] = store_point_normal(r);
						}
					}
					break;
				case 5:
					if (v[5] == 0.0f)
					{
						if (signbf(v[4]))
						{
							if (z)
							{
								NL[x + 1] = p[5] = p[4] = Oy[y][x + 1];
								if (signbf(v[1])) p[9] = p[5];
							}
							else
							{
								NL[x + 1] = p[5] = Oy[y][x + 1] = p[4] = surfint(x + 1,y + 1,0,r);
								if (signbf(v[1])) Ox[y + 1][x] = p[9] = p[5];
							}
						}
						else if (signbf(v[1]))
						{
							if (z)
								NL[x + 1] = p[5] = p[9] = Ox[y + 1][x];
							else
								NL[x + 1] = p[5] = Ox[y + 1][x] = p[9] = surfint(x + 1,y + 1,0,r);
						}
						else
							NL[x + 1] = p[5] = surfint(x + 1,y + 1,z,r);
					}
					else if (v[6] == 0.0f)
					{
						NL[x + 1] = p[5] = surfint(x + 1,y + 1,z + 1,r);
						if (signbf(v[2])) Nx[y + 1][x] = p[10] = p[5];
						if (signbf(v[7])) Ny[y][x + 1] = p[6] = p[5];
					}
					else
					{
						t = v[5]/(v[5] - v[6]);
						r[0] = float(x + 1); r[1] = float(y + 1);
						r[2] = z + t;
						r[3] = (x + 1 < nx? 0.5f*((F[z][y + 1][x] - F[z][y + 1][x + 2])*(1.0f - t)
									+ (F[z + 1][y + 1][x] - F[z + 1][y + 1][x + 2])*t):
									(v[5] - v[1])*(1.0f - t) + (v[6] - v[2])*t);
						r[4] = (y + 1 < ny? 0.5f*((F[z][y][x + 1] - F[z][y + 2][x + 1])*(1.0f - t)
									+ (F[z + 1][y][x + 1] - F[z + 1][y + 2][x + 1])*t):
									(v[5] - v[4])*(1.0f - t) + (v[6] - v[7])*t);
						r[5] = v[6] - v[5];
						NL[x + 1] = p[5] = store_point_normal(r);
					}
					break;
				case 6:
					if (v[7] == 0.0f)
					{
						if (signbf(v[3]))
						{
							if (y)
							{
								Ny[y][x + 1] = p[6] = p[11] = Nx[y][x];
								if (signbf(v[4])) p[7] = p[6];
							}
							else
							{
								Ny[y][x + 1] = p[6] = Nx[0][x] = p[11] = surfint(x + 1,0,z + 1,r);
								if (signbf(v[4])) OL[x + 1] = p[7] = p[6];
							}
						}
						else if (signbf(v[4]))
						{
							if (y)
								Ny[y][x + 1] = p[6] = p[7] = OL[x + 1];
							else
								Ny[y][x + 1] = p[6] = OL[x + 1] = p[7] = surfint(x + 1,0,z + 1,r);
						}
						else
							Ny[y][x + 1] = p[6] = surfint(x + 1,y,z + 1,r);
					}
					else if (v[6] == 0.0f)
					{
						Ny[y][x + 1] = p[6] = surfint(x + 1,y + 1,z + 1,r);
						if (signbf(v[5])) NL[x + 1] = p[5] = p[6];
						if (signbf(v[2])) Nx[y + 1][x] = p[10] = p[6];
					}
					else
					{
						t = v[7]/(v[7] - v[6]);
						r[0] = float(x + 1);
						r[1] = y + t; r[2] = float(z + 1);
						r[3] = (x + 1 < nx? 0.5f*((F[z + 1][y][x] - F[z + 1][y][x + 2])*(1.0f - t)
									+ (F[z + 1][y + 1][x] - F[z + 1][y + 1][x + 2])*t):
									(v[7] - v[3])*(1.0f - t) + (v[6] - v[2])*t);
						r[4] = v[6] - v[7];
						r[5] = (z + 1 < nz? 0.5f*((F[z][y][x + 1] - F[z + 2][y][x + 1])*(1.0f - t)
										+ (F[z][y + 1][x + 1] - F[z + 2][y + 1][x + 1])*t):
									(v[7] - v[4])*(1.0f - t) + (v[6] - v[5])*t);
						Ny[y][x + 1] = p[6] = store_point_normal(r);
					}
					break;
				case 7:
					if (y)
						p[7] = OL[x + 1];
					else
					{
						if (v[4] == 0.0f)
						{
							OL[x + 1] = p[7] = surfint(x + 1,0,z,r);
							if (signbf(v[0])) p[8] = p[7];
							if (signbf(v[5]))
							{
								p[4] = p[7];
								if (z == 0)
									Oy[0][x + 1] = p[7];
							}
						}
						else if (v[7] == 0.0f)
						{
							OL[x + 1] = p[7] = surfint(x + 1,0,z + 1,r);
							if (signbf(v[6])) Ny[0][x + 1] = p[6] = p[7];
							if (signbf(v[3])) Nx[0][x] = p[11] = p[7];
						}
						else
						{
							t = v[4]/(v[4] - v[7]);
							r[0] = float(x + 1); r[1] = 0.0f;
							r[2] = z + t;
							r[3] = (x + 1 < nx? 0.5f*((F[z][0][x] - F[z][0][x + 2])*(1.0f - t)
										+ (F[z + 1][0][x] - F[z + 1][0][x + 2])*t):
										(v[4] - v[0])*(1.0f - t) + (v[7] - v[3])*t);
							r[4] = (v[5] - v[4])*(1.0f - t) + (v[6] - v[7])*t;
							r[5] = v[7] - v[4];
							OL[x + 1] = p[7] = store_point_normal(r);
						}
					}
					break;
				case 8:
					if (z || y)
						p[8] = Ox[y][x];
					else
					{
						if (v[0] == 0.0f)
						{
							p[8] = surfint(x,0,0,r);
							if (signbf(v[1])) p[0] = p[8];
							if (signbf(v[3])) p[3] = p[8];
						}
						else if (v[4] == 0.0f)
						{
							p[8] = surfint(x + 1,0,0,r);
							if (signbf(v[5]))
								Oy[0][x + 1] = p[4] = p[8];
							if (signbf(v[7]))
								OL[x + 1] = p[7] = p[8];
						}
						else
						{
							t = v[0]/(v[0] - v[4]);
							r[1] = r[2] = 0.0f;
							r[0] = x + t;
							r[3] = v[4] - v[0];
							r[4] = (v[1] - v[0])*(1.0f - t) + (v[5] - v[4])*t;
							r[5] = (v[3] - v[0])*(1.0f - t) + (v[7] - v[4])*t;
							p[8] = store_point_normal(r);
						}
					}
					break;
				case 9:
					if (z)
						p[9] = Ox[y + 1][x];
					else
					{
						if (v[1] == 0.0f)
						{
							Ox[y + 1][x] = p[9] = surfint(x,y + 1,0,r);
							if (signbf(v[2]))
							{
								p[1] = p[9];
								if (x == 0) NL[0] = p[9];
							}
							if (signbf(v[0])) p[0] = p[9];
						}
						else if (v[5] == 0.0f)
						{
							Ox[y + 1][x] = p[9] = surfint(x + 1,y + 1,0,r);
							if (signbf(v[6])) NL[x + 1] = p[5] = p[9];
							if (signbf(v[4])) Oy[y][x + 1] = p[4] = p[9];
						}
						else
						{
							t = v[1]/(v[1] - v[5]);
							r[1] = float(y + 1); r[2] = 0.0f;
							r[0] = x + t;
							r[3] = v[5] - v[1];
							r[4] = (y + 1 < ny? 0.5f*((F[0][y][x] - F[0][y + 2][x])*(1.0f - t)
										+ (F[0][y][x + 1] - F[0][y + 2][x + 1])*t):
										(v[1] - v[0])*(1.0f - t) + (v[5] - v[4])*t);
							r[5] = (v[2] - v[1])*(1.0f - t) + (v[6] - v[5])*t;
							Ox[y + 1][x] = p[9] = store_point_normal(r);
						}
					}
					break;
				case 10:
					if (v[2] == 0.0f)
					{
						if (signbf(v[1]))
						{
							if (x)
							{
								Nx[y + 1][x] = p[10] = p[1] = NL[x];
								if (signbf(v[3])) p[2] = p[10];
							}
							else
							{
								Nx[y + 1][0] = p[10] = NL[0] = p[1] = surfint(0,y + 1,z + 1,r);
								if (signbf(v[3])) Ny[y][0] = p[2] = p[10];
							}
						}
						else if (signbf(v[3]))
						{
							if (x)
								Nx[y + 1][x] = p[10] = p[2] = Ny[y][x];
							else
								Nx[y + 1][0] = p[10] = Ny[y][0] = p[2] = surfint(0,y + 1,z + 1,r);
						}
						else
							Nx[y + 1][x] = p[10] = surfint(x,y + 1,z + 1,r);
					}
					else if (v[6] == 0.0f)
					{
						Nx[y + 1][x] = p[10] = surfint(x + 1,y + 1,z + 1,r);
						if (signbf(v[5])) NL[x + 1] = p[5] = p[10];
						if (signbf(v[7])) Ny[y][x + 1] = p[6] = p[10];
					}
					else
					{
						t = v[2]/(v[2] - v[6]);
						r[0] = x + t; 
						r[1] = float(y + 1); r[2] = float(z + 1);
						r[3] = v[6] - v[2];
						r[4] = (y + 1 < ny? 0.5f*((F[z + 1][y][x] - F[z + 1][y + 2][x])*(1.0f - t)
									+ (F[z + 1][y][x + 1] - F[z + 1][y + 2][x + 1])*t):
									(v[2] - v[3])*(1.0f - t) + (v[6] - v[7])*t);
						r[5] = (z + 1 < nz? 0.5f*((F[z][y + 1][x] - F[z + 2][y + 1][x])*(1.0f - t)
									+ (F[z][y + 1][x + 1] - F[z + 2][y + 1][x + 1])*t):
									(v[2] - v[1])*(1.0f - t) + (v[6] - v[5])*t);
						Nx[y + 1][x] = p[10] = store_point_normal(r);
					}
					break;
				case 11:
					if (y)
						p[11] = Nx[y][x];
					else
					{
						if (v[3] == 0.0f)
						{
							Nx[0][x] = p[11] = surfint(x,0,z + 1,r);
							if (signbf(v[0])) p[3] = p[11];
							if (signbf(v[2]))
							{
								p[2] = p[11];
								if (x == 0)
									Ny[0][0] = p[11];
							}
						}
						else if (v[7] == 0.0f)
						{
							Nx[0][x] = p[11] = surfint(x + 1,0,z + 1,r);
							if (signbf(v[4])) OL[x + 1] = p[7] = p[11];
							if (signbf(v[6])) Ny[0][x + 1] = p[6] = p[11];
						}
						else
						{
							t = v[3]/(v[3] - v[7]);
							r[1] = 0.0f; r[2] = float(z + 1);
							r[0] = x + t;
							r[3] = v[7] - v[3];
							r[4] = (v[2] - v[3])*(1.0f - t) + (v[6] - v[7])*t;
							r[5] = (z + 1 < nz? 0.5f*((F[z][0][x] - F[z + 2][0][x])*(1.0f - t)
										+ (F[z][0][x + 1] - F[z + 2][0][x + 1])*t):
										(v[3] - v[0])*(1.0f - t) + (v[7] - v[4])*t);
							Nx[0][x] = p[11] = store_point_normal(r);
						}
					}
				break;
				default:
					r[0] = x + 0.5f; r[1] = y + 0.5f; r[2] = z + 0.5f;
					r[3] = v[4] + v[5] + v[6] + v[7] - v[0] - v[1] - v[2] - v[3];
					r[4] = v[1] + v[2] + v[5] + v[6] - v[0] - v[3] - v[4] - v[7];
					r[5] = v[2] + v[3] + v[6] + v[7] - v[0] - v[1] - v[4] - v[5];
					p[12] = store_point_normal(r);
				}
			}
			ti[++k] = p[c];//now ti contains the vertex indices of the triangle
			i >>= 4;
		}
		if (ti[0] != ti[1] && ti[0] != ti[2] && ti[1] != ti[2])//to avoid zero area triangles
		{
#ifndef MC_Normal_neg
			if (m)//The order of triangle vertices is reversed
#else //it is also reversed if MC_Normal_neg was defined
			if (!m)
#endif
				{ti[2] = ti[0]; ti[0] = p[c];}
			if (!(MC_S->nT&0x0FFF))
			{
				try
				{
					MC_S->T.resize(MC_S->nT + 0x1000);
				}
				catch (...)
				{
					memoryfault = 1;
					if (MC_S->nT)
						MC_S->nT = 1;
					return;
				}
			}
			memcpy(MC_S->T[MC_S->nT++].v,ti,3*sizeof(int));
//			store_triangle(ti);
		}
	}
}

void MC33::free_temp_O_N()
{
	free(Ox); free(Nx); free(Oy); free(Ny);
	free(OL); free(NL);
}

void MC33::clear_temp_isosurface()
{
	if (F)
	{
		for (int y = 0; y != ny; ++y)
		{
			free(Ox[y]); free(Nx[y]); free(Oy[y]); free(Ny[y]);
		}
		free(Ox[ny]); free(Nx[ny]);
		free_temp_O_N();
		F = 0;
	}
}

int MC33::set_grid3d(grid3d *G)
{
	if (!G)
		return -1;
	clear_temp_isosurface();
	nx = G->N[0];
	ny = G->N[1];
	nz = G->N[2];
	for (int j = 0; j != 3; ++j)
	{
		MC_O[j] = float(G->r0[j]);
		MC_D[j] = float(G->d[j]);
	}
#ifndef GRD_orthogonal
	nonortho = G->nonortho;
	if (nonortho)
	{
		_A = G->_A;
		A_ = G->A_;
		mult_Abf(A_,MC_O,MC_O,0);
	}
#endif
	int x = nx*sizeof(int);
	OL = static_cast<int*>(malloc(x + sizeof(int)));//edges 3 (only read) and 7
	NL = static_cast<int*>(malloc(x + sizeof(int)));//edges 1 and 5 (only write)
	Oy = static_cast<int**>(malloc(sizeof(int*)*ny));//edges 0 (only read) and 4
	Ny = static_cast<int**>(malloc(sizeof(int*)*ny));//edges 2 and 6 (only write)
	Ox = static_cast<int**>(malloc(sizeof(int*)*(ny + 1)));//edges 8 (only read) and 9
	Nx = static_cast<int**>(malloc(sizeof(int*)*(ny + 1)));//edges 10 (only write) and 11
	if (!Nx || !NL)
	{
		free_temp_O_N();
		return -1;
	}
	F = G->F;
	for (int j = 0; j != ny; ++j)
	{
		Ox[j] = static_cast<int*>(malloc(x));
		Nx[j] = static_cast<int*>(malloc(x));
		Oy[j] = static_cast<int*>(malloc(x + sizeof(int)));
		Ny[j] = static_cast<int*>(malloc(x + sizeof(int)));
	}
	if (!Ny[ny - 1])
	{
		Ox[ny] = Nx[ny] = 0;
		clear_temp_isosurface();
		return -1;
	}
	Ox[ny] = static_cast<int*>(malloc(x));
	Nx[ny] = static_cast<int*>(malloc(x));
	if (!Nx[ny])
	{
		clear_temp_isosurface();
		return -1;
	}
	return 0;
}

surface* MC33::calculate_isosurface(float iso)
{
	GRD_data_type ***FG = F, **F0, **F1, *V00, *V01, *V11, *V10;
	int x, y, z;
	float Vt[12];
	float *w = Vt + 4;
	v = Vt;
	if (!FG)//The init_temp_isosurface function was not executed
		return 0;
	MC_S = new surface;
	MC_S->iso = iso;
	memoryfault = 0;
	for (z = 0; z != nz; ++z)
	{
		F0 = *FG;
		F1 = *(++FG);
		for (y = 0; y != ny; ++y)
		{
			V00 = *F0;
			V01 = *(++F0);
			V10 = *F1;
			V11 = *(++F1);
			w[0] = iso - *V00;//the difference was inverted to use signbit function
			w[1] = iso - *V01;
			w[2] = iso - *V11;
			w[3] = iso - *V10;
			//the eight least significant bits of i correspond to vertex indices. (x...x01234567)
			//If the bit is 1 then the vertex value is greater than zero.
			int i = signbf(w[3]);
			if (signbf(w[2])) i |= 2;
			if (signbf(w[1])) i |= 4;
			if (signbf(w[0])) i |= 8;
			for (x = 0; x != nx; ++x)
			{
				std::swap(v,w);
				w[0] = iso - *(++V00);
				w[1] = iso - *(++V01);
				w[2] = iso - *(++V11);
				w[3] = iso - *(++V10);
				i = ((i&0x0F)<<4)|signbf(w[3]);
				if (signbf(w[2])) i |= 2;
				if (signbf(w[1])) i |= 4;
				if (signbf(w[0])) i |= 8;
				if (i && i^0xFF) // i is different from 0 and 0xFF
				{
					if (v > w) memcpy(v + 4,w,4*sizeof(float));
					find_case(x,y,z,i);
				}
			}
			std::swap(OL,NL);
		}
		std::swap(Ox,Nx);
		std::swap(Oy,Ny);
	}
	if (memoryfault)
	{
		delete MC_S;
		return 0;
	}
	return MC_S;
}

#endif //marching_cubes_33_cpp
