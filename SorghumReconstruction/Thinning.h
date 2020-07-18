#pragma once

#include <QImage>

#include <opencv2/core/core.hpp>

enum class ThinningTypes
{
	ZhangSuen = 0, // Thinning technique of Zhang-Suen
	GuoHall = 1    // Thinning technique of Guo-Hall
};

cv::Mat convertToCvMat(const QImage& image);

QImage convertToQtImage(cv::InputArray input);

void thinning(cv::InputArray src, cv::OutputArray dst, ThinningTypes thinningType = ThinningTypes::ZhangSuen);

/**
 * \brief Skeletonization reduces binary objects to 1 pixel wide representations.
 * \param image The input image
 * \return The skeletonized version of the input image
 */
QImage skeletonize(const QImage& image, ThinningTypes thinningType = ThinningTypes::ZhangSuen);
