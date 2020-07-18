#pragma once

#include <memory>

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

#include "Renderable.h"

class PhysicalCamera final : public Renderable
{
public:
	PhysicalCamera(int order,
		           const Camera& camera,
		           float focalLength,
		           QImage image);

	~PhysicalCamera() = default;

	void initialize(QOpenGLContext* context) override;

	void cleanup() override;

	void paint(QOpenGLContext* context, const Camera& camera) override;

private:

	/**
	 * \brief Compute the location of the frustum in the 3D space
	 * \param topLeft 3D coordinates of the top left corner of the frustum
	 * \param topRight 3D coordinates of the top right corner of the frustum
	 * \param bottomLeft 3D coordinates of the bottom left corner of the frustum
	 * \param bottomRight 3D coordinates of the top right corner of the frustum
	 */
	void computeFrustumLocation(QVector3D& topLeft,
								QVector3D& topRight,
								QVector3D& bottomLeft,
							    QVector3D& bottomRight) const;
	
	void initializeImage();

	void initializeFrustum();

	
	std::unique_ptr<QOpenGLShaderProgram> m_imageProgram;
	QOpenGLVertexArrayObject m_imageVao;
	QOpenGLBuffer m_imageVbo;
	int m_imageNumberVertices;
	QImage m_image;
	QOpenGLTexture m_texture;

	std::unique_ptr<QOpenGLShaderProgram> m_frustumProgram;
	QOpenGLVertexArrayObject m_frustumVao;
	QOpenGLBuffer m_frustumVbo;
	int m_frustumNumberVertices;
	
	QVector3D m_eye;
	QVector3D m_at;
	QVector3D m_up;
	float m_focalLength;
	float m_fovy;
	float m_aspectRatio;
};
