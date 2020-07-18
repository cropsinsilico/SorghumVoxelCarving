#pragma once

#include <QVector3D>
#include <QDir>

#include "AABB.h"
#include "Camera.h"
#include "OBJWriter.h"
#include "StatsUtils.h"

/**
 * \brief A struct holding the coordinates of a voxel in the grid
 */
struct Voxel
{
	int x;
	int y;
	int z;

	Voxel() : x(-1), y(-1), z(-1) {}

	Voxel(int x, int y, int z) :
		x(x),
		y(y),
		z(z)
	{

	}

	friend bool operator<(const Voxel& l, const Voxel& r)
	{
		return std::tie(l.x, l.y, l.z)
			 < std::tie(r.x, r.y, r.z);
	}

	friend bool operator==(const Voxel& l, const Voxel& r)
	{
		return std::tie(l.x, l.y, l.z)
		    == std::tie(r.x, r.y, r.z);
	}

	friend bool operator!=(const Voxel& l, const Voxel& r)
	{
		return !(l == r);
	}

	/**
	 * \brief Sorting operator, order by Z, X, Y
	 * \param l 
	 * \param r 
	 * \return 
	 */
	static bool compareZXY(const Voxel& l, const Voxel& r)
	{
		return std::tie(l.z, l.x, l.y)
			 < std::tie(r.z, r.x, r.y);
	}
};

class VoxelGrid
{
public:
	VoxelGrid(const AABB& boundingBox, int resolutionX, int resolutionY, int resolutionZ);

	/**
	 * \brief Return the bounding box of the voxel grid
	 * \return The bounding box of the voxel grid
	 */
	const AABB& boundingBox() const;
	
	/**
	 * \brief Return the resolution of the voxel grid on the X axis
	 * \return Resolution of the voxel grid on the X axis
	 */
	int resolutionX() const;

	/**
	 * \brief Return the resolution of the voxel grid on the Y axis
	 * \return Resolution of the voxel grid on the Y axis
	 */
	int resolutionY() const;

	/**
	 * \brief Return the resolution of the voxel grid on the Z axis
	 * \return Resolution of the voxel grid on the Z axis
	 */
	int resolutionZ() const;

	/**
	 * \brief Return the size of a voxel on the X axis
	 * \return The size of a voxel on the X axis
	 */
	float voxelSizeX() const;

	/**
	 * \brief Return the size of a voxel on the Y axis
	 * \return The size of a voxel on the Y axis
	 */
	float voxelSizeY() const;

	/**
	 * \brief Return the size of a voxel on the Z axis
	 * \return The size of a voxel on the Z axis
	 */
	float voxelSizeZ() const;

	/**
	 * \brief Remove all voxels from the grid
	 */
	void clear();

	/**
	 * \brief Return the list of voxels after space has been carved
	 * \return The list of voxels after space has been carved
	 */
	const std::vector<Voxel>& voxels() const;

	/**
	 * \brief Return true if the voxel grid is empty (no voxels)
	 * \return True if the voxel grid is empty
	 */
	bool empty() const;

	/**
	 * \brief Sort the list of voxels
	 */
	void sortVoxels();

	/**
	 * \brief Give the index of a voxel in the list
	 *        Must be called with a voxel that is in the list, return -1 otherwise
	 *        Must be sorted to return valid results!
	 * \param x Integer coordinate on the X axis
	 * \param y Integer coordinate on the Y axis
	 * \param z Integer coordinate on the Z axis
	 * \return The index of the voxel in the list of voxels
	 */
	int voxelNumber(int x, int y, int z) const;

	/**
	 * \brief Return the index of a voxel in the flat grid
	 * \param x Integer coordinate on the X axis
	 * \param y Integer coordinate on the Y axis
	 * \param z Integer coordinate on the Z axis
	 * \return The index of a voxel in the flat grid
	 */
	std::size_t voxelIndex(int x, int y, int z) const;

	/**
	 * \brief Return the voxel corresponding to an index in the flat grid
	 * \param index The index of a voxel in the flat grid
	 * \return A voxel in the grid
	 */
	Voxel voxelFromIndex(std::size_t index) const;

	/**
	 * \brief Add a voxel in the grid
	 * \param x Integer coordinate on the X axis
	 * \param y Integer coordinate on the Y axis
	 * \param z Integer coordinate on the Z axis
	 */
	void add(int x, int y, int z);

	/**
	 * \brief Return true if the voxel exists in the grid
	 * \param x Integer coordinate on the X axis
	 * \param y Integer coordinate on the Y axis
	 * \param z Integer coordinate on the Z axis
	 * \return True if the voxel is present, false if it is not present
	 */
	bool hasVoxel(int x, int y, int z) const;

