#pragma once

#include <vector>
#include <memory>

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include "Renderable.h"

class MeshObject final : public Renderable
{
public:
	MeshObject(int order,
			   const QMatrix4x4& worldMatrix,
		       std::vector<QVector3D> vertices,
		       std::vector<std::tuple<int, int, int>> faces);

	~MeshObject() = default;

	void initialize(QOpenGLContext* context) override;

	void cleanup() override;

	void paint(QOpenGLContext* context, const Camera& camera) override;

private:

	void initializeVbo();
	void initializeEbo();

	std::unique_ptr<QOpenGLShaderProgram> m_program;

	QOpenGLVertexArrayObject m_vao;
	QOpenGLBuffer m_vbo;
	QOpenGLBuffer m_ebo;

	std::vector<QVector3D> m_vertices;
	std::vector<std::tuple<int, int, int>> m_faces;
};

