#include "VoxelCarver.h"

#include <omp.h>

#include <chrono>

#include <QDebug>
#include <QtMath>

#include <opencv2/imgproc/imgproc.hpp>

#include "MathUtils.h"
#include "Thinning.h"

VoxelCarver::VoxelCarver(const AABB& boundingBox, int resolutionX, int resolutionY, int resolutionZ) :
	m_colorThreshold(235),
	m_maximumRadiusAroundVoxel(0.0f),
	m_voxelGrid(boundingBox, resolutionX, resolutionY, resolutionZ),
	m_voxelGridReady(false)
{
	
}

void VoxelCarver::setMaximumRadiusAroundVoxel(float maximumRadiusAroundVoxel)
{
	m_maximumRadiusAroundVoxel = maximumRadiusAroundVoxel;
}

void VoxelCarver::addCameraImage(const Camera& camera, const cv::Mat& image, int offsetX, int offsetY)
{
	m_cameras.emplace_back(camera, image, offsetX, offsetY);

	m_voxelGridReady = false;
}

void VoxelCarver::clearCameras()
{
	m_cameras.clear();
}

bool VoxelCarver::isInside(const QVector3D& point) const
{
	bool inside = true;
	
	for (const auto& cameraImage : m_cameras)
	{
		// Dimensions of the image
		const auto width = float(cameraImage.image.cols);
		const auto height = float(cameraImage.image.rows);

		// Diagonal point in the image plane
		const auto diagonalPoint = point + m_maximumRadiusAroundVoxel * (cameraImage.camera.up() + cameraImage.camera.right()) * M_SQRT1_2;
		
		// Real coordinates of the pixel on the image
		const auto realPixel = cameraImage.camera.project(point, width, height);
		// Real coordinates of the pixel on the image
		const auto realDiagonalPixel = cameraImage.camera.project(diagonalPoint, width, height);

		// Size of the neighborhood around the current pixel
		const auto offset = int(std::ceil(realPixel.distanceToPoint(realDiagonalPixel)));

		// Look at neighboring positions
		// If at least one pixel from the neighborhood is present in the image, the voxel is inside
		bool pixelExists = false;
		for (int offsetX = -offset; offsetX <= offset && !pixelExists; offsetX++)
		{
			for (int offsetY = -offset; offsetY <= offset && !pixelExists; offsetY++)
			{
				const auto x = int(std::round(realPixel.x() + float(offsetX) + float(cameraImage.offsetX)));
				const auto y = int(std::round(realPixel.y() + float(offsetY) + float(cameraImage.offsetY)));

				// If the point is visible in the image
				if (x >= 0 && y >= 0 && x < cameraImage.image.cols && y < cameraImage.image.rows)
				{					
					// If the corresponding pixel in the image is not white, the point is part the object
					if (isPixelInObject(cameraImage.image, y, x))
					{
						pixelExists = true;
					}
				}
				else
				{
					// If the pixel is not visible in the image, we consider it as present
					// so that we don't remove it even if it is visible from other views
					pixelExists = true;
				}
			}
		}
		
		// The voxel is not visible from this camera, therefore it is not part of the object
		if (!pixelExists)
		{
			inside = false;
			break;
		}
	}

	return inside;
}


void VoxelCarver::process()
{
	const int defaultVoxelReserve = m_voxelGrid.resolutionY() * m_voxelGrid.resolutionZ();
	
	m_voxelGrid.clear();

	const auto start = std::chrono::high_resolution_clock::now();

	// Add voxels from each threads in a separate list to avoid critical sections
	std::vector<std::vector<Voxel>> voxels(omp_get_max_threads());
	// Reserve some memory to avoid too many re-allocations
	for (auto v : voxels)
	{
		v.reserve(m_voxelGrid.resolutionX());
	}

	#pragma omp parallel for schedule(static)
	for (int x = 0; x < m_voxelGrid.resolutionX(); x++)
	{
		for (int y = 0; y < m_voxelGrid.resolutionY(); y++)
		{
			for (int z = 0; z < m_voxelGrid.resolutionZ(); z++)
			{
				const auto point = m_voxelGrid.voxel(x, y ,z);

				if (isInside(point))
				{
					const auto currentThread = omp_get_thread_num();
					voxels[currentThread].emplace_back(x, y, z);
				}
			}
		}
	}

	// Add all the voxels from the sub lists to the grid
	for (const auto& threadVoxels : voxels)
	{
		for (const auto& voxel : threadVoxels)
		{
			m_voxelGrid.add(voxel.x, voxel.y, voxel.z);
		}
	}

	const auto end = std::chrono::high_resolution_clock::now();

	// Calculating total time taken by the program. 
	const double elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	qInfo() << "Voxel carving time: " << elapsedTime << " ms";

	m_voxelGrid.sortVoxels();

	m_voxelGridReady = true;
}