	/**
	 * \brief Return true if the voxel exists in the grid
	 * \param v Coordinates of the voxel
	 * \return True if the voxel is present, false if it is not present
	 */
	bool hasVoxel(const Voxel& v) const;
	
	/**
	 * \brief Return the number of neighboring voxels (between 0 and 26)
	 * \param x Integer coordinate on the X axis
	 * \param y Integer coordinate on the Y axis
	 * \param z Integer coordinate on the Z axis
	 * \return The number of neighboring voxels
	 */
	int numberNeighbors(int x, int y, int z) const;

	/**
	 * \brief Return the number of neighboring voxels (between 0 and 26)
	 * \param v Coordinates of the voxel
	 * \return The number of neighboring voxels
	 */
	int numberNeighbors(const Voxel& v) const;

	/**
	 * \brief Return the 3D coordinates of a voxel's center in the grid
	 * \param x Integer coordinate on the X axis
	 * \param y Integer coordinate on the Y axis
	 * \param z Integer coordinate on the Z axis
	 * \return The 3D coordinates of the voxel's center in the grid
	 */
	QVector3D voxel(int x, int y, int z) const;

	/**
	 * \brief Return the 3D coordinates of a voxel's center in the grid
	 * \param v Coordinates of the voxel
	 * \return The 3D coordinates of the voxel's center in the grid
	 */
	QVector3D voxel(const Voxel& v) const;

	/**
	 * \brief Return the voxel in which a point lies
	 * \param point A 3D point
	 * \return The voxel in which the point lies
	 */
	Voxel voxelFromPoint(const QVector3D& point) const;

	/**
	 * \brief Return true if the voxel is inside the grid
	 * \param voxel A voxel
	 * \return True if the voxel is inside the grid, otherwise false
	 */
	bool isVoxelInsideGrid(const Voxel& voxel) const;

	/**
	 * \brief Return the nearest voxel from a query point
	 * \param query A query point
	 * \return The nearest voxel
	 */
	QVector3D nearestVoxel(const QVector3D& query) const;

	/**
	 * \brief Return the nearest voxel (among a list of active voxel) from a query point
	 * \param query A query point
	 * \param activeVoxels A list of active voxels
	 * \return The nearest voxel
	 */
	QVector3D nearestVoxelWithActiveSet(const QVector3D& query, const std::vector<int>& activeVoxels) const;

	/**
	 * \brief Return the bounding box of voxels in the grid
	 * \return A pair of voxel coordinates defining a 3D voxel box
	 */
	std::pair<Voxel, Voxel> voxelBoundingBox() const;

	/**
	 * \brief Return an optimized version of this grid
	 * \return An equivalent grid without empty borders
	 */
	VoxelGrid optimized() const;
	
	/**
	 * \brief Save the center of voxels in OBJ format
	 * \param filename The path to the file
	 */
	void saveAsOBJ(const std::string& filename) const;

	/**
	 * \brief Export voxel cubes in OBJ format
	 * \param keepSurfaceOnly Discard voxels inside the shape
	 * \return 
	 */
	OBJWriter getVoxelsAsOBJ(bool keepSurfaceOnly = true) const;

	/**
	 * \brief Save voxel cubes in OBJ format
	 * \param filename The path to the file
	 * \param keepSurfaceOnly Discard voxels inside the shape
	 */
	bool saveVoxelsAsOBJ(const std::string& filename, bool keepSurfaceOnly = true) const;

	/**
	 * \brief Save the voxels in a pgm3d image
	 * \param filename The path to the file
	 */
	void saveAsPgm3d(const std::string& filename) const;

	/**
	 * \brief Export the voxel grid in a file
	 * \param filename The path to the file
	 */
	void exportVoxels(const std::string& filename) const;

	/**
	 * \brief Export the mesh of the voxel grid in a file.
	 *        Use Marching Cube to convert voxels to a mesh.
	 * \param filename The path to the file
	 */
	void exportMesh(const std::string& filename) const;

	/**
	 * \brief Import the voxel grid from a file
	 * \param filename The path to the file
	 */
	void importVoxels(const std::string& filename);

	/**
	 * \brief Import vertices from a file in voxel format
	 * \param filename The path to the file
	 */
	void importOBJ(const std::string& filename);

private:
	
	/**
	 * \brief 3D bounding box containing the voxel grid
	 */
	AABB m_boundingBox;

	/**
	 * \brief Resolution of the voxel grid on the X axis
	 */
	int m_resolutionX;

	/**
	 * \brief Resolution of the voxel grid on the Y axis
	 */
	int m_resolutionY;

