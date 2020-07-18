#pragma once

#include <vector>

#include <QVector3D>

#include "VoxelGrid.h"
#include "AbstractSkeletonBranchClassifier.h"

/**
 * \brief Extract the major connected component from a voxel grid
 *        Voxels in the grid must be sorted, this assumption allows the algorithm to run faster
 * \param grid The voxel grid
 */
VoxelGrid extractMajorConnectedComponent(const VoxelGrid& grid);

/**
 * \brief Extract endpoints in a voxel grid (generally a voxel skeleton)
 *        Endpoints are voxels with only one neighbor
 * \param grid The voxel grid
 * \return A list of endpoints (voxels)
 */
std::vector<Voxel> extractSkeletonEndpoints(const VoxelGrid& grid);

/**
 * \brief Find the lowest (Z axis) endpoint and remove it from the list
 * \param endpoints A list of endpoints (voxels)
 * \return The lowest endpoint
 */
Voxel findAndRemoveLowestEndpoint(std::vector<Voxel>& endpoints);

/**
 * \brief Return all shortest paths from the starting voxel to all endpoints
 *        Use Dijkstra algorithm. Allow connections between two non-neighboring voxels, with a penalty (twice the cost)
 *        Return paths sorted by decreasing length
 * \param grid A grid containing a voxel skeleton
 * \param startingVoxel The voxel where all shortest path start
 * \param endpoints A list of endpoints
 * \return All shortest paths from the starting voxel to all endpoints, and the precedence array of the voxel tree
 */
std::tuple<std::vector<std::vector<Voxel>>, std::vector<int>>
shortestPathsFromSkeleton(const VoxelGrid& grid,
	                      const Voxel& startingVoxel,
	                      std::vector<Voxel>& endpoints);

/**
 * \brief Count the number of common voxels in two paths
 *        The two paths must start with the same voxel
 * \param longerPath The longer of the two paths
 * \param shorterPath The shorter of the two paths 
 * \return The number of common voxels in the two paths
 */
int numberCommonVoxelsInPaths(const std::vector<Voxel>& longerPath, const std::vector<Voxel>& shorterPath);

/**
 * \brief Output only paths in paths that are selected
 * \param paths A list of paths in a voxel grid
 * \param selectedPath A list of booleans: true, keep the path; false, discard the path
 * \return A list of only selected paths
 */
std::vector<std::vector<Voxel>> keepOnlySelectedPaths(const std::vector<std::vector<Voxel>>& paths,
													  std::vector<bool> selectedPath);

/**
 * \brief Filter paths to keep those with the most information
 *        Use a threshold to decide whether a branch should be kept or not
 * \param paths A collection of shortest paths, starting from the same voxel, forming a tree
 * \param pathProportionThreshold Threshold to discard or keep a branch in the skeleton
 * \return Only paths with the most information and an array of boolean saying whether a path in paths are selected. 
 */
std::tuple<std::vector<std::vector<Voxel>>, std::vector<bool>>
filterShortestPathsThreshold(const std::vector<std::vector<Voxel>>& paths, float pathProportionThreshold);

/**
 * \brief Filter paths to keep those with the most information
 *        Use a classifier to decide whether a branch should be kept or not
 * \param paths A collection of shortest paths, starting from the same voxel, forming a tree
 * \param classifier A classifier used to decide whether a branch should be kept or not
 * \return Only paths with the most information and an array of boolean saying whether a path in paths are selected.
 */
std::tuple<std::vector<std::vector<Voxel>>, std::vector<bool>>
filterShortestPathsClassifier(const std::vector<std::vector<Voxel>>& paths,
                              const AbstractSkeletonBranchClassifier& classifier);

/**
 * \brief Segment paths. Any voxel that is shared between at least two paths belongs to the trunk.
 *        Every path is modified to contain only non trunk voxels
 *        Finally, the trunk is added at the back of the path list
 * \param paths A list of shortest paths from the same starting voxel to a collection of endpoints
 * \return The list of segmented path plus all voxels from the trunk in the last position of the list
 */
