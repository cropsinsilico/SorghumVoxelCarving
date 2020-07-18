#include "VoxelGrid.h"

#include <algorithm>
#include <fstream>

#include <QtMath>
#include <QImage>

#include <MC33/MC33.h>

#include "MathUtils.h"
#include "OBJWriter.h"
#include "Reconstruction.h"
#include "TriangleBoxIntersection.h"
#include "UnionFind.h"
#include "StatsUtils.h"

VoxelGrid::VoxelGrid(const AABB& boundingBox, int resolutionX, int resolutionY, int resolutionZ) :
	m_boundingBox(boundingBox),
	m_resolutionX(resolutionX),
	m_resolutionY(resolutionY),
	m_resolutionZ(resolutionZ),
	m_grid(std::size_t(resolutionX) * std::size_t(resolutionY) * std::size_t(resolutionZ), false)
{
	
}

const AABB& VoxelGrid::boundingBox() const
{
	return m_boundingBox;
}

int VoxelGrid::resolutionX() const
{
	return m_resolutionX;
}

int VoxelGrid::resolutionY() const
{
	return m_resolutionY;
}

int VoxelGrid::resolutionZ() const
{
	return m_resolutionZ;
}

float VoxelGrid::voxelSizeX() const
{
	return m_boundingBox.sizeX() / float(m_resolutionX);
}

float VoxelGrid::voxelSizeY() const
{
	return m_boundingBox.sizeY() / float(m_resolutionY);
}

float VoxelGrid::voxelSizeZ() const
{
	return m_boundingBox.sizeZ() / float(m_resolutionZ);
}

void VoxelGrid::clear()
{
	m_voxels.clear();

	std::fill(m_grid.begin(), m_grid.end(), false);
}

void VoxelGrid::saveAsOBJ(const std::string& filename) const
{
	OBJWriter obj;

	for (const auto& v : m_voxels)
	{
		const auto point = voxel(v);
		obj.addVertex(point);
	}

	obj.save(filename);
}

OBJWriter VoxelGrid::getVoxelsAsOBJ(bool keepSurfaceOnly) const
{
	const auto halfSizeX = voxelSizeX() / 2;
	const auto halfSizeY = voxelSizeY() / 2;
	const auto halfSizeZ = voxelSizeZ() / 2;

	OBJWriter obj;

	for (const auto& v : m_voxels)
	{
		if (!keepSurfaceOnly || numberNeighbors(v.x, v.y, v.z) < 26)
		{
			const auto center = voxel(v);

			const auto a = center + QVector3D(-halfSizeX, -halfSizeY, -halfSizeZ);
			const auto b = center + QVector3D(-halfSizeX, halfSizeY, -halfSizeZ);
			const auto c = center + QVector3D(-halfSizeX, halfSizeY, halfSizeZ);
			const auto d = center + QVector3D(-halfSizeX, -halfSizeY, halfSizeZ);
			const auto e = center + QVector3D(halfSizeX, -halfSizeY, -halfSizeZ);
			const auto f = center + QVector3D(halfSizeX, halfSizeY, -halfSizeZ);
			const auto g = center + QVector3D(halfSizeX, halfSizeY, halfSizeZ);
			const auto h = center + QVector3D(halfSizeX, -halfSizeY, halfSizeZ);

			const auto indexA = obj.addVertex(a);
			const auto indexB = obj.addVertex(b);
			const auto indexC = obj.addVertex(c);
			const auto indexD = obj.addVertex(d);
			const auto indexE = obj.addVertex(e);
			const auto indexF = obj.addVertex(f);
			const auto indexG = obj.addVertex(g);
			const auto indexH = obj.addVertex(h);


			obj.addFace(indexA, indexD, indexB);
			obj.addFace(indexC, indexB, indexD);

			obj.addFace(indexE, indexF, indexH);
			obj.addFace(indexG, indexH, indexF);

			obj.addFace(indexB, indexC, indexF);
			obj.addFace(indexG, indexF, indexC);

			obj.addFace(indexA, indexE, indexD);
			obj.addFace(indexH, indexD, indexE);

			obj.addFace(indexD, indexH, indexC);
			obj.addFace(indexC, indexH, indexG);

			obj.addFace(indexA, indexB, indexE);
			obj.addFace(indexF, indexE, indexB);
		}
	}

	return obj;
}

