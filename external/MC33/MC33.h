/*
	File: MC33.h
	Programmed by: David Vega - dvega@uc.edu.ve
	version: 2.0
	February 2020
	This library is the C++ version of the library described in the paper:
	Vega, D., Abache, J., Coll, D., A Fast and Memory Saving Marching Cubes 33
	implementation with the correct interior test, Journal of Computer Graphics
	Techniques (JCGT), vol. 8, no. 3, 1–17, 2019.
*/

#ifndef MC33_h_
#define MC33_h_

#include <vector>
/********************************CUSTOMIZING**********************************/
//The following lines can be only changed before compiling the library:
//#define integer_GRD // for dataset with integer type
//#define size_type_GRD 4 // 1, 2, 4 or 8 (8 for double, if not defined integer_GRD)
#define GRD_orthogonal // If defined, the library only works with orthogonal grids.
#include "VoxelGrid.h"
/*****************************************************************************/
#if defined(integer_GRD)
#if size_type_GRD == 4
/*
GRD_data_type is the variable type of the grid data, by default it is float.
*/
typedef unsigned int GRD_data_type;
#elif size_type_GRD == 2
typedef unsigned short int GRD_data_type;
#elif size_type_GRD == 1
typedef unsigned char GRD_data_type;
#else
#error "Incorrect size of the data type. size_type_GRD permitted values: 1, 2 or 4."
#endif
#elif size_type_GRD == 8
typedef double GRD_data_type;
#else
typedef float GRD_data_type;
#undef size_type_GRD
#define size_type_GRD 4
#endif

class MC33;

/*
The class grid3d contains a 3D matrix (F[][][]) that stores the values of a function
evaluated at points of a 3D regularly spaced grid. N[] is the number of intervals in
each dimension. The grid contains (N[2] + 1)*(N[1] + 1)*(N[0] + 1) points. L[] is the
grid size in each dimension. r0[] are the coordinates of the first grid point. d[]
is the distance between adjacent points in each dimension (can be different for each
dimension), nonortho has a value of 1 when the grid is inclined else 0. _A and A_
are the matrices that transform from inclined to orthogonal coordinates and vice
versa, respectively. If the grid is periodic (is infinitely repeated along each
dimension) the flag periodic must be 1, else 0.

In this library, if GRD_orthogonal is defined, then nonortho, _A and A_ can be
removed from this structure, and it only works with orthogonal grids.
*/
class grid3d {
private:
	GRD_data_type ***F;
	int N[3];
	float L[3], Ang[3];
	double r0[3], d[3];
#ifndef GRD_orthogonal
	int nonortho;
	double _A[3][3], A_[3][3];
#endif
	int periodic; // to work with periodical grids, not used here...
	char title[160];
	void free_F();
	int alloc_F();
public:
	// get pointers to some class members:
	const int* get_N();
	const float* get_L();
	const float* get_Ang();
	const double* get_r0();
	const double* get_d();
#ifndef GRD_orthogonal
	const double (*get__A())[3];
	const double (*get_A_())[3];
	int isnotorthogonal(); // returns nonortho value
#endif
	void set_ratio_aspect(double rx, double ry, double rz); // modifies d and L

	// read_grd reads a *.grd file from the DMol3 program.
	// The function returns zero when succeeds.
	int read_grd(const char *filename);
	// read_grd_binary reads a file with an internal binary format
	int read_grd_binary(const char *filename);
	/*
	read_scanfiles reads a set of files that contain a slab of res*res scan data points,
	the data points are read as unsigned short int (if order is different from 0, the
	bytes of the unsigned short are exchanged). The filename must end with a number, and
	the function reads all files with end number greater or equal to filename.
	The function returns zero when succeeds.
	(Some datasets: http://www.graphics.stanford.edu/data/voldata/voldata.html)
	*/
	int read_scanfiles(const char *filename, int res, int order);
	/*
	read_raw_file reads a file that contains integer (8, 16 or 32 bits) or float (or double)
	data points. byte is the number of bytes of the integer (1, 2 or 4), or of the float (4
	or 8). If the data is big endian, byte must be negative (only for integers). The vector
	n[3] contains the number of points in each dimension. The size of file must be
	abs(byte)*n[0]*n[1]*n[2]. The function returns zero when succeeds.
	*/
	int read_raw_file(const char *filename, int *n, int byte, int isfloat = 0);
	/*
	read_dat_file reads a dat file. The function returns zero when succeeds. 
	http://www.cg.tuwien.ac.at/research/vis/datasets/
	*/
	int read_dat_file(const char *filename);

	/**
	 * \brief Convert a VoxelGrid to grid3d for a subsequent execution of the marching cube algorithm.
	 * \param grid A voxel grid
	 * \return 0 if conversion is successful, -1 otherwise
	 */
	int convert_from_voxel_grid(const VoxelGrid& grid);
	
	grid3d(void);
	~grid3d(void);
friend MC33;
};

struct int3
{
	int v[3];
};

struct float3
{
	float v[3];
};

