#pragma once

#include <QString>

#include <opencv2/core/core.hpp>

/**
 * \brief Rotate an OpenCV image counter clockwise
 * \param image The image to rotate
 * \param angleInDegrees A angle in degrees (positive: counter clockwise, negative: clockwise)
 * \return The rotated image
 */
cv::Mat rotateCounterClockwise(const cv::Mat& image, float angleInDegrees);

/**
 * \brief Rotate an OpenCV image 90 degrees clockwise
 * \param image The image to rotate
 * \return The rotated image
 */
cv::Mat rotate90Clockwise(const cv::Mat& image);

/**
 * \brief Translate an image so that it matches a calibration image
 * \param rawImage The raw image to calibrate
 * \param calibrationImage The calibration image (reference)
 * \param segmentationImage The segmentation mask of the calibration mask
 * \param potLocation Location of the pot in the calibration image
 * \param erasePot Whether or not to erase the pot after calibration
 * \return The translated raw image
 */
cv::Mat calibrateImage(const cv::Mat& rawImage,
	                   const cv::Mat& calibrationImage,
	                   const cv::Mat& segmentationImage,
	                   const cv::Rect& potLocation,
					   bool erasePot);

/**
 * \brief Segment a plant in the image based on a range of HSV colors
 * \param image The image containing the plant
 * \return The image with only the plant on a white background
 */
cv::Mat segmentPlantInImage(const cv::Mat& image);

/**
 * \brief Automatically calibrate an image taken from the side
 * \param input Path to input file
 * \param output Path to output file
 * \return True, if the calibration was successful, False otherwise
 */
bool autoCalibrationSideImage(const QString& input, const QString& output);

/**
 * \brief Automatically calibrate an image taken from the top
 * \param input Path to input file
 * \param output Path to output file
 * \return True, if the calibration was successful, False otherwise
 */
bool autoCalibrationTopImage(const QString& input, const QString& output);