void VoxelGrid::saveAsPgm3d(const std::string& filename) const
{
	// Write voxels to a file
	std::ofstream file(filename, std::fstream::out);

	if (!file.is_open())
	{
		return;
	}

	// Write the header
	file << "P2-3D\n";
	file << m_resolutionX << " " << m_resolutionY << " " << m_resolutionZ << "\n";
	file << "255\n\n\n";

	for (int z = 0; z < m_resolutionZ; z++)
	{
		for (int y = 0; y < m_resolutionY; y++)
		{
			for (int x = 0; x < m_resolutionX; x++)
			{
				if (hasVoxel(x, y, z))
				{
					file << "255 ";
				}
				else
				{
					file << "0 ";
				}
			}
			
			file << "\n";
		}

		file << "\n\n";
	}

	file.close();
}

std::size_t VoxelGrid::voxelIndex(int x, int y, int z) const
{
	return (std::size_t(m_resolutionY) * std::size_t(m_resolutionZ)) * x
	      + std::size_t(m_resolutionZ) * y
	      + z;
}

Voxel VoxelGrid::voxelFromIndex(std::size_t index) const
{
	Voxel v;

	const auto division = index / (std::size_t(m_resolutionY) * std::size_t(m_resolutionZ));
	const auto remainder = index % (std::size_t(m_resolutionY) * std::size_t(m_resolutionZ));

	v.x = division;
	v.y = remainder / std::size_t(m_resolutionZ);
	v.z = remainder % std::size_t(m_resolutionZ);

	// Check that the result is the inverse of the voxelIndex
	assert(index == voxelIndex(v.x, v.y, v.z));

	return v;
}

const std::vector<Voxel>& VoxelGrid::voxels() const
{
	return m_voxels;
}

bool VoxelGrid::empty() const
{
	return m_voxels.empty();
}

void VoxelGrid::sortVoxels()
{
	std::sort(m_voxels.begin(), m_voxels.end());
}

int VoxelGrid::voxelNumber(int x, int y, int z) const
{
	const auto it = std::lower_bound(m_voxels.begin(), m_voxels.end(), Voxel(x, y, z));

	if (it != m_voxels.end())
	{
		return std::distance(m_voxels.begin(), it);
	}

	return -1;
}

void VoxelGrid::add(int x, int y, int z)
{
	assert(x >= 0);
	assert(y >= 0);
	assert(z >= 0);

	assert(x < m_resolutionX);
	assert(y < m_resolutionY);
	assert(z < m_resolutionZ);

	if (!hasVoxel(x, y, z))
	{
		m_grid[voxelIndex(x, y, z)] = true;

		m_voxels.emplace_back(x, y, z);
	}
}

bool VoxelGrid::hasVoxel(int x, int y, int z) const
{
	if (x >= 0 && y >= 0 && z >= 0
	 && x < m_resolutionX && y < m_resolutionY && z < m_resolutionZ)
	{
		return m_grid[voxelIndex(x, y, z)];
	}
	else
	{
		return false;
	}
}

bool VoxelGrid::hasVoxel(const Voxel& v) const
{
	return hasVoxel(v.x, v.y, v.z);
}

int VoxelGrid::numberNeighbors(int x, int y, int z) const
{
	int neighbors = 0;
	
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			for (int k = -1; k <= 1; k++)
			{
				const auto vx = x + i;
				const auto vy = y + j;
				const auto vz = z + k;

				if ((i != 0 || j != 0 || k != 0) && hasVoxel(vx, vy, vz))
				{
					neighbors++;
				}
			}
		}
	}

	return neighbors;
}

