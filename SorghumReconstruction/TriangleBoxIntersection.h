#pragma once

#include <QVector3D>

/**
 * \brief Check whether a triangle intersect an AABB box
 * \param boxCenter Center of the box
 * \param boxHalfSize Half size of the box
 * \param triangleVertex1 First vertex of the triangle
 * \param triangleVertex2 Second vertex of the triangle
 * \param triangleVertex3 Third vertex of the triangle
 */
bool triangleBoxIntersection(const QVector3D& boxCenter,
	                         const QVector3D& boxHalfSize,
	                         const QVector3D& triangleVertex1,
	                         const QVector3D& triangleVertex2,
	                         const QVector3D& triangleVertex3);
