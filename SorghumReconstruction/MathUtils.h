#pragma once

#include <array>

#include <QVector3D>
#include <QMatrix4x4>

template<typename T>
const T& clamp(const T& v, const T& lo, const T& hi)
{
	assert(lo <= hi);

	if (v < lo)
	{
		return lo;
	}
	else if (v > hi)
	{
		return hi;
	}
	else
	{
		return v;
	}
}

template<typename T>
T lerp(const T& a, const T& b, const T& x)
{
	// FMA friendly
	return x * b + (a - a * x);
}

inline QVector3D lerp(const QVector3D& a, const QVector3D& b, const float& x)
{
    return a + x * (b - a);
}

// Source: https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
template <typename T> int sign(T val)
{
	return (T(0) < val) - (val < T(0));
}

template<typename T>
T cubicInterpolate(const T& p0, const T& p1, const T& p2, const T& p3, double t)
{
	assert(0.0 <= t && t <= 1.0);

	return p1 + 0.5 * t * (p2 - p0 + t * (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3 + t * (3.0 * (p1 - p2) + p3 - p0)));
}

template<typename T>
T cubicInterpolate(const std::array<T, 4>& p, double t)
{
	assert(0.0 <= t && t <= 1.0);

	return cubicInterpolate(p[0], p[1], p[2], p[3], t);
}

template<typename T>
T biCubicInterpolate(const std::array<std::array<T, 4>, 4>& p, double u, double v)
{
	assert(0.0 <= u && u <= 1.0);
	assert(0.0 <= v && v <= 1.0);

	std::array<T, 4> temp{};
	for (unsigned int i = 0; i < 4; i++)
	{
		temp[i] = cubicInterpolate(p[i], v);
	}
	return cubicInterpolate(temp, u);
}

/**
 * \brief Generate a vector with equally spaced angles 
 * \param numberAngles The number angles to generate
 * \param maxAngles The maximum angle
 * \return A vector with number of angles
 */
std::vector<float> generateUniformAngles(int numberAngles, float maxAngles = 360.0f);

/**
 * \brief Generate a transformation matrix that rotates around a point on the Z axis
 * \param origin The point around which to rotate
 * \param angle The angle in degrees
 * \return A transformation matrix
 */
QMatrix4x4 generateRotationZMatrix(const QVector3D& origin, float angle);

/**
 * \brief Return the squared distance between points A and B
 * \param a First point
 * \param b Second point
 * \return The squared distance between points A and B
 */
float distanceSquared(const QVector3D& a, const QVector3D& b);

/**
 * \brief Project point P on the line (AB)
 * \param p The point to project
 * \param a The beginning of the line segment
 * \param b The end of the line segment
 * \return The normalized distance from A to the projection of P
 */
float pointLineProjection(const QVector3D& p, const QVector3D& a, const QVector3D& b);

/**
 * \brief Project point P on the line segment [AB]
 * \param p The point to project
 * \param a The beginning of the line segment
 * \param b The end of the line segment
 * \return The normalized distance in [0; 1] from A to the projection of P
 */
float pointLineSegmentProjection(const QVector3D& p, const QVector3D& a, const QVector3D& b);

/**
 * \brief Project point P on the line segment [AB] and return the distance from P to that point
 * \param p The point to project
 * \param a The beginning of the line segment
 * \param b The end of the line segment
 * \param c The projected point
 * \return The distance from point P to the line segment [AB]
 */
float distanceToLineSegment(const QVector3D& p, const QVector3D& a, const QVector3D& b, QVector3D& c);
float distanceToLineSegment(const QVector3D& p, const QVector3D& a, const QVector3D& b);

/**
 * \brief Compute the squared distance to a 3D triangle
 * \param p The query point
 * \param v1 First vertex of the triangle
 * \param v2 Second vertex of the triangle
 * \param v3 Third vertex of the triangle
 * \return The squared distance from the point P to the triangle ABC
 */
float distanceSquaredToTriangle(const QVector3D& p, const QVector3D& v1, const QVector3D& v2, const QVector3D& v3);