int VoxelGrid::numberNeighbors(const Voxel& v) const
{
	return numberNeighbors(v.x, v.y, v.z);
}

QVector3D VoxelGrid::voxel(int x, int y, int z) const
{
	assert(x >= 0);
	assert(y >= 0);
	assert(z >= 0);

	assert(x < m_resolutionX);
	assert(y < m_resolutionY);
	assert(z < m_resolutionZ);
	
	return m_boundingBox.lerp({
		float(x) / (m_resolutionX - 1),
		float(y) / (m_resolutionY - 1),
		float(z) / (m_resolutionZ - 1)
	});
}

QVector3D VoxelGrid::voxel(const Voxel& v) const
{
	return voxel(v.x, v.y, v.z);
}

Voxel VoxelGrid::voxelFromPoint(const QVector3D& point) const
{
	const auto t = m_boundingBox.inverseLerp(point);
	
	return {
		int(std::round(float(m_resolutionX - 1) * t.x())),
		int(std::round(float(m_resolutionY - 1) * t.y())),
		int(std::round(float(m_resolutionZ - 1) * t.z()))
	};
}

bool VoxelGrid::isVoxelInsideGrid(const Voxel& voxel) const
{
	return (voxel.x >= 0 && voxel.x < m_resolutionX)
		&& (voxel.y >= 0 && voxel.y < m_resolutionY)
		&& (voxel.z >= 0 && voxel.z < m_resolutionZ);
}

QVector3D VoxelGrid::nearestVoxel(const QVector3D& query) const
{
	float minimumDistanceSq = std::numeric_limits<float>::max();
	QVector3D nearestPoint;
	
	for (const auto& v : m_voxels)
	{
		const auto point = voxel(v);

		const float distSq = distanceSquared(query, point);

		if (distSq < minimumDistanceSq)
		{
			minimumDistanceSq = distSq;
			nearestPoint = point;
		}
	}

	return nearestPoint;
}

QVector3D VoxelGrid::nearestVoxelWithActiveSet(const QVector3D& query, const std::vector<int>& activeVoxels) const
{
	float minimumDistanceSq = std::numeric_limits<float>::max();
	QVector3D nearestPoint;

	for (auto voxelIndex : activeVoxels)
	{
		assert(voxelIndex >= 0 && voxelIndex < m_voxels.size());

		const auto point = voxel(m_voxels[voxelIndex]);

		const float distSq = distanceSquared(query, point);

		if (distSq < minimumDistanceSq)
		{
			minimumDistanceSq = distSq;
			nearestPoint = point;
		}
	}

	return nearestPoint;
}

std::pair<Voxel, Voxel> VoxelGrid::voxelBoundingBox() const
{
	int maxX = 0;
	int maxY = 0;
	int maxZ = 0;
	
	int minX = m_resolutionX;
	int minY = m_resolutionY;
	int minZ = m_resolutionZ;
	
	// #pragma omp parallel for reduction(min:minX, minY, minZ) reduction(max:maxX, maxY, maxZ)
	for (int i = 0; i < m_voxels.size(); i++)
	{
		minX = std::min(minX, m_voxels[i].x);
		minY = std::min(minY, m_voxels[i].y);
		minZ = std::min(minZ, m_voxels[i].z);

		maxX = std::max(maxX, m_voxels[i].x);
		maxY = std::max(maxY, m_voxels[i].y);
		maxZ = std::max(maxZ, m_voxels[i].z);
	}

	return std::make_pair(Voxel(minX, minY, minZ), Voxel(maxX, maxY, maxZ));
}

