#include "AABB.h"

AABB::AABB(const QVector3D& a, const QVector3D& b) :
	m_start(std::min(a.x(), b.x()),
		    std::min(a.y(), b.y()),
		    std::min(a.z(), b.z())),
	m_end(std::max(a.x(), b.x()),
		  std::max(a.y(), b.y()),
		  std::max(a.z(), b.z()))
{
	
}

AABB::AABB(const QVector3D& center, float halfSize) :
	AABB(QVector3D(center.x() - halfSize, center.y() - halfSize, center.z() - halfSize),
		 QVector3D(center.x() + halfSize, center.y() + halfSize, center.z() + halfSize))
{
	
}

float AABB::minX() const
{
	return m_start.x();
}

float AABB::minY() const
{
	return m_start.y();
}

float AABB::minZ() const
{
	return m_start.z();
}

float AABB::maxX() const
{
	return m_end.x();
}

float AABB::maxY() const
{
	return m_end.y();
}

float AABB::maxZ() const
{
	return m_end.z();
}

float AABB::sizeX() const
{
	return m_end.x() - m_start.x();
}

float AABB::sizeY() const
{
	return m_end.y() - m_start.y();
}

float AABB::sizeZ() const
{
	return m_end.z() - m_start.z();
}

QVector3D AABB::center() const
{
	return (m_start + m_end) / 2.0;
}

QVector3D AABB::lerp(const QVector3D& value) const
{
	return {
		m_start.x() + value.x() * (m_end.x() - m_start.x()),
		m_start.y() + value.y() * (m_end.y() - m_start.y()),
		m_start.z() + value.z() * (m_end.z() - m_start.z())
	};
}

QVector3D AABB::inverseLerp(const QVector3D& point) const
{
	return {
		(point.x() - m_start.x()) / (m_end.x() - m_start.x()),
		(point.y() - m_start.y()) / (m_end.y() - m_start.y()),
		(point.z() - m_start.z()) / (m_end.z() - m_start.z())
	};
}

bool AABB::isInside(const QVector3D& point) const
{
	return (m_start.x() <= point.x() && point.x() <= m_end.x())
	    && (m_start.y() <= point.y() && point.y() <= m_end.y())
		&& (m_start.z() <= point.z() && point.z() <= m_end.z());
}
