#pragma once

#include <QImage>

/**
 * \brief Interpolate (bi-cubic) a real pixel in an image. 
 * \param image An image
 * \param rx The real x coordinate between 0 and the image's width
 * \param ry The real y coordinate between 0 and the image's height
 * \return The value of the pixel
 */
float interpolateRealPixel(const QImage& image, float rx, float ry);