VoxelGrid VoxelGrid::optimized() const
{
	// Find bounding box of the current voxels
	const auto box = voxelBoundingBox();

	const AABB newBoundingBox(
		voxel(box.first.x, box.first.y, box.first.z),
		voxel(box.second.x, box.second.y, box.second.z)
	);
	
	const int resolutionX = 1 + box.second.x - box.first.x;
	const int resolutionY = 1 + box.second.y - box.first.y;
	const int resolutionZ = 1 + box.second.z - box.first.z;

	// Create a new voxel grid
	VoxelGrid newGrid(newBoundingBox, resolutionX, resolutionY, resolutionZ);

	// Copy vector and offset
	for (const auto voxel : m_voxels)
	{
		newGrid.add(
			voxel.x - box.first.x,
			voxel.y - box.first.y,
			voxel.z - box.first.z
		);
	}

	return newGrid;
}

void VoxelGrid::exportVoxels(const std::string& filename) const
{
	// Write voxels to a file
	std::ofstream file(filename, std::fstream::out);

	if (!file.is_open())
	{
		return;
	}

	// Write the total number of voxels in the file
	file << m_voxels.size() << "\n";
	
	// Export the voxels
	for (const auto& v : m_voxels)
	{
		file << v.x << " " << v.y << " " << v.z << "\n";
	}

	file.close();
}

void VoxelGrid::exportMesh(const std::string& filename) const
{
	// Convert this grid to the format required by the marching cube algorithm
	grid3d grid;
	grid.convert_from_voxel_grid(*this);

	MC33 marchingCube;
	marchingCube.set_grid3d(&grid);

	// The interior of the plant has a value of 1 and the exterior 0.
	// We thus calculate the iso surface with value 0.5
	surface* surf = marchingCube.calculate_isosurface(0.5f);

	OBJWriter writer;

	// Add vertices and normals
	for (int i = 0; i < surf->get_num_vertices(); i++)
	{
		const float* v = surf->getVertex(i);
		const QVector3D vertex(v[0], v[1], v[2]);
		writer.addVertex(vertex);

		const float* n = surf->getNormal(i);
		const QVector3D normal(n[0], n[1], n[2]);
		writer.addNormal(normal);
	}

	// Add faces
	for (int i = 0; i < surf->get_num_triangles(); i++)
	{
		const int* t = surf->getTriangle(i);
		writer.addFace(t[0], t[1], t[2]);
	}

	writer.save(filename);
}

void VoxelGrid::importVoxels(const std::string& filename)
{
	// Import voxels from a file
	std::ifstream file(filename, std::fstream::in);

	if (!file.is_open())
	{
		return;
	}

	// Number of voxels in the file
	int numberVoxels;
	file >> numberVoxels;

	assert(numberVoxels >= 0);

	// Allocation de la grille
	clear();
	m_voxels.reserve(numberVoxels);

	// Import the voxels
	for (int i = 0; i < numberVoxels; i++)
	{
		Voxel v;
		
		file >> v.x >> v.y >> v.z;

		add(v.x, v.y, v.z);
	}

	file.close();
}

void VoxelGrid::importOBJ(const std::string& filename)
{
	// Read file
	std::ifstream file(filename);

	if (!file.is_open())
	{
		return;
	}

	// Clear the existing grid
	clear();
	
	std::string command;

	// For each voxel compute the coordinates in 3D space
	while (file >> command)
	{
		// If the line is a vertex
		if (command.size() == 1 && command.front() == 'v')
		{
			// Coordinates of the vertices in 3D space
			float x, y, z;
			file >> x >> y >> z;

			// If the point is in the bounding box
			if (m_boundingBox.isInside(QVector3D(x, y, z)))
			{
				// Convert to voxel coordinates
				const auto v = voxelFromPoint(QVector3D(x, y, z));

				// Add hte current voxel to the grid
				add(v.x, v.y, v.z);
			}
		}
	}

	file.close();
}

bool VoxelGrid::saveVoxelsAsOBJ(const std::string& filename, bool keepSurfaceOnly) const
{
	const auto obj = getVoxelsAsOBJ(keepSurfaceOnly);

	return obj.save(filename);
}

