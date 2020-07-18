#include "Calibration.h"

#include <QtDebug>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Thinning.h"

cv::Mat equalizeColorImageHistogram(const cv::Mat& image)
{
	// Convert the image from BGR to YCrCb color space
	cv::Mat histEqualizedImage;
	cvtColor(image, histEqualizedImage, cv::COLOR_BGR2YCrCb);

	// Split the image into 3 channels; Y, Cr and Cb channels respectively and store it in a std::vector
	std::vector<cv::Mat> vecChannels;
	split(histEqualizedImage, vecChannels);

	// Equalize the histogram of only the Y channel
	equalizeHist(vecChannels[0], vecChannels[0]);

	// Merge 3 channels in the vector to form the color image in YCrCB color space.
	merge(vecChannels, histEqualizedImage);

	// Convert the histogram equalized image from YCrCb to BGR color space again
	cvtColor(histEqualizedImage, histEqualizedImage, cv::COLOR_YCrCb2BGR);

	return histEqualizedImage;
}

cv::Mat rotateCounterClockwise(const cv::Mat& image, float angleInDegrees)
{
	const cv::Scalar white(255.0, 255.0, 255.0, 255.0);
	const cv::Point2f center(float(image.cols) / 2, float(image.rows) / 2);
	const cv::Mat rotationMat = cv::getRotationMatrix2D(center, angleInDegrees, 1.0);

	cv::Mat rotatedImage;
	cv::warpAffine(image,
		rotatedImage,
		rotationMat,
		image.size(),
		cv::INTER_LINEAR,
		cv::BORDER_CONSTANT,
		white);

	return rotatedImage;
}

cv::Mat rotate90Clockwise(const cv::Mat& image)
{
	return rotateCounterClockwise(image, -90.0);
}

// TODO: Add a cv::Rect at the location of the pot
cv::Mat calibrateImage(
	const cv::Mat& rawImage,
	const cv::Mat& calibrationImage,
	const cv::Mat& segmentationImage,
	const cv::Rect& potLocation,
	bool erasePot)
{
	// Convert to grayscale
	cv::Mat grayCalibratedImage, grayRawImage;
	cv::cvtColor(calibrationImage, grayCalibratedImage, cv::COLOR_BGR2GRAY);
	cv::cvtColor(rawImage, grayRawImage, cv::COLOR_BGR2GRAY);

	// Convert to real values
	cv::Mat realGrayCalibratedImage, realGrayRawImage;
	grayCalibratedImage.convertTo(realGrayCalibratedImage, CV_32FC1, 1.0 / 255.0);
	grayRawImage.convertTo(realGrayRawImage, CV_32FC1, 1.0 / 255.0);

	// Find the translation between the two images
	const cv::Point2d result = phaseCorrelate(realGrayRawImage, realGrayCalibratedImage);

	// Translate the image to get an approximate calibration
	cv::Mat translationMat = (cv::Mat_<double>(2, 3) << 1, 0, result.x, 0, 1, result.y);
	cv::Mat translatedRawImage;
	const cv::Scalar white(255.0, 255.0, 255.0, 255.0);
	cv::warpAffine(rawImage,
		           translatedRawImage,
		           translationMat,
		           rawImage.size(),
		           cv::INTER_LINEAR,
		           cv::BORDER_CONSTANT,
		           white);

	// Use coords of the pot from the calibration image
	const auto potImage = calibrationImage(potLocation);

	// Find the pot in the almost calibrated image (only search in the region where the pot should be)
	cv::Rect potRoi(potLocation.x - potLocation.width / 2,
		            potLocation.y - potLocation.height / 2,
		            2 * potLocation.width,
		            2 * potLocation.height);
	cv::Mat matchingResult;
	cv::matchTemplate(translatedRawImage(potRoi), potImage, matchingResult, cv::TM_CCOEFF_NORMED);
	double minVal, maxVal;
	cv::Point minLoc, maxLoc;
	cv::minMaxLoc(matchingResult, &minVal, &maxVal, &minLoc, &maxLoc);
	// Location of the pot in the almost calibrated image
	const auto potInTranslatedRawImage = maxLoc + potRoi.tl();

	// Draw rectangle on translatedRawImage (only for Debug)
	// const auto topLeft = maxLoc + potRoi.tl();
	// const cv::Point bottomLeft(topLeft.x + potImage.cols, topLeft.y + potImage.rows);
	// const cv::Scalar color(255.0, 0.0, 0.0, 255.0);
	// cv::rectangle(translatedRawImage, topLeft, bottomLeft, color, 3);

	// Inverted mask
	cv::Mat invertedSegmentationImage;
	cv::bitwise_not(segmentationImage, invertedSegmentationImage);
	// Segment the image according to the segmentation mask
	cv::Mat segmentedTranslatedRawImage;
	cv::bitwise_and(translatedRawImage, segmentationImage, segmentedTranslatedRawImage);
	cv::bitwise_or(segmentedTranslatedRawImage, invertedSegmentationImage, segmentedTranslatedRawImage);

	// Slightly translate the image to get a perfect calibration
	const auto translationVector = potLocation - potInTranslatedRawImage;
	translationMat = (cv::Mat_<double>(2, 3) << 1, 0, translationVector.x, 0, 1, translationVector.y);
	cv::Mat calibratedRawImage;
	cv::warpAffine(segmentedTranslatedRawImage,
		           calibratedRawImage,
		           translationMat,
		           segmentedTranslatedRawImage.size(), 
		           cv::INTER_LINEAR,
		           cv::BORDER_CONSTANT,
		           white);

	// Erase the pot (and potentially some of the bottom of the plant)
	if (erasePot)
	{
		cv::rectangle(calibratedRawImage, potLocation, white, cv::FILLED);
	}
	
	return calibratedRawImage;
}