bool VoxelCarver::isVoxelGridReady() const
{
	return m_voxelGridReady;
}

const VoxelGrid& VoxelCarver::voxelGrid() const
{
	return m_voxelGrid;
}

float VoxelCarver::reprojectionError(const VoxelGrid& grid, std::vector<cv::Mat>& reprojections) const
{	
	// Convert the grid to voxel cubes
	const auto objGrid = grid.getVoxelsAsOBJ(true);

	// Colors (in BGR order) for true positive, false positive, etc.
	const cv::Vec3b tpColor(0, 255, 0); // Green
	const cv::Vec3b fpColor(255, 0, 0); // Blue
	const cv::Vec3b fnColor(0, 0, 255); // Red
	const cv::Vec3b tnColor(255, 255, 255); // White

	// Initialize reprojection maps with white images (true negative color)
	reprojections.clear();
	for (const auto& camera : m_cameras)
	{
		reprojections.emplace_back(camera.image.rows,
		                           camera.image.cols,
		                           camera.image.type(),
		                           tnColor);
	}

	long long truePositives = 0;
	long long falsePositives = 0;
	long long falseNegatives = 0;

	#pragma omp parallel for shared(truePositives, falsePositives, falseNegatives)
	for (int c = 0; c < m_cameras.size(); c++)
	{
		const auto cameraImage = m_cameras[c];
		
		const auto width = cameraImage.image.cols;
		const auto height = cameraImage.image.rows;
		
		// Re project voxel cubes on this camera
		const auto projectedGridImage = cameraImage.camera.render(QMatrix4x4(),
			                                                      objGrid.vertices(),
			                                                      objGrid.faces(),
			                                                      width,
			                                                      height);

		// Convert to OpenCV format to be consistent with the image masks
		const auto projectedGrid = convertToCvMat(projectedGridImage);
		
		// Compute the Dice coefficient
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				// Reference
				const auto mask = isPixelInObject(cameraImage.image, i, j);
				// Prediction
				const auto projection = isPixelInObject(projectedGrid, i, j);

				// Present in the prediction and in the reference
				if (projection && mask)
				{
					#pragma omp atomic
					truePositives++;

					// Update the reprojection image
					reprojections[c].at<cv::Vec3b>(i, j) = tpColor;
				}
				// Present in the prediction but not in the reference
				else if (projection && !mask)
				{
					#pragma omp atomic
					falsePositives++;

					// Update the reprojection image
					reprojections[c].at<cv::Vec3b>(i, j) = fpColor;
				}
				// Not present in the prediction but present in the reference
				else if (!projection && mask)
				{
					#pragma omp atomic
					falseNegatives++;

					// Update the reprojection image
					reprojections[c].at<cv::Vec3b>(i, j) = fnColor;
				}
			}
		}
	}

	const auto diceCoefficient = 2.0f * float(truePositives) /
		(2.0f * float(truePositives) + float(falsePositives) + float(falseNegatives));

	return diceCoefficient;
}

float VoxelCarver::reprojectionError(const VoxelGrid& grid) const
{
	std::vector<cv::Mat> reprojections;
	return reprojectionError(grid, reprojections);
}

bool VoxelCarver::isPixelInObject(const cv::Mat& image, int i, int j) const
{
	// Read the color of the pixel in the image
	const auto pixel = image.at<cv::Vec3b>(i, j);

	// The image is in BGR format
	const QColor color(pixel.val[2], pixel.val[1], pixel.val[0]);

	// If the corresponding pixel in the image is not white, the point is part the object
	return color.value() < m_colorThreshold;
}