void drawCubeInGrid(
	VoxelGrid& grid,
	const std::pair<int, int>& xRange,
	const std::pair<int, int>& yRange,
	const std::pair<int, int>& zRange)
{
	// Fill bottom half of the grid
	for (int x = xRange.first; x <= xRange.second; x++)
	{
		for (int y = yRange.first; y <= yRange.second; y++)
		{
			// Bottom cap
			grid.add(x, y, zRange.first);

			// Top cap
			grid.add(x, y, zRange.second);

			// On the sides
			if (x == xRange.first || y == yRange.first || x == xRange.second || y == yRange.second)
			{
				for (int z = zRange.first; z <= zRange.second; z++)
				{
					grid.add(x, y, z);
				}
			}
		}
	}
}

long long countNumberSurfaceVoxels(const VoxelGrid& grid)
{
	const auto& voxels = grid.voxels();
	
	long long numberSurfaceVoxels = 0;

	#pragma omp parallel for shared(numberSurfaceVoxels)
	for (int i = 0; i < voxels.size(); i++)
	{
		const auto v = voxels[i];
		
		if (grid.numberNeighbors(v.x, v.y, v.z) < 26)
		{
			#pragma omp atomic
			numberSurfaceVoxels++;
		}
	}

	return numberSurfaceVoxels;
}

VoxelGrid morphingDilation(const VoxelGrid& grid, int kernelSize)
{
	const auto kernelWidth = kernelSize / 2;
	
	VoxelGrid newGrid(grid.boundingBox(), grid.resolutionX(), grid.resolutionY(), grid.resolutionZ());

	#pragma omp parallel for collapse(3)
	for (int x = 0; x < grid.resolutionX(); x++)
	{
		for (int y = 0; y < grid.resolutionY(); y++)
		{
			for (int z = 0; z < grid.resolutionZ(); z++)
			{
				bool dilateHere = false;
				
				// Apply kernel
				for (int i = -kernelWidth; i <= kernelWidth && !dilateHere; i++)
				{
					for (int j = -kernelWidth; j <= kernelWidth && !dilateHere; j++)
					{
						for (int k = -kernelWidth; k <= kernelWidth && !dilateHere; k++)
						{
							const auto vx = clamp(x + i, 0, grid.resolutionX() - 1);
							const auto vy = clamp(y + j, 0, grid.resolutionY() - 1);
							const auto vz = clamp(z + k, 0, grid.resolutionZ() - 1);

							if (vx != x || vy != y || vz != z)
							{
								if (grid.hasVoxel(vx, vy, vz))
								{
									// Keep the voxel(x, y, z)
									dilateHere = true;
								}
							}
						}
					}
				}

				// If at least one voxel is in the kernel
				if (dilateHere)
				{
					// Keep the voxel (x, y, z)
					#pragma omp critical(dilation_add_voxel)
					{
						newGrid.add(x, y, z);
					}
				}
			}
		}
	}

	return newGrid;
}

VoxelGrid morphingErosion(const VoxelGrid& grid, int kernelSize)
{
	const auto kernelWidth = kernelSize / 2;

	VoxelGrid newGrid(grid.boundingBox(), grid.resolutionX(), grid.resolutionY(), grid.resolutionZ());

	#pragma omp parallel for collapse(3)
	for (int x = kernelWidth; x < grid.resolutionX() - kernelWidth; x++)
	{
		for (int y = kernelWidth; y < grid.resolutionY() - kernelWidth; y++)
		{
			for (int z = kernelWidth; z < grid.resolutionZ() - kernelWidth; z++)
			{
				bool erodeHere = true;

				// Apply kernel
				for (int i = -kernelWidth; i <= kernelWidth && erodeHere; i++)
				{
					for (int j = -kernelWidth; j <= kernelWidth && erodeHere; j++)
					{
						for (int k = -kernelWidth; k <= kernelWidth && erodeHere; k++)
						{
							const auto vx = clamp(x + i, 0, grid.resolutionX() - 1);
							const auto vy = clamp(y + j, 0, grid.resolutionY() - 1);
							const auto vz = clamp(z + k, 0, grid.resolutionZ() - 1);

							if (vx != x || vy != y || vz != z)
							{
								if (!grid.hasVoxel(vx, vy, vz))
								{
									// Keep the voxel(x, y, z)
									erodeHere = false;
								}
							}
						}
					}
				}

				// If all voxels are in the kernel
				if (erodeHere)
				{
					// Keep the voxel (x, y, z)
					#pragma omp critical(erosion_add_voxel)
					{
						newGrid.add(x, y, z);
					}
				}
			}
		}
	}

	return newGrid;
}