	/**
	 * \brief Resolution of the voxel grid on the Z axis
	 */
	int m_resolutionZ;

	/**
	 * \brief A vector containing the flat 3D voxel grid
	 */
	std::vector<Voxel> m_voxels;

	/**
	 * \brief The voxel grid
	 */
	std::vector<bool> m_grid;
};

/**
 * \brief Draw an empty cube in the voxel grid
 * \param grid 
 * \param xRange Range of coordinates on x axis
 * \param yRange Range of coordinates on y axis
 * \param zRange Range of coordinates on z axis
 */
void drawCubeInGrid(VoxelGrid& grid,
					const std::pair<int, int>& xRange,
					const std::pair<int, int>& yRange,
					const std::pair<int, int>& zRange);

/**
 * \brief Count the number of voxels on the surface in a voxel grid.
 *        Any voxel that has less than 26 neighbors is considered on the surface.
 * \param grid A voxels grid
 * \return The number of voxel on the surface 
 */
long long countNumberSurfaceVoxels(const VoxelGrid& grid);

/**
 * \brief Apply a 3D dilation morphology operation on a voxel grid
 * \param grid The initial voxel grid
 * \param kernelSize Size of the kernel for dilation
 * \return A dilated version of the initial voxel grid
 */
VoxelGrid morphingDilation(const VoxelGrid& grid, int kernelSize);

/**
 * \brief Apply a 3D erosion morphology operation on a voxel grid
 * \param grid The initial voxel grid
 * \param kernelSize Size of the kernel for dilation
 * \return An eroded version of the initial voxel grid
 */
VoxelGrid morphingErosion(const VoxelGrid& grid, int kernelSize);

/**
 * \brief Add voxels where the mesh intersects the voxel grid
 * \param grid An existing voxel grid, in which voxels will be added
 * \param vertices Vertices of the mesh
 * \param faces Triangles of the mesh
 */
void voxelizeMesh(VoxelGrid& grid,
				  const QMatrix4x4& worldMatrix,
	              const std::vector<QVector3D>& vertices,
	              const std::vector<std::tuple<int, int, int>>& faces);

/**
 * \brief Fill cavities in a voxel grid
 *        Search for connected components
 *        Fill all empty connected components, except the biggest one
 * \param grid A voxel grid to fill
 */
void fillCavitiesInGrid(VoxelGrid& grid);

/**
 * \brief Compute the precision and recall of a voxel grid with regard to a reference voxel grid.
 * \param grid A predicted grid
 * \param reference A grid containing the ground truth
 * \return A pair: first the precision, second the recall
 */
PrecisionRecall computePrecisionRecall(const VoxelGrid& grid, const VoxelGrid& reference);

/**
 * \brief Find the volume of the bounding cylinder.
 *        The axis of the cylinder goes through the center of the voxel grid parallel to the Z axis.
 *        The cylinder axis of the cylinder goes from the lowest voxel to the highest voxel.
 * \param grid A voxel grid
 * \return The volume of the bounding cylinder
 */
float boundingCylinderVolume(const VoxelGrid& grid);

/**
 * \brief Compute the directionality of a plant in the grid.
 *		  The computation project the plant from the top and count the proportion of the floor in shade
 * \param grid A voxel grid with a plant
 * \return The proportion of shadow on the floor of the voxel grid
 */
float computeDirectionality(const VoxelGrid& grid);

/**
 * \brief Compute the plant height
 * \param grid A voxel grid with a plant
 * \return The height of the top voxel in the grid
 */
float computeHeight(const VoxelGrid& grid);

/**
 * \brief Render a voxel grid from different point of views and save them in a directory
 * \param grid The voxel grid to render
 * \param imageAngles A list of camera angles
 * \param width Width in pixels of the output images
 * \param height Height in pixels of the output images
 * \param filename Beginning of the filename. The file format is: {filename}_{angle}.png
 * \param outputDir Path to the folder in which to save images
 */
void renderGridAndSave(const VoxelGrid& grid,
	                   const std::vector<std::pair<float, bool>>& imageAngles,
	                   int width,
	                   int height,
	                   const QString& filename,
	                   const QDir& outputDir);

/**
 * \brief For each voxel in grid, find the nearest voxel in the reference.
 *        This function returns the maximum of such a distance.
 *        Brute force algorithm
 * \param grid A voxel grid from which we compute the distance to the reference grid
 * \param reference A reference voxel grid
 * \return The maximum distance from any voxel in the grid to the nearest voxel in the reference
 */
float maximumNearestDistanceFromGridToGrid(const VoxelGrid& grid, const VoxelGrid& reference);
