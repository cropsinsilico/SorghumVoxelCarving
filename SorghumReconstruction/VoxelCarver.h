#pragma once

#include <utility>

#include <QImage>

#include <opencv2/core/core.hpp>

#include "AABB.h"
#include "Camera.h"
#include "VoxelGrid.h"

class VoxelCarver
{
public:
	VoxelCarver(const AABB& boundingBox, int resolutionX, int resolutionY, int resolutionZ);

	/**
	 * \brief The maximum radius around a voxel in which we look for a corresponding pixel
	 * \param maximumRadiusAroundVoxel The new value of maximumRadiusAroundVoxel
	 */
	void setMaximumRadiusAroundVoxel(float maximumRadiusAroundVoxel);

	/**
	 * \brief Add a camera and its associated picture for reconstruction
	 * \param camera The settings of the camera
	 * \param image The picture taken by the camera
	 * \param offsetX Add a X axis pixel offset when reading the image (for calibration)
	 * \param offsetY Add a Y axis pixel offset when reading the image (for calibration)
	 */
	void addCameraImage(const Camera& camera, const cv::Mat& image, int offsetX = 0, int offsetY = 0);

	/**
	 * \brief Delete all cameras in the voxel carver
	 */
	void clearCameras();

	/**
	 * \brief Check whether a query point is inside the object hull
	 * In other words, check that the projection of this point appears on all cameras
	 * \param point The query point
	 * \return True if the point is part of the voxel grid, false otherwise
	 */
	bool isInside(const QVector3D& point) const;

	/**
	 * \brief Carve the voxel grid according to the images taken by the cameras
	 */
	void process();

	/**
	 * \brief Return True if the voxel grid is ready to be used
	 * \return True after space has been successfully carved, false otherwise
	 */
	bool isVoxelGridReady() const;

	/**
	 * \brief Return the voxel grid after space has been carved.
	 * \return The voxel grid.
	 */
	const VoxelGrid& voxelGrid() const;

	/**
	 * \brief Re-project voxels on images and compute the Dice coefficient
	 * \param grid The grid on which to compute the error
	 * \param reprojections Output the re-projections 
	 * \return The Dice coefficient of the re-projections
	 */
	float reprojectionError(const VoxelGrid& grid, std::vector<cv::Mat>& reprojections) const;

	/**
	 * \brief Re-project voxels on images and compute the Dice coefficient
	 * \param grid The grid on which to compute the error
	 * \return The Dice coefficient of the re-projections
	 */
	float reprojectionError(const VoxelGrid& grid) const;
	
private:
	/**
	 * \brief A struct holding a camera and the picture associated to it
	 */
	struct CameraImage
	{
		Camera camera;
		cv::Mat image;
		int offsetX;
		int offsetY;

		CameraImage(const Camera& camera, cv::Mat image) :
			camera(camera),
			image(std::move(image)),
			offsetX(0),
			offsetY(0)
		{
			
		}

		CameraImage(const Camera& camera, cv::Mat image, int offsetX, int offsetY) :
			camera(camera),
			image(std::move(image)),
			offsetX(offsetX),
			offsetY(offsetY)
		{

		}
	};

	/**
	 * \brief Check that a pixel is in the object when voxel carving
	 * \param image The image in which to lookup the pixel
	 * \param i The pixel row
	 * \param j The pixel column
	 * \return True if the pixel is in the object
	 */
	bool isPixelInObject(const cv::Mat& image, int i, int j) const;

	/**
	 * \brief Threshold on the value (like in HSV color model) used to discriminate object from background
	 */
	int m_colorThreshold;

	/**
	 * \brief When running the voxel carving algorithm, this is the maximum radius
	 *        around a voxel in which we look for a corresponding pixel.
	 */
	float m_maximumRadiusAroundVoxel;

	/**
	 * \brief True after space has been successfully carved
	 * Means that the voxel grid is ready to use and consistent with cameras
	 */
	bool m_voxelGridReady;
	
	/**
	 * \brief The generated voxel grid
	 */
	VoxelGrid m_voxelGrid;
	
	/**
	 * \brief A list of camera and their associated pictures
	 */
	std::vector<CameraImage> m_cameras;
};