/* The class surface contains the data of a isosurface. The isovalue is iso, the
number of surface points is nV, and the number of triangles is nT. The vector V
contains the vertex coordinates, the vector T contains the triangle indices, The
vector N contains the normal coordinates (one normal for each vertex), and the
color vector contains the color index of each point.*/
class surface {
private:
	unsigned int nV, nT;
	std::vector<int3> T;
	std::vector<float3> V, N;
	std::vector<int> color;
	float iso;
	int flag;
public:
	float get_isovalue(); // returns the isovalue
	int get_num_vertices(); // gets the number of vertices
	int get_num_triangles(); // gets the number of triangles
	int *getTriangle(unsigned int n); // gets a pointer to indices of triangle n
	float *getVertex(unsigned int n); // gets a pointer to coordinates of vertex n
	float *getNormal(unsigned int n); // gets a pointer to the normal vector n
	unsigned char *getColor(unsigned int n); // gets a pointer to the color of vertex n
	void setColor(unsigned int n, unsigned char *color);
	/******************************************************************
	Saves all the surface *S data (in binary format) to a "filename" file. The
	return value is 0 if the call succeeds, else -1.
	*/
	int save_bin(const char *filename);
	/******************************************************************
	Saves all the surface *S data (in plain text format) to a "filename" file.
	The return value is 0 if the call succeeds, else -1.
	*/
	int save_txt(const char *filename);
	/******************************************************************
	Reads (from a "filename" file) the surface data stored in binary format.
	The return value is 0 if the call succeeds, else -1.
	*/
	int read_bin(const char *filename);
	/* Draw the surface */
	void draw();
	/* Draw some points of the surface */
	void drawdraft();
	/* Clear all vector data */
	void clear();
	/* Set or correct the vector lengths */
	void adjustvectorlenght();
	surface(void);
//	~surface(void);
friend MC33;
};

/* Marching cubes 33 class.
The function member set_grid3d must be called once before calculate an isosurface.
If the grid3d object is modified or you want to use another grid3d object with the
same MC33 object, this function must be called again.
The function calculate_isosurface creates a surface object 

*/
class MC33 {
private:
	static int DefaultColor;
	//Variables for marching cubes surface
	surface *MC_S;
	//Auxiliary grid variables
	int nx, ny, nz;
	float MC_O[3], MC_D[3];
	#ifndef GRD_orthogonal
	int nonortho;
	double (*_A)[3], (*A_)[3];
	#endif
	GRD_data_type ***F;
	//Other auxiliary variables
	int memoryfault;
	float *v;
	// temporary structures that store the indexes of triangle vertices:
	int **Ox, **Oy, **Nx, **Ny, *OL, *NL;
	//Look up table
	unsigned short int table[2310];
	//Procedures
	int face_tests(int *, int);
	int face_test1(int);
	int interior_test(int, int);
	int store_point_normal(float *);
	//void store_triangle(int *);
	int surfint(int, int, int, float *);
	void find_case(int, int, int, int);
	void free_temp_MC();
	void free_temp_MC2();
	int init_temp_isosurface();
	void free_temp_O_N();
	void clear_temp_isosurface();
public:
	void set_default_surface_color(unsigned char *color);
	int set_grid3d(grid3d *G);
	surface* calculate_isosurface(float iso); // creates an isosurface
	MC33(void);
	~MC33(void);
};

#ifndef compiling_libMC33
int MC33::DefaultColor = 0xff5c5c5c;//gray RGB color as unsigned char[3], 0xAABBGGRR

#ifdef GL_VERSION
void surface::draw()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, &V[0]);
	glNormalPointer(GL_FLOAT, 0, &N[0]);
//	glColorPointer(3, GL_UNSIGNED_BYTE, 4, &color[0]);//rgb
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, &color[0]);//rgb alpha

	glDrawElements(GL_TRIANGLES, 3*nT, GL_UNSIGNED_INT, &T[0]);
	
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void surface::drawdraft()
{
	glDisable(GL_LIGHTING);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 12*sizeof(float), &V[0]);
	glColorPointer(3, GL_UNSIGNED_BYTE, 16, &color[0]);//rgb

	glDrawArrays(GL_POINTS, 0, nV>>2);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_LIGHTING);
}
#endif

//from https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Reciprocal_of_the_square_root
//http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf (0x5f375a86)
float invSqrt(float x)
{
	union
	{
		float x;
		int i;
	} u;
	u.x = x;
	x *= 0.5f;
	u.i = 0x5f375a86 - (u.i>>1);
	/* The next line can be repeated any number of times to increase accuracy */
	u.x = u.x * (1.5f - x * u.x * u.x);
	return u.x;
}

#ifndef GRD_orthogonal
//c = Ab, A is a 3x3 upper triangular matrix. If t != 0, A is transposed.
void _multTSA_bf(const double (*A)[3], float *b, float *c, int t)
{
	if(t)
	{
		c[2] = A[0][2]*b[0] + A[1][2]*b[1] + A[2][2]*b[2];
		c[1] = A[0][1]*b[0] + A[1][1]*b[1];
		c[0] = A[0][0]*b[0];
	}
	else
	{
		c[0] = A[0][0]*b[0] + A[0][1]*b[1] + A[0][2]*b[2];
		c[1] = A[1][1]*b[1] + A[1][2]*b[2];
		c[2] = A[2][2]*b[2];
	}
}
//Performs the multiplication of the matrix A and the vector b: c = Ab. If t != 0, A is transposed.
void _multA_bf(const double (*A)[3], float* b, float* c, int t)
{
	double u,v;
	if(t)
	{
		u = A[0][0]*b[0] + A[1][0]*b[1] + A[2][0]*b[2];
		v = A[0][1]*b[0] + A[1][1]*b[1] + A[2][1]*b[2];
		c[2] = A[0][2]*b[0] + A[1][2]*b[1] + A[2][2]*b[2];
	}
	else
	{
		u = A[0][0]*b[0] + A[0][1]*b[1] + A[0][2]*b[2];
		v = A[1][0]*b[0] + A[1][1]*b[1] + A[1][2]*b[2];
		c[2] = A[2][0]*b[0] + A[2][1]*b[1] + A[2][2]*b[2];
	}
	c[0] = u;
	c[1] = v;
}
void (*mult_Abf)(const double (*)[3], float *, float *, int) = _multA_bf;
#endif // GRD_orthogonal
#endif // compiling_libMC33

#endif // MC33_h_