cv::Mat segmentPlantInImage(const cv::Mat& image)
{
	// Convert image to HSV color space
	cv::Mat hsv;
	cvtColor(image, hsv, cv::COLOR_BGR2HSV);

	// Creating masks to detect green color
	cv::Mat mask;
	inRange(hsv, cv::Scalar(15, 40, 20), cv::Scalar(90, 255, 255), mask);

	// Close the shape of the mask
	const cv::Mat kernel = cv::Mat::ones(5, 5, CV_32F);
	morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

	// Creating an inverted mask to segment out the cloth from the frame
	cv::Mat invertMask;
	cv::bitwise_not(mask, invertMask);

	// Object with black background
	cv::Mat segmentedImage;
	cv::bitwise_and(image, image, segmentedImage, mask);
	// Object with white background
	segmentedImage.setTo(cv::Scalar(255, 255, 255), invertMask);

	return segmentedImage;
}

bool autoCalibrationSideImage(const QString& input, const QString& output)
{
	assert(!input.isEmpty());
	assert(!output.isEmpty());

	const bool applyMask = true;
	const bool erasePot = true;
	
	const auto calibrationImage = cv::imread("Images/calibration/calibration_image.png");

	if (calibrationImage.empty())
	{
		qWarning() << "Could not load the calibration image";
		return false;
	}

	// All white segmentation image (nothing segmented)
	cv::Mat segmentationMaskImage(calibrationImage.rows,
		                          calibrationImage.cols,
		                          calibrationImage.type(),
		                          cv::Scalar(255, 255, 255));

	// If we apply the segmentation mask, we replace the white mask with the image mask
	if (applyMask)
	{
		segmentationMaskImage = cv::imread("Images/calibration/segmentation_mask.png");

		if (segmentationMaskImage.empty())
		{
			qWarning() << "Could not load the segmentation mask";
			return false;
		}
	}
	
	const cv::Rect potLocation(1043, 1672, 363, 214);
	

	// Load and calibrate the input image
	const auto image = cv::imread(input.toStdString());

	if (image.empty())
	{
		qWarning() << "Could not load the input image";
		return false;
	}
	
	auto calibratedImage = calibrateImage(image,
		                                  calibrationImage,
		                                  segmentationMaskImage,
		                                  potLocation,
		                                  erasePot);

	// Slightly rotate the image to make it vertical
	calibratedImage = rotateCounterClockwise(calibratedImage, 0.29);

	return cv::imwrite(output.toStdString(), calibratedImage);
}

bool autoCalibrationTopImage(const QString& input, const QString& output)
{
	const auto calibrationTopImage = cv::imread("Images/calibration/calibration_top_image.png");

	if (calibrationTopImage.empty())
	{
		qWarning() << "Could not load the calibration image";
		return false;
	}
	
	// All white segmentation image (nothing segmented)
	const cv::Mat segmentationMaskTopImage(calibrationTopImage.rows,
		                                   calibrationTopImage.cols,
		                                   calibrationTopImage.type(),
		                                   cv::Scalar(255, 255, 255));
	const cv::Rect potTopLocation(1001, 800, 454, 453);

	// Load and calibrate the input image
	const auto topView = cv::imread(input.toStdString());

	if (topView.empty())
	{
		qWarning() << "Could not load the input image";
		return false;
	}

	// Rotate 89.33 degrees clockwise
	const auto rotatedTopView = rotateCounterClockwise(topView, -89.33);
	const auto calibratedTopImage = calibrateImage(rotatedTopView,
		                                           calibrationTopImage,
		                                           segmentationMaskTopImage,
		                                           potTopLocation,
		                                           false);

	return cv::imwrite(output.toStdString(), calibratedTopImage);
}
