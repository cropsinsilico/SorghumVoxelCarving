#include "MathUtils.h"

std::vector<float> generateUniformAngles(int numberAngles, float maxAngles)
{
	assert(numberAngles >= 0);
	
	std::vector<float> angles(numberAngles, 0.0f);

	for (unsigned int i = 0; i < angles.size(); i++)
	{
		angles[i] = maxAngles * (float(i) / angles.size());
	}
	
	return angles;
}

QMatrix4x4 generateRotationZMatrix(const QVector3D& origin, float angle)
{
	QMatrix4x4 matrix;

	matrix.translate(origin);
	matrix.rotate(angle, 0.0, 0.0, 1.0);
	matrix.translate(-origin);
	
	return matrix;
}

float distanceSquared(const QVector3D& a, const QVector3D& b)
{
	return (a.x() - b.x()) * (a.x() - b.x())
		 + (a.y() - b.y()) * (a.y() - b.y())
		 + (a.z() - b.z()) * (a.z() - b.z());
}

float pointLineProjection(const QVector3D& p, const QVector3D& a, const QVector3D& b)
{
	const QVector3D ap(p - a);
	const QVector3D ab(b - a);

	// Segment is only a point and has no length
	if (ab.isNull())
	{
		// The nearest point on the segment is A (or B)
		return 0.0f;
	}

	// Segment has a length greater than 0
	// Projection of the point p on the line (AB)
	return QVector3D::dotProduct(ap, ab) / ab.lengthSquared();
}

float pointLineSegmentProjection(const QVector3D& p, const QVector3D& a, const QVector3D& b)
{
	const float u = pointLineProjection(p, a, b);
	return clamp(u, 0.0f, 1.0f);
}

float distanceToLineSegment(const QVector3D& p, const QVector3D& a, const QVector3D& b, QVector3D& c)
{
	const float u = pointLineSegmentProjection(p, a, b);

	c = a * (1.0f - u) + b * u;

	return p.distanceToPoint(c);
}

float distanceToLineSegment(const QVector3D& p, const QVector3D& a, const QVector3D& b)
{
	QVector3D c;
	return distanceToLineSegment(p, a, b, c);
}

// Source: https://iquilezles.org/www/articles/triangledistance/triangledistance.htm
float distanceSquaredToTriangle(const QVector3D& p, const QVector3D& v1, const QVector3D& v2, const QVector3D& v3)
{
	const auto v21 = v2 - v1;
	const auto p1 = p - v1;
	const auto v32 = v3 - v2;
	const auto p2 = p - v2;
	const auto v13 = v1 - v3;
	const auto p3 = p - v3;
	const auto nor = QVector3D::crossProduct(v21, v13);

	// If the three signs are positive, then we are inside the triangle
	if (sign(QVector3D::dotProduct(QVector3D::crossProduct(v21, nor), p1)) +
	    sign(QVector3D::dotProduct(QVector3D::crossProduct(v32, nor), p2)) +
	    sign(QVector3D::dotProduct(QVector3D::crossProduct(v13, nor), p3)) < 2)
	{
		// We are outside of the triangle, we compute the minimum distance to its edges
		const auto d1 = (v21 * clamp(QVector3D::dotProduct(v21, p1) / v21.lengthSquared(), 0.0f, 1.0f) - p1).lengthSquared();
		const auto d2 = (v32 * clamp(QVector3D::dotProduct(v32, p2) / v32.lengthSquared(), 0.0f, 1.0f) - p2).lengthSquared();
		const auto d3 = (v13 * clamp(QVector3D::dotProduct(v13, p3) / v13.lengthSquared(), 0.0f, 1.0f) - p3).lengthSquared();

		return std::min({d1, d2, d3});
	}
	else
	{
		// We are inside the triangle, we compute the minimum distance to its face
		return QVector3D::dotProduct(nor, p1) * QVector3D::dotProduct(nor, p1) / QVector3D::dotProduct(nor, nor);
	}
}
