#pragma once

#include <QMatrix4x4>

#include "Camera.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLContext)

class Renderable
{
public:
	Renderable() = default;

	Renderable(int order, const QMatrix4x4& worldMatrix);
	
	virtual ~Renderable() = default;

	/**
	 * \brief Initialize the renderable object in the current context
	 * The OpenGL context must be current
	 * \param context The OpenGL context
	 */
	virtual void initialize(QOpenGLContext* context) = 0;

	/**
	 * \brief Cleanup memory
	 * The OpenGL context must be current
	 */
	virtual void cleanup() = 0;

	/**
	 * \brief Paint the renderable object from the point of view of a camera
	 * The OpenGL context must be current
	 * \param context The OpenGL context
	 * \param camera The camera
	 */
	virtual void paint(QOpenGLContext* context, const Camera& camera) = 0;

	virtual int order() const
	{
		return m_order;
	}

	virtual void setWorldMatrix(const QMatrix4x4& worldMatrix)
	{
		m_worldMatrix = worldMatrix;
	}
	
	virtual const QMatrix4x4& worldMatrix() const
	{
		return m_worldMatrix;
	}
	
private:
	/**
	 * \brief Number to order the rendering
	 */
	int m_order;
	
	/**
	 * \brief Object to world matrix
	 */
	QMatrix4x4 m_worldMatrix;
};