void voxelizeMesh(
	VoxelGrid& grid,
	const QMatrix4x4& worldMatrix,
	const std::vector<QVector3D>& vertices,
	const std::vector<std::tuple<int, int, int>>& faces)
{	
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		const auto vertex1 = worldMatrix.map(vertices[std::get<0>(faces[i])]);
		const auto vertex2 = worldMatrix.map(vertices[std::get<1>(faces[i])]);
		const auto vertex3 = worldMatrix.map(vertices[std::get<2>(faces[i])]);

		// Compute larger side of triangle
		const auto largerSide = std::max({
			vertex1.distanceToPoint(vertex2),
			vertex1.distanceToPoint(vertex3),
			vertex2.distanceToPoint(vertex3)
		});

		const auto minVoxelSize = std::min({
			grid.voxelSizeX(),
			grid.voxelSizeY(),
			grid.voxelSizeZ()
		});

		// Compute the sampling resolution so that the larger side is sampled with
		// Twenty times more points evenly spaced than the resolution of the voxel grid
		// Oversampling to be sure that we get all the voxels
		const auto samplingResolution = int(std::ceil(20.0 * largerSide / minVoxelSize));

		// Sample points on this triangle		
		for (int j = 0; j < samplingResolution; j++)
		{
			for (int k = 0; k < samplingResolution; k++)
			{
				const auto u = float(j) / (samplingResolution - 1);
				const auto v = float(k) / (samplingResolution - 1);

				if (1.0f - u - v >= 0.0f)
				{
					const auto vertex = u * vertex1 + v * vertex2 + (1.0f - u - v) * vertex3;
					// Find nearest voxel center and add it to the grid
					const auto voxel = grid.voxelFromPoint(vertex);

					if (grid.isVoxelInsideGrid(voxel))
					{
						grid.add(voxel.x, voxel.y, voxel.z);
					}
				}
			}
		}
	}
}

