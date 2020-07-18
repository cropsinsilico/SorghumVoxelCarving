/*
	File: surface.cpp
	Programmed by: David Vega - dvega@uc.edu.ve
	August 2019
	February 2020
*/

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>

#include "../MC33.h"

using namespace std;

float surface::get_isovalue() { return iso;}

int surface::get_num_vertices() { return nV; }

int surface::get_num_triangles() { return nT; }

surface::surface() : nV(0), nT(0) {}

void surface::clear()
{
	T.clear();
	V.clear();
	N.clear();
	color.clear();
	nV = nT = 0;
}

void surface::adjustvectorlenght()
{
	T.resize(nT);
	V.resize(nV);
	N.resize(nV);
	color.resize(nV);
}


int *surface::getTriangle(unsigned int n) { return (n < nT? T[n].v: 0); }

float *surface::getVertex(unsigned int n) { return (n < nV? V[n].v: 0); }

float *surface::getNormal(unsigned int n) { return (n < nV? N[n].v: 0); }

void surface::setColor(unsigned int n, unsigned char *color) {
	if (n < nV)
		surface::color[n] = *(reinterpret_cast<int*>(color));
}

unsigned char* surface::getColor(unsigned int n) {
	return (n < nV? reinterpret_cast<unsigned char*>(&color[n]): 0);
}

int surface::save_bin(const char *filename)
{
	ofstream out(filename, ios::binary);
	if (!out)
		return -1;

	int n = 0x7075732e; //".sup";
	out.write(reinterpret_cast<char*>(&n),sizeof(int));
	out.write(reinterpret_cast<char*>(&iso),sizeof(float));
	out.write(reinterpret_cast<char*>(&nV),sizeof(int));
	out.write(reinterpret_cast<char*>(&nT),sizeof(int));

	out.write(reinterpret_cast<char*>(&T[0]),3*sizeof(int)*nT);
	out.write(reinterpret_cast<char*>(&V[0]),3*sizeof(float)*nV);
	out.write(reinterpret_cast<char*>(&N[0]),3*sizeof(float)*nV);
	out.write(reinterpret_cast<char*>(&color[0]),sizeof(int)*nV);
	return (out.good()? 0: -1);
}

int surface::read_bin(const char *filename)
{
	ifstream in(filename, ios::binary);
	if (!in)
		return -1;
	int n;
	in.read(reinterpret_cast<char*>(&n),sizeof(int));
	if (n != 0x7075732e) //".sup"
		return -1;

	in.read(reinterpret_cast<char*>(&iso),sizeof(float));
	in.read(reinterpret_cast<char*>(&nV),sizeof(int));
	in.read(reinterpret_cast<char*>(&nT),sizeof(int));
	adjustvectorlenght();
	in.read(reinterpret_cast<char*>(&T[0]),3*sizeof(int)*nT);
	in.read(reinterpret_cast<char*>(&V[0]),3*sizeof(float)*nV);
	in.read(reinterpret_cast<char*>(&N[0]),3*sizeof(float)*nV);
	in.read(reinterpret_cast<char*>(&color[0]),sizeof(int)*nV);
	return (in.good()? 0: -1);
}

int surface::save_txt(const char* filename)
{
	ofstream out(filename);
	adjustvectorlenght();
	if (!out)
		return -1;

	out << "isovalue: ";
	out.setf(ios_base::scientific, ios_base::floatfield);
	out.width(10);
	out.precision(5);
	out << iso << "\n\nVERTICES:\n" << nV << "\n\n";
	out.setf(ios_base::fixed, ios_base::basefield);
	out.precision(6);
	for (const auto &r: V)
		out << setw(9) << r.v[0] << ' ' << setw(9) << r.v[1] << ' ' << setw(9) << r.v[2] << endl;

	out << "\n\nTRIANGLES:\n" << nT << "\n\n";
	for (const auto &t: T)
		out << setw(8) << t.v[0] << ' ' << setw(8) << t.v[1] << ' ' << setw(8) << t.v[2] << endl;

	out << "\n\nNORMALS:\n";
	for (const auto &r: N)
		out << setw(9) << r.v[0] << ' ' << setw(9) << r.v[1] << ' ' << setw(9) << r.v[2] << endl;

	out << "\n\nCOLORS:\n";
	for (const auto &c: color)
		out << c << endl;
	out << "\nEND\n";
	return (out.good()? 0: -1);
}

