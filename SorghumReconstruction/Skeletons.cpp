#include "Skeletons.h"

#include <fstream>
#include <queue>

#include "MathUtils.h"
#include "UnionFind.h"

VoxelGrid extractMajorConnectedComponent(const VoxelGrid& grid)
{
	const auto nbVoxels = grid.voxels().size();

	UnionFind unionFind(nbVoxels);

	// Union voxels based on connectivity
	for (const auto& voxel : grid.voxels())
	{
		// Coordinates of the current voxel
		const auto x = voxel.x;
		const auto y = voxel.y;
		const auto z = voxel.z;

		const auto iStart = std::max(0, x - 1);
		const auto iEnd = std::min(grid.resolutionX() - 1, x + 1);
		const auto jStart = std::max(0, y - 1);
		const auto jEnd = std::min(grid.resolutionY() - 1, y + 1);
		const auto kStart = std::max(0, z - 1);
		const auto kEnd = std::min(grid.resolutionZ() - 1, z + 1);

		// Look at 26-connected neighbors
		for (int i = iStart; i <= iEnd; i++)
		{
			for (int j = jStart; j <= jEnd; j++)
			{
				for (int k = kStart; k <= kEnd; k++)
				{
					// If it is a neighbor
					if (i != x || j != y || k != z)
					{
						if (grid.hasVoxel(i, j, k))
						{
							// Find the index of the voxel in the union find list
							const auto firstIndex = grid.voxelNumber(x, y, z);
							const auto secondIndex = grid.voxelNumber(i, j, k);

							// Union between (x, y, z) and (i, j, k)
							unionFind.unionNodes(firstIndex, secondIndex);
						}
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

	// Add every voxels to their connected components
	for (unsigned int i = 0; i < nbVoxels; i++)
	{
		const auto root = unionFind.findNode(i);

		// If the node is not a root
		if (root != i)
		{
			// Search for his connected component
			const auto it = std::lower_bound(connectedComponentsRoots.begin(),
				                             connectedComponentsRoots.end(), root);

			if (it != connectedComponentsRoots.end())
			{
				const int connectedComp = std::distance(connectedComponentsRoots.begin(), it);
				connectedComponents[connectedComp].push_back(i);
			}
		}
	}

	// Find the major connected component
	unsigned int majorComponent = 0;
	for (unsigned int i = 1; i < connectedComponents.size(); i++)
	{
		if (connectedComponents[i].size() > connectedComponents[majorComponent].size())
		{
			majorComponent = i;
		}
	}

	// Setup a new grid with the major connected component
	VoxelGrid majorComponentGrid(grid.boundingBox(), grid.resolutionX(), grid.resolutionY(), grid.resolutionZ());

	if (!connectedComponents.empty())
	{
		for (const auto& voxelIndex : connectedComponents[majorComponent])
		{
			const auto& voxel = grid.voxels()[voxelIndex];

			majorComponentGrid.add(voxel.x, voxel.y, voxel.z);
		}
	}

	return majorComponentGrid;
}

std::vector<Voxel> extractSkeletonEndpoints(const VoxelGrid& grid)
{
	std::vector<Voxel> endpoints;

	// Look at every voxels and count the number of voxels in the 26-connected neighborhood
#pragma omp parallel for
	for (int i = 0; i < grid.voxels().size(); i++)
	{
		// The current voxel
		const auto& voxel = grid.voxels()[i];

		// Number of neighbors of the current voxel
		const auto numberNeighbors = grid.numberNeighbors(voxel);

		// If the current voxel has exactly 1 neighbor, it's an end point
		if (numberNeighbors <= 1)
		{
			// Keep this voxel
#pragma omp critical(extract_end_points)
			{
				endpoints.emplace_back(voxel.x, voxel.y, voxel.z);
			}
		}
	}

	// Generate a new grid with endpoints
	VoxelGrid endpointsGrid(grid.boundingBox(), grid.resolutionX(), grid.resolutionY(), grid.resolutionZ());

	for (const auto& voxel : endpoints)
	{
		endpointsGrid.add(voxel.x, voxel.y, voxel.z);
	}

	return endpoints;
}

Voxel findAndRemoveLowestEndpoint(std::vector<Voxel>& endpoints)
{
	int lowestEndpoint = 0;
	for (unsigned int i = 1; i < endpoints.size(); i++)
	{
		if (endpoints[i].z < endpoints[lowestEndpoint].z)
		{
			lowestEndpoint = i;
		}
	}

	// Keep the root and remove it from the list of endpoints
	const auto rootVoxel = endpoints[lowestEndpoint];
	endpoints.erase(endpoints.begin() + lowestEndpoint);

	return rootVoxel;
}

std::tuple<std::vector<std::vector<Voxel>>, std::vector<int>>
shortestPathsFromSkeleton(
	const VoxelGrid& grid,
	const Voxel& startingVoxel,
	std::vector<Voxel>& endpoints)
{
	const auto nbVoxels = grid.voxels().size();

	using intPair = std::pair<int, int>;

	std::priority_queue<intPair, std::vector<intPair>, std::greater<intPair>> queue;

	std::vector<int> dist(nbVoxels, std::numeric_limits<int>::max());
	std::vector<int> precedent(nbVoxels, -1);

	// The starting voxel is the first to be visited
	const auto startingVoxelNumber = grid.voxelNumber(startingVoxel.x, startingVoxel.y, startingVoxel.z);
	dist[startingVoxelNumber] = 0;
	queue.emplace(0, startingVoxelNumber);

	// Dijkstra algorithm
	while (!queue.empty())
	{
		// Current voxel
		const auto currentVoxelDistance = queue.top().first;
		const auto currentVoxelNumber = queue.top().second;
		const auto currentVoxel = grid.voxels()[currentVoxelNumber];
		queue.pop();

		// Test if the pair in the priority queue is up to date
		if (currentVoxelDistance > dist[currentVoxelNumber])
		{
			// This voxel in the priority queue is outdated, go to the next one
			continue;
		}

		// Look for 26-connected neighbors around the current voxel
		const int connectionPattern = 24;
		for (int i = -connectionPattern; i <= connectionPattern; i++)
		{
			for (int j = -connectionPattern; j <= connectionPattern; j++)
			{
				for (int k = -connectionPattern; k <= connectionPattern; k++)
				{
					// If it is a neighbor, and not the current voxel
					if (i != 0 || j != 0 || k != 0)
					{
						const auto x = currentVoxel.x + i;
						const auto y = currentVoxel.y + j;
						const auto z = currentVoxel.z + k;

						if (grid.hasVoxel(x, y, z))
						{
							// Find the index of the voxel
							const auto neighborNumber = grid.voxelNumber(x, y, z);

							// Distance between the current voxel and the neighbor (infinity norm)
							int distNeighbor = std::max({ std::abs(i), std::abs(j), std::abs(k) });

							// If the neighbor is not in the 26-connected neighborhood, we add a penalty
							if (distNeighbor > 1)
							{
								// Compute the Manhattan distance
								const int manhattanDist = std::abs(i) + std::abs(j) + std::abs(k);
								// Get the penalty according to the distance
								distNeighbor = (manhattanDist * (manhattanDist + 1)) / 2;
							}

							if (dist[neighborNumber] > currentVoxelDistance + distNeighbor)
							{
								dist[neighborNumber] = currentVoxelDistance + distNeighbor;
								precedent[neighborNumber] = currentVoxelNumber;
								queue.emplace(dist[neighborNumber], neighborNumber);
							}
						}
					}
				}
			}
		}
	}

	// Check that every endpoint of the precedence graph has been identified. If necessary add new endpoints
	std::vector<int> numberOfSuccessors(nbVoxels, 0);
	for (unsigned int i = 0; i < precedent.size(); i++)
	{
		const auto pred = precedent[i];
		// If the current voxel is not the root
		if (pred >= 0 && pred < nbVoxels)
		{
			numberOfSuccessors[pred]++;
		}
	}
	// If a voxel has no successor, it is on the border of the graph
	for (unsigned int i = 0; i < numberOfSuccessors.size(); i++)
	{
		// If the voxel has no successor, it's on the border of the skeleton
		if (numberOfSuccessors[i] == 0)
		{
			const auto voxel = grid.voxels()[i];

			// Number of neighbors of the current voxel
			const auto numberNeighbors = grid.numberNeighbors(voxel);

			// If the current voxel has at most 2 neighbors, it may be an endpoint we previously missed
			if (numberNeighbors <= 2)
			{
				// We check that it's not the startingVoxel and it's not in the list of endpoints
				const auto itEndpoint = std::find(endpoints.begin(), endpoints.end(), voxel);
				if (voxel != startingVoxel && itEndpoint == endpoints.end())
				{
					// Add this voxel to the list of endpoints
					endpoints.push_back(voxel);
				}
			}
		}
	}

	// Store paths from endpoints to the root in a vector
	std::vector<std::vector<Voxel>> paths(endpoints.size());

	// Start from each endpoint and add the path to the root
	for (int i = 0; i < endpoints.size(); i++)
	{
		int currentVoxelNumber = grid.voxelNumber(endpoints[i].x, endpoints[i].y, endpoints[i].z);

		// Register this voxel
		paths[i].push_back(endpoints[i]);

		while (precedent[currentVoxelNumber] != -1)
		{
			currentVoxelNumber = precedent[currentVoxelNumber];

			// Register this voxel
			const auto& v = grid.voxels()[currentVoxelNumber];
			paths[i].push_back(v);
		}
	}

	// Sort paths by decreasing length
	std::sort(paths.begin(), paths.end(),
		[](const std::vector<Voxel>& a, const std::vector<Voxel>& b)
		{
			return b.size() < a.size();
		});
	// Reverse paths so that they are stored from the root to the endpoint
	for (auto& path : paths)
	{
		std::reverse(path.begin(), path.end());
	}
	// Delete paths with only one voxel (the endpoint could not be connected to the root)
	paths.erase(std::remove_if(paths.begin(), paths.end(),
				[](const std::vector<Voxel>& p)
				{
					// Remove if the path contains only one voxel
					return p.size() <= 1;
				}), paths.end());

	return { paths, precedent };
}

int numberCommonVoxelsInPaths(const std::vector<Voxel>& longerPath, const std::vector<Voxel>& shorterPath)
{
	int lastCommonVoxel = 0;
	while (lastCommonVoxel < longerPath.size()
		&& lastCommonVoxel < shorterPath.size()
		&& longerPath[lastCommonVoxel] == shorterPath[lastCommonVoxel])
	{
		lastCommonVoxel++;
	}

	return lastCommonVoxel;
}

std::vector<std::vector<Voxel>> keepOnlySelectedPaths(
	const std::vector<std::vector<Voxel>>& paths,
	std::vector<bool> selectedPath)
{
	assert(paths.size() == selectedPath.size());
	
	std::vector<std::vector<Voxel>> filteredPaths;
	
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		// If this path is not discarded
		if (selectedPath[i])
		{
			filteredPaths.push_back(paths[i]);
		}
	}

	return filteredPaths;
}

std::tuple<std::vector<std::vector<Voxel>>, std::vector<bool>>
filterShortestPathsThreshold(const std::vector<std::vector<Voxel>>& paths, float pathProportionThreshold)
{
	std::vector<bool> selectedPath(paths.size(), true);

	for (unsigned int i = 0; i < paths.size(); i++)
	{
		// Take a path and potentially discard other paths that are shorter
		const auto& longerPath = paths[i];

		// If this path is discarded, we don't consider it
		if (!selectedPath[i])
		{
			continue;
		}

		// Check all other paths
		for (unsigned int j = i + 1; j < paths.size(); j++)
		{
			// If this path is discarded, we don't consider it
			if (!selectedPath[j])
			{
				continue;
			}

			// The current path, that we compare against the longest path
			const auto& shorterPath = paths[j];

			const auto lastCommonVoxel = numberCommonVoxelsInPaths(longerPath, shorterPath);
			const float shorterPathProportion = float(lastCommonVoxel) / float(shorterPath.size());

			// If this path does not bring much information, discard it
			if (shorterPathProportion > pathProportionThreshold)
			{
				selectedPath[j] = false;
			}
		}
	}

	const auto filteredPaths = keepOnlySelectedPaths(paths, selectedPath);

	return { filteredPaths, selectedPath };
}

std::tuple<std::vector<std::vector<Voxel>>, std::vector<bool>> filterShortestPathsClassifier(
	const std::vector<std::vector<Voxel>>& paths,
	const AbstractSkeletonBranchClassifier& classifier)
{
	std::vector<bool> selectedPath(paths.size(), true);

	for (unsigned int i = 0; i < paths.size(); i++)
	{
		// Take a path and potentially discard other paths that are shorter
		const auto& longerPath = paths[i];

		// If this path is discarded, we don't consider it
		if (!selectedPath[i])
		{
			continue;
		}

		// Check all other paths
		for (unsigned int j = i + 1; j < paths.size(); j++)
		{
			// If this path is discarded, we don't consider it
			if (!selectedPath[j])
			{
				continue;
			}

			// The current path, that we compare against the longest path
			const auto& shorterPath = paths[j];

			// Use the classifier to decide whether the shorter path should be discarded or not
			if (!classifier.predict(longerPath, shorterPath))
			{
				// Discard the shorter path
				selectedPath[j] = false;
			}
		}
	}

	const auto filteredPaths = keepOnlySelectedPaths(paths, selectedPath);

	return { filteredPaths, selectedPath };
}

std::vector<std::vector<Voxel>> segmentShortestPaths(const std::vector<std::vector<Voxel>>& paths)
{
	// Any voxel that is shared between at least two paths belongs to the trunk

	// Compute the histogram of voxels
	std::map<Voxel, int> histogram;
	for (const auto& path : paths)
	{
		for (const auto& voxel : path)
		{
			// Look for the voxel in the histogram
			auto search = histogram.find(voxel);

			if (search != histogram.end())
			{
				// If the voxel is already present in the histogram, simply increment the frequency
				search->second++;
			}
			else
			{
				// If the voxel is not yet present in the histogram
				histogram.emplace(voxel, 1);
			}
		}
	}

	// Voxels associated to the trunk
	std::vector<Voxel> trunkVoxels;

	// List all voxels and frequencies
	for (const auto& histBin : histogram)
	{
		const auto& voxel = histBin.first;
		const auto& frequency = histBin.second;

		if (frequency > 1)
		{
			// This voxel belongs to the trunk
			trunkVoxels.push_back(voxel);
		}
	}

	// TODO: Keep only the longest part of the trunk without a T junction and keep leaves with common voxels

	// Sort this array to accelerate further searches (it should already be sorted)
	std::sort(trunkVoxels.begin(), trunkVoxels.end());

	// Copy and change the paths
	auto segmentedPaths = paths;

	// For each path, remove all voxels from the trunk
	for (auto& path : segmentedPaths)
	{
		path.erase(
			std::remove_if(path.begin(), path.end(), [&trunkVoxels](const Voxel& v)
				{
					// If this voxel is in the trunk, remove it
					return std::binary_search(trunkVoxels.begin(), trunkVoxels.end(), v);
				}),
			path.end());
	}

	// Sort again ordered by increasing altitude (Z)
	std::sort(trunkVoxels.begin(), trunkVoxels.end(), Voxel::compareZXY);

	// Add the trunk at the end of the vector
	segmentedPaths.push_back(trunkVoxels);

	return segmentedPaths;
}

bool checkPathHasNoJunction(
	const std::vector<Voxel>& path,
	const VoxelGrid& grid,
	const std::vector<int>& voxelPrecedence)
{
	std::vector<int> numberOfSuccessors(path.size(), 0);
	
	for (unsigned int i = 0; i < path.size(); i++)
	{
		const auto voxel = path[i];

		// Find the number of the current voxel
		const auto currentVoxelNumber = grid.voxelNumber(voxel.x, voxel.y, voxel.z);
		assert(currentVoxelNumber >= 0 && currentVoxelNumber < voxelPrecedence.size());
		
		// Find the number of its predecessor, if it exists, using the voxelPrecedence array
		const auto predecessorNumber = voxelPrecedence[currentVoxelNumber];
		if (predecessorNumber >= 0 && predecessorNumber < grid.voxels().size())
		{
			// Find the position of the predecessor voxel
			const auto predecessorVoxel = grid.voxels()[predecessorNumber];

			// Find this voxel in the path
			const auto itVoxel = std::find(path.begin(), path.end(), predecessorVoxel);
			if (itVoxel != path.end())
			{
				// If it exists
				const auto indexInPath = std::size_t(std::distance(path.begin(), itVoxel));
				assert(indexInPath < numberOfSuccessors.size());

				// If it's not the current voxel pointing to itself
				// increment the number of voxels pointing on it
				if (indexInPath != i)
				{
					numberOfSuccessors[indexInPath]++;
				}
			}
		}
	}

	// If all the voxels have at most one voxel pointing at it as a predecessor, the path has no T junction
	for (unsigned int i = 0; i < numberOfSuccessors.size(); i++)
	{
		if (numberOfSuccessors[i] > 1)
		{
			return false;
		}
	}
	
	return true;
}

VoxelGrid generateGridFromSkeleton(const VoxelGrid& initialGrid, const std::vector<std::vector<Voxel>>& paths)
{
	// Generate a new grid with shortest paths
	VoxelGrid pathsGrid(
		initialGrid.boundingBox(),
		initialGrid.resolutionX(),
		initialGrid.resolutionY(),
		initialGrid.resolutionZ());

	for (const auto& path : paths)
	{
		for (const auto& voxel : path)
		{
			pathsGrid.add(voxel.x, voxel.y, voxel.z);
		}
	}

	return pathsGrid;
}

QVector3D nearestVoxel(const VoxelGrid& grid, const std::vector<Voxel>& voxels, const QVector3D& query)
{
	assert(!voxels.empty());

	float minimumDistanceSq = std::numeric_limits<float>::max();
	QVector3D nearestPoint;

	for (const auto& v : voxels)
	{
		const auto point = grid.voxel(v);

		const float distSq = distanceSquared(query, point);

		if (distSq < minimumDistanceSq)
		{
			minimumDistanceSq = distSq;
			nearestPoint = point;
		}
	}

	return nearestPoint;
}

int nearestPath(const VoxelGrid& grid, const std::vector<std::vector<Voxel>>& paths, const QVector3D& query)
{
	assert(!paths.empty());

	float minimumDistanceSq = std::numeric_limits<float>::max();
	int nearestPath = -1;

	for (int i = 0; i < paths.size(); i++)
	{
		// Nearest voxel on this path
		const auto point = nearestVoxel(grid, paths[i], query);
		// Compute the squared distance
		const auto distSq = distanceSquared(query, point);

		if (distSq < minimumDistanceSq)
		{
			minimumDistanceSq = distSq;
			nearestPath = i;
		}
	}

	return nearestPath;
}

std::vector<int> assignVoxelsToNearestPath(const VoxelGrid& grid, const std::vector<std::vector<Voxel>>& paths)
{
	const auto& voxels = grid.voxels();

	std::vector<int> nearestPathPerVoxel(voxels.size(), -1);

#pragma omp parallel for
	for (int i = 0; i < voxels.size(); i++)
	{
		// 3D coordinates of this voxel
		const auto point = grid.voxel(voxels[i]);

		// What is the nearest path to this voxel
		nearestPathPerVoxel[i] = nearestPath(grid, paths, point);
	}

	return nearestPathPerVoxel;
}

std::vector<int> indicesOfVoxels(const std::vector<int>& voxelsPathIndices, int pathIndex)
{
	std::vector<int> indices;

	// We provision more space than needed to avoid 
	indices.reserve(voxelsPathIndices.size());
	
	for (int i = 0; i < voxelsPathIndices.size(); i++)
	{
		if (voxelsPathIndices[i] == pathIndex)
		{
			indices.push_back(i);
		}
	}

	// Shrink
	indices.shrink_to_fit();

	return indices;
}

bool exportPaths(const std::vector<std::vector<Voxel>>& paths, const std::string& filename)
{
	// Write vertices to a file
	std::ofstream file(filename, std::fstream::out);

	if (!file.is_open())
	{
		return false;
	}

	// Write the total number of paths
	file << paths.size() << std::endl;
	
	for (const auto& path : paths)
	{
		// Write the length of the current path
		file << path.size() << std::endl;

		// Write each voxel from the path
		for (const auto& voxel : path)
		{
			file << voxel.x << " " << voxel.y << " " << voxel.z << std::endl;
		}
	}

	file.close();

	return true;
}

std::vector<std::vector<Voxel>> importPaths(const std::string& filename)
{
	// Read file
	std::ifstream file(filename);

	std::vector<std::vector<Voxel>> paths;
	
	if (file.is_open())
	{
		int numberOfPaths;
		file >> numberOfPaths;

		paths.resize(numberOfPaths);

		// Read each path
		for (int i = 0; i < numberOfPaths; i++)
		{
			int voxelsInPath;
			file >> voxelsInPath;
			
			// Read each voxel in the current path
			for (int j = 0; j < voxelsInPath; j++)
			{
				int x, y, z;
				file >> x >> y >> z;

				paths[i].emplace_back(x, y, z);
			}
		}
	}

	file.close();

	return paths;
}

void exportSegmentedLeaves(
	const VoxelGrid& grid,
	int numberOfSegments,
	const std::vector<int>& voxelSegmentation,
	const std::string& filepath)
{
	// For each leaf, including the trunk, segment voxels
	VoxelGrid segmentedGrid(grid.boundingBox(), grid.resolutionX(), grid.resolutionY(), grid.resolutionZ());

	for (int i = 0; i < numberOfSegments; i++)
	{
		segmentedGrid.clear();

		// Keep only voxels assigned to this branch
		for (unsigned int j = 0; j < grid.voxels().size(); j++)
		{
			if (voxelSegmentation[j] == i)
			{
				const auto voxel = grid.voxels()[j];
				segmentedGrid.add(voxel.x, voxel.y, voxel.z);
			}
		}

		segmentedGrid.saveVoxelsAsOBJ(filepath + "\\leaf_" + std::to_string(i + 1) + "_voxels.obj");
	}
}

void exportSegmentedSkeleton(
	const VoxelGrid& skeletonGrid,
	const std::vector<std::vector<Voxel>>& segmentedSkeletonPaths,
	const std::string& filepath)
{
	for (unsigned int i = 0; i < segmentedSkeletonPaths.size(); i++)
	{
		std::vector<std::vector<Voxel>> uniquePath(1, segmentedSkeletonPaths[i]);
		const auto grid = generateGridFromSkeleton(skeletonGrid, uniquePath);
		grid.saveAsOBJ(filepath + "\\leaf_" + std::to_string(i + 1) + "_skeleton.obj");
	}
}