std::vector<std::vector<Voxel>> segmentShortestPaths(const std::vector<std::vector<Voxel>>& paths);

/**
 * \brief Check that a path has no T junction and it is topologically equivalent to a curve.
 *        The voxel grid must have sorted voxels.
 * \param path A path of voxels from the root
 * \param grid The grid in which the path is defined
 * \param voxelPrecedence List of precedence of voxels in the tree
 * \return True if the path has a T junction, false otherwise
 */
bool checkPathHasNoJunction(const std::vector<Voxel>& path, 
                            const VoxelGrid& grid, 
                            const std::vector<int>& voxelPrecedence);

/**
 * \brief Generate and fill a voxel grid from paths
 * \param initialGrid An initial grid, just used for dimensions and resolutions
 * \param paths A collection of shortest paths
 * \return A grid with all the paths
 */
VoxelGrid generateGridFromSkeleton(const VoxelGrid& initialGrid, const std::vector<std::vector<Voxel>>& paths);

/**
 * \brief Return the 3D coordinates of the nearest voxel to a query point
 * \param grid A voxel grid used to compute coordinates of voxels in the list
 * \param voxels A list of voxels, within the voxel grid
 * \param query A query point
 * \return The 3D coordinates of the nearest voxel
 */
QVector3D nearestVoxel(const VoxelGrid& grid, const std::vector<Voxel>& voxels, const QVector3D& query);

/**
 * \brief Return the index of the nearest path to a query point
 * \param grid A voxel grid used to compute coordinates of voxels in the paths
 * \param paths A list of voxel paths, within the voxel grid
 * \param query A query point
 * \return The index of the nearest path to the query point
 */
int nearestPath(const VoxelGrid& grid, const std::vector<std::vector<Voxel>>& paths, const QVector3D& query);

/**
 * \brief For each voxel in the grid, find the nearest path in the list of paths
 * \param grid A voxel grid
 * \param paths A list of voxel paths, within the voxel grid
 * \return A vector containing for each voxel of the grid, the index of the nearest path
 */
std::vector<int> assignVoxelsToNearestPath(const VoxelGrid& grid, const std::vector<std::vector<Voxel>>& paths);

/**
 * \brief Return the list of the indices of voxels assigned to a particular path
 * \param voxelsPathIndices A vector containing for each voxel of the grid, the index of the nearest path
 * \param pathIndex The index of the selected path
 * \return The list of indices of the voxels assigned to the selected path
 */
std::vector<int> indicesOfVoxels(const std::vector<int>& voxelsPathIndices, int pathIndex);

/**
 * \brief Export a list of paths in a txt file
 * \param paths A list of voxel paths
 * \param filename File in which to save the list of paths
 */
bool exportPaths(const std::vector<std::vector<Voxel>>& paths, const std::string& filename);

/**
 * \brief Import a list of paths in from a txt file
 * \param filename File in which to save the list of paths
 * \return The list of path from the file
 */
std::vector<std::vector<Voxel>> importPaths(const std::string& filename);

/**
 * \brief Export segmented voxels in OBJ files 
 * \param grid The voxel grid that has been previously segmented
 * \param numberOfSegments Number of segments in the segmented skeleton
 * \param voxelSegmentation Segmentation of voxels in the voxel grid
 * \param filepath Path to the folder in which OBJ files are saved
 */
void exportSegmentedLeaves(const VoxelGrid& grid,
	                       int numberOfSegments,
	                       const std::vector<int>& voxelSegmentation,
	                       const std::string& filepath);

/**
 * \brief Export the segmented skeleton in OBJ files
 * \param skeletonGrid The grid in which the skeleton is embedded
 * \param segmentedSkeletonPaths A list of each part of the skeleton
 * \param filepath Path to the folder in which OBJ files are saved
 */
void exportSegmentedSkeleton(const VoxelGrid& skeletonGrid,
	                         const std::vector<std::vector<Voxel>>& segmentedSkeletonPaths,
	                         const std::string& filepath);