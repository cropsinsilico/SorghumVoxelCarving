#include "Thinning.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Source: OpenCV extra module ximgproc thinning function
// URL: https://github.com/opencv/opencv_contrib/blob/master/modules/ximgproc/src/thinning.cpp
void thinningIteration(cv::Mat img, int iter, ThinningTypes thinningType)
{
	cv::Mat marker = cv::Mat::zeros(img.size(), CV_8UC1);

	if (thinningType == ThinningTypes::ZhangSuen)
	{
		for (int i = 1; i < img.rows - 1; i++)
		{
			for (int j = 1; j < img.cols - 1; j++)
			{
				uchar p2 = img.at<uchar>(i - 1, j);
				uchar p3 = img.at<uchar>(i - 1, j + 1);
				uchar p4 = img.at<uchar>(i, j + 1);
				uchar p5 = img.at<uchar>(i + 1, j + 1);
				uchar p6 = img.at<uchar>(i + 1, j);
				uchar p7 = img.at<uchar>(i + 1, j - 1);
				uchar p8 = img.at<uchar>(i, j - 1);
				uchar p9 = img.at<uchar>(i - 1, j - 1);

				int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
					(p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
					(p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
					(p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
				int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
				int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
				int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

				if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
					marker.at<uchar>(i, j) = 1;
			}
		}
	}

	if (thinningType == ThinningTypes::GuoHall)
	{
		for (int i = 1; i < img.rows - 1; i++)
		{
			for (int j = 1; j < img.cols - 1; j++)
			{
				uchar p2 = img.at<uchar>(i - 1, j);
				uchar p3 = img.at<uchar>(i - 1, j + 1);
				uchar p4 = img.at<uchar>(i, j + 1);
				uchar p5 = img.at<uchar>(i + 1, j + 1);
				uchar p6 = img.at<uchar>(i + 1, j);
				uchar p7 = img.at<uchar>(i + 1, j - 1);
				uchar p8 = img.at<uchar>(i, j - 1);
				uchar p9 = img.at<uchar>(i - 1, j - 1);

				int C = ((!p2) & (p3 | p4)) + ((!p4) & (p5 | p6)) +
					((!p6) & (p7 | p8)) + ((!p8) & (p9 | p2));
				int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
				int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
				int N = N1 < N2 ? N1 : N2;
				int m = iter == 0 ? ((p6 | p7 | (!p9)) & p8) : ((p2 | p3 | (!p5)) & p4);

				if ((C == 1) && ((N >= 2) && ((N <= 3)) & (m == 0)))
					marker.at<uchar>(i, j) = 1;
			}
		}
	}

	img &= ~marker;
}

cv::Mat convertToCvMat(const QImage& image)
{
	assert(image.format() == QImage::Format_RGB32 || image.format() == QImage::Format_ARGB32);
	
	const cv::Mat cvImage(
		image.height(),
		image.width(),
		CV_8UC4,
		const_cast<uchar*>(image.bits()),
		image.bytesPerLine());

	cv::Mat output;
	cv::cvtColor(cvImage, output, cv::COLOR_RGBA2RGB);
	
	return output;
}

QImage convertToQtImage(cv::InputArray input)
{
	assert(input.type() == CV_8UC3);

	const cv::Mat view(input.getMat());
	cv::Mat rgbImage;
	cv::cvtColor(view, rgbImage, cv::COLOR_BGR2RGB);
	
	const QImage imageView(rgbImage.data, rgbImage.cols, rgbImage.rows, rgbImage.step, QImage::Format_RGB888);
	
	return imageView.convertToFormat(QImage::Format_RGB32);
}

void thinning(cv::InputArray src, cv::OutputArray dst, ThinningTypes thinningType)
{
	cv::Mat processed = src.getMat().clone();

	// Enforce the range of the input image to be in between 0 - 255
	processed /= 255;

	cv::Mat prev = cv::Mat::zeros(processed.size(), CV_8UC1);
	cv::Mat diff;

	do {
		thinningIteration(processed, 0, thinningType);
		thinningIteration(processed, 1, thinningType);
		absdiff(processed, prev, diff);
		processed.copyTo(prev);
	} while (countNonZero(diff) > 0);

	processed *= 255;

	dst.assign(processed);
}

QImage skeletonize(const QImage& image, ThinningTypes thinningType)
{
	const auto input = convertToCvMat(image);

	// Transform the image to black and white
	cv::Mat inputBlackWhite;
	cv::cvtColor(input, inputBlackWhite, cv::COLOR_BGR2GRAY);
	cv::threshold(inputBlackWhite, inputBlackWhite, 235, 255, cv::THRESH_BINARY_INV);

	// Apply thinning on the input image
	cv::Mat output;
	thinning(inputBlackWhite, output, thinningType);

	// Output a color QImage
	cv::Mat colorOutput;
	cv::cvtColor(output, colorOutput, cv::COLOR_GRAY2BGRA);
	return convertToQtImage(colorOutput);
}
