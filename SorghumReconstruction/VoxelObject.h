#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include "Renderable.h"
#include "VoxelGrid.h"
#include "VoxelCarver.h"

class VoxelObject final : public Renderable
{
public:
	VoxelObject(int order, const QMatrix4x4& worldMatrix, const VoxelGrid& voxelGrid);

	~VoxelObject() = default;

	void initialize(QOpenGLContext* context) override;

	void cleanup() override;

	void paint(QOpenGLContext* context, const Camera& camera) override;

private:

	void createInstances(const VoxelGrid& voxelGrid);

	void initializeVbo();
	void initializeEbo();
	void initializeIbo();

	std::unique_ptr<QOpenGLShaderProgram> m_program;

	QOpenGLVertexArrayObject m_vao;
	QOpenGLBuffer m_vbo;
	QOpenGLBuffer m_ebo;
	int m_numberTriangles;

	QOpenGLBuffer m_ibo;
	int m_numberInstances;
	
	std::vector<QVector3D> m_instances;
	float m_voxelSize;
};

