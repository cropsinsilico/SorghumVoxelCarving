#pragma once

#include <QVector3D>

class AABB
{
public:
	explicit AABB(const QVector3D& a, const QVector3D& b);
	explicit AABB(const QVector3D& center, float halfSize);

	/**
	 * \brief Return the minimum coordinate on the X axis
	 * \return The minimum coordinate on the X axis
	 */
	float minX() const;

	/**
	 * \brief Return the minimum coordinate on the Y axis
	 * \return The minimum coordinate on the Y axis
	 */
	float minY() const;

	/**
	 * \brief Return the minimum coordinate on the Z axis
	 * \return The minimum coordinate on the Z axis
	 */
	float minZ() const;

	/**
	 * \brief Return the maximum coordinate on the X axis
	 * \return The maximum coordinate on the X axis
	 */
	float maxX() const;

	/**
	 * \brief Return the maximum coordinate on the Y axis
	 * \return The maximum coordinate on the Y axis
	 */
	float maxY() const;

	/**
	 * \brief Return the maximum coordinate on the Z axis
	 * \return The maximum coordinate on the Z axis
	 */
	float maxZ() const;

	/**
	 * \brief Return the size on the X axis
	 * \return The size on the X axis
	 */
	float sizeX() const;

	/**
	 * \brief Return the size on the Y axis
	 * \return The size on the Y axis
	 */
	float sizeY() const;

	/**
	 * \brief Return the size on the Z axis
	 * \return The size on the Z axis
	 */
	float sizeZ() const;

	/**
	 * \brief Return the center of the AABB
	 * \return The center of the AABB
	 */
	QVector3D center() const;

	/**
	 * \brief Linearly interpolate a point in the box
	 * \param value A 3D point with coordinates between 0.0 and 1.0
	 * \return A point in the box
	 */
	QVector3D lerp(const QVector3D& value) const;

	/**
	 * \brief Gives the inverse of the lerp function. p = lerp(inverseLerp(p))
	 * \param point A point in 3D space
	 * \return The point indexed in the bounding box with coordinates between 0.0 and 1.0
	 */
	QVector3D inverseLerp(const QVector3D& point) const;

	/**
	 * \brief Check whether a point is inside or outside the box
	 * \param point Coordinates of the point
	 * \return True if the point is in the box, false otherwise
	 */
	bool isInside(const QVector3D& point) const;

private:
	/**
	 * \brief Minimum coordinates of the AABB
	 */
	QVector3D m_start;
	
	/**
	 * \brief Maximum coordinates of the AABB
	 */
	QVector3D m_end;
};