void fillCavitiesInGrid(VoxelGrid& grid)
{
	// Complete volumes with Union-Find
	// Warning: requires a lot of memory for big grids
	const auto nbVoxels = grid.resolutionX() * grid.resolutionY() * grid.resolutionZ();

	UnionFind unionFind(nbVoxels);

	std::array<std::tuple<int, int, int>, 3> directions =
	{ {
		{1, 0, 0},
		{0, 1, 0},
		{0, 0, 1}
	} };

	for (int x = 0; x < grid.resolutionX(); x++)
	{
		for (int y = 0; y < grid.resolutionY(); y++)
		{
			for (int z = 0; z < grid.resolutionZ(); z++)
			{
				// Check that neighbors are of the same type
				// Check that neighbors are of the same type
				for (const auto& direction : directions)
				{
					const auto vx = clamp(x + std::get<0>(direction), 0, grid.resolutionX() - 1);
					const auto vy = clamp(y + std::get<1>(direction), 0, grid.resolutionY() - 1);
					const auto vz = clamp(z + std::get<2>(direction), 0, grid.resolutionZ() - 1);

					if ((vx != x || vy != y || vz != z) && (grid.hasVoxel(x, y, z) == grid.hasVoxel(vx, vy, vz)))
					{
						// The voxel (x, y, z) has the same type as its neighbor
						// Find the index of the voxel in the union find list
						const auto firstIndex = grid.voxelIndex(x, y, z);
						const auto secondIndex = grid.voxelIndex(vx, vy, vz);

						// Union between (x, y, z) and (vx, vy, vz)
						unionFind.unionNodes(firstIndex, secondIndex);
					}
				}
			}
		}
	}

	// List all non empty subsets indexed by root node
	std::vector<unsigned int> connectedComponentsRoots;
	for (unsigned int i = 0; i < nbVoxels; i++)
	{
		if (unionFind.findNode(i) == i)
		{
			connectedComponentsRoots.push_back(i);
		}
	}
	std::sort(connectedComponentsRoots.begin(), connectedComponentsRoots.end());

	// A vector with all connected components
	std::vector<std::vector<int>> connectedComponents;
	connectedComponents.reserve(connectedComponentsRoots.size());

	// Add the roots to the connected components
	for (const auto root : connectedComponentsRoots)
	{
		connectedComponents.emplace_back(1, root);
	}

	qInfo() << "Total number of connected components: " << connectedComponents.size();

	// Add every voxels to their connected components
	for (unsigned int i = 0; i < nbVoxels; i++)
	{
		const auto root = unionFind.findNode(i);

		// If the node is not a root
		if (root != i)
		{
			// Search for his connected component
			const auto it = std::lower_bound(connectedComponentsRoots.begin(), connectedComponentsRoots.end(), root);

			if (it != connectedComponentsRoots.end())
			{
				const int connectedComp = std::distance(connectedComponentsRoots.begin(), it);
				connectedComponents[connectedComp].push_back(i);
			}
		}
	}

	// Find the second major empty connected component
	int majorEmptyComponent = -1;
	for (int i = 0; i < connectedComponents.size(); i++)
	{
		const auto firstVoxel = grid.voxelFromIndex(connectedComponents[i].front());

		// Check if this connected component is empty in the voxel grid
		if (!grid.hasVoxel(firstVoxel))
		{
			qInfo() << "Empty component " << i << " " << connectedComponents[i].size() << " voxels";

			// If no major empty component has been found yet
			// Or the current major component is smaller than the component in review
			if (majorEmptyComponent == -1
				|| (connectedComponents[i].size() > connectedComponents[majorEmptyComponent].size()))
			{
				majorEmptyComponent = i;
			}
		}
	}

	// Fill all empty connected components that are not the biggest
	for (int i = 0; i < connectedComponents.size(); i++)
	{
		if (i != majorEmptyComponent)
		{
			const auto firstVoxel = grid.voxelFromIndex(connectedComponents[i].front());

			// Check if this connected component is empty in the voxel grid
			if (!grid.hasVoxel(firstVoxel))
			{

				// Fill all voxels of this connected component
				for (const auto index : connectedComponents[i])
				{
					const auto v = grid.voxelFromIndex(index);
					grid.add(v.x, v.y, v.z);
				}
			}
		}
	}
}

PrecisionRecall computePrecisionRecall(const VoxelGrid& grid, const VoxelGrid& reference)
{
	// Voxels present in both grids
	long long truePositives = 0;

	// Voxels present in the predicted grid but not in the reference grid
	long long falsePositives = 0;

	// Voxels present in the reference grid but not in the predicted grid
	long long falseNegatives = 0;

	// Compute true positives and false positives
	for (const auto& v : grid.voxels())
	{
		// Voxel v is in the predicted grid
		if (reference.hasVoxel(v))
		{
			// If it is also in the reference grid, it's a true positive
			truePositives++;
		}
		else
		{
			// If it is not in the reference grid, it's a false positive
			falsePositives++;
		}
	}

	// Compute false negatives
	for (const auto& v : reference.voxels())
	{
		// Voxel v is in the reference grid
		if (!grid.hasVoxel(v))
		{
			// If it is in the reference, but not in the predicted gris, it's a false negative
			falseNegatives++;
		}
	}

	const float precision = float(truePositives) / (truePositives + falsePositives);
	const float recall = float(truePositives) / (truePositives + falseNegatives);
	
	return { precision, recall };
}

