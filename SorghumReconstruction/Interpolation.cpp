#include "Interpolation.h"

#include <cmath>

#include "MathUtils.h"

float interpolateRealPixel(const QImage& image, float rx, float ry)
{
	const auto x = int(std::floor(rx));
	const auto y = int(std::floor(ry));

	const auto maxX = image.width() - 1;
	const auto maxY = image.height() - 1;

	const int x0 = clamp(x - 1, 0, maxX);
	const int y0 = clamp(y - 1, 0, maxY);

	const int x1 = clamp(x, 0, maxX);
	const int y1 = clamp(y, 0, maxY);

	const int x2 = clamp(x + 1, 0, maxX);
	const int y2 = clamp(y + 1, 0, maxY);

	const int x3 = clamp(x + 2, 0, maxX);
	const int y3 = clamp(y + 2, 0, maxY);

	const float u = rx - float(x);
	const float v = ry - float(y);

	// Interpolation of the value color components
	const std::array<std::array<float, 4>, 4> values = {
		{
			{{
				float(image.pixelColor(x0, y0).value()),
				float(image.pixelColor(x0, y1).value()),
				float(image.pixelColor(x0, y2).value()),
				float(image.pixelColor(x0, y3).value())
			}},
			{{
				float(image.pixelColor(x1, y0).value()),
				float(image.pixelColor(x1, y1).value()),
				float(image.pixelColor(x1, y2).value()),
				float(image.pixelColor(x1, y3).value())
			}},
			{{
				float(image.pixelColor(x2, y0).value()),
				float(image.pixelColor(x2, y1).value()),
				float(image.pixelColor(x2, y2).value()),
				float(image.pixelColor(x2, y3).value())
			}},
			{{
				float(image.pixelColor(x3, y0).value()),
				float(image.pixelColor(x3, y1).value()),
				float(image.pixelColor(x3, y2).value()),
				float(image.pixelColor(x3, y3).value())
			}}
		} };

	return biCubicInterpolate(values, u, v);
}
