#pragma once

#include <memory>

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include "Renderable.h"

class Cylinder final : public Renderable
{
public:
	Cylinder(int order, const QMatrix4x4& worldMatrix);

	~Cylinder() = default;

	void initialize(QOpenGLContext* context) override;

	void cleanup() override;

	void paint(QOpenGLContext* context, const Camera& camera) override;
	
private:

	std::unique_ptr<QOpenGLShaderProgram> m_program;
	
	QOpenGLVertexArrayObject m_vao;
	QOpenGLBuffer m_vbo;
	int m_numberPatches;
};