float boundingCylinderVolume(const VoxelGrid& grid)
{
	const auto center = grid.boundingBox().center();

	// The axis of the cylinder goes through the center of the bounding box and is parallel to Z
	const QVector3D axisBottom(center.x(), center.y(), grid.boundingBox().minZ());
	const QVector3D axisTop(center.x(), center.y(), grid.boundingBox().maxZ());

	// For each voxel we compute the distance to the cylinder axis, and we take the maximum
	float minimumZ = axisTop.z();
	float maximumZ = axisBottom.z();
	float maximumRadius = 0.0f;
	for (int i = 0; i < grid.voxels().size(); i++)
	{
		const auto voxel = grid.voxels()[i];
		const auto voxelPosition = grid.voxel(voxel);
		const auto radius = distanceToLineSegment(voxelPosition, axisBottom, axisTop);

		minimumZ = std::min(minimumZ, voxelPosition.z());
		maximumZ = std::max(maximumZ, voxelPosition.z());
		maximumRadius = std::max(maximumRadius, radius);
	}

	// Compute the volume of the cylinder
	const auto volume = maximumRadius * maximumRadius * float(M_PI) * (maximumZ - minimumZ);

	return volume;
}

float computeDirectionality(const VoxelGrid& grid)
{
	std::vector<bool> shadow(grid.resolutionX() * grid.resolutionY(), false);

	const auto& voxels = grid.voxels();
	for (int i = 0; i < voxels.size(); i++)
	{
		const auto voxel = voxels[i];
		// Index of the Z projection in a 1D array
		const auto index = voxel.y * grid.resolutionX() + voxel.x;

		shadow[index] = true;
	}

	// Count number of true in the shadow grid
	const auto shadowTotal = std::count(shadow.begin(), shadow.end(), true);

	// Return the proportion of shadow on the floor of the voxel grid
	return float(shadowTotal) / float(shadow.size());
}

float computeHeight(const VoxelGrid& grid)
{
	const auto meshBoundingBox = grid.voxelBoundingBox();
	return grid.voxel(meshBoundingBox.second).z() - grid.boundingBox().minZ();
}

void renderGridAndSave(
	const VoxelGrid& grid,
	const std::vector<std::pair<float, bool>>& imageAngles,
	int width,
	int height,
	const QString& filename,
	const QDir& outputDir)
{
	assert(outputDir.exists());
	
	const auto cameras = generateCameras(imageAngles, 90.f);
	// Reprojection of voxels
	const auto objSkeletonGrid = grid.getVoxelsAsOBJ(true);
	for (int c = 0; c < cameras.size(); c++)
	{
		const auto camera = cameras[c];

		// Re project voxel cubes on this camera
		const auto projectedGridImage = camera.render(QMatrix4x4(),
			                                          objSkeletonGrid.vertices(),
			                                          objSkeletonGrid.faces(),
			                                          width,
			                                          height);

		QString fullname = filename + "_";
		fullname += (imageAngles[c].second) ? "top" : QString::number(imageAngles[c].first);
		fullname += +".png";
		projectedGridImage.save(outputDir.filePath(fullname));
	}
}

float maximumNearestDistanceFromGridToGrid(const VoxelGrid& grid, const VoxelGrid& reference)
{
	float maximumNearestDistance = 0.0f;
	
	for (int i = 0; i < grid.voxels().size(); i++)
	{
		const auto query = grid.voxel(grid.voxels()[i]);
		const auto nearestVoxel = reference.nearestVoxel(query);
		const auto dist = query.distanceToPoint(nearestVoxel);

		maximumNearestDistance = std::max(maximumNearestDistance, dist);
	}

	return maximumNearestDistance;
}
