#include "VoxelObject.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions_4_3_Core>

VoxelObject::VoxelObject(int order, const QMatrix4x4& worldMatrix, const VoxelGrid& voxelGrid) :
	Renderable(order, worldMatrix),
	m_vbo(QOpenGLBuffer::VertexBuffer),
	m_ebo(QOpenGLBuffer::IndexBuffer),
	m_numberTriangles(0),
	m_ibo(QOpenGLBuffer::VertexBuffer),
	m_numberInstances(0),
	m_voxelSize(0.0f)
{
	createInstances(voxelGrid);
}

void VoxelObject::initialize(QOpenGLContext* context)
{
	auto f = context->versionFunctions<QOpenGLFunctions_4_3_Core>();
	
	const QString shader_dir = ":/MainWindow/Shaders/";

	// Init Program
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, shader_dir + "voxel_grid_vs.glsl");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, shader_dir + "voxel_grid_fs.glsl");
	m_program->link();

	m_program->bind();

	// Initialize vertices and indices
	initializeVbo();
	initializeEbo();
	initializeIbo();

	// Init VAO
	m_vao.create();
	QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

	// Configure VBO
	m_vbo.bind();
	const auto posLoc = 0;
	m_program->enableAttributeArray(posLoc);
	m_program->setAttributeArray(posLoc, nullptr, 3, 0);

	// Configure EBO
	m_ebo.bind();

	// Configure IBO
	m_ibo.bind();
	const auto instancePositionLoc = 1;
	m_program->enableAttributeArray(instancePositionLoc);
	m_program->setAttributeArray(instancePositionLoc, nullptr, 3, 0);
	f->glVertexAttribDivisor(instancePositionLoc, 1);
	
	m_program->release();
}

void VoxelObject::cleanup()
{
	if (m_program)
	{
		m_vao.destroy();
		m_vbo.destroy();
		m_ebo.destroy();
		m_ibo.destroy();
		m_program.reset(nullptr);
	}
}

void VoxelObject::paint(QOpenGLContext* context, const Camera& camera)
{
	auto f = context->versionFunctions<QOpenGLFunctions_4_3_Core>();
	
	if (m_program && m_numberTriangles > 0)
	{
		// Setup matrices
		const auto normalMatrix = worldMatrix().normalMatrix();
		const auto viewMatrix = camera.viewMatrix();
		const auto projectionMatrix = camera.projectionMatrix();
		const auto pvMatrix = projectionMatrix * viewMatrix;
		const auto pvmMatrix = pvMatrix * worldMatrix();

		m_program->bind();

		// Update matrices
		m_program->setUniformValue("P", projectionMatrix);
		m_program->setUniformValue("V", viewMatrix);
		m_program->setUniformValue("M", worldMatrix());
		m_program->setUniformValue("N", normalMatrix);
		m_program->setUniformValue("PV", pvMatrix);
		m_program->setUniformValue("PVM", pvmMatrix);

		// Update voxel size
		m_program->setUniformValue("voxel_size", m_voxelSize);

		// Bind the VAO containing the patches
		QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
		
		f->glDrawElementsInstanced(GL_TRIANGLES,
								   m_numberTriangles,
								   GL_UNSIGNED_INT,
								   nullptr,
								   m_numberInstances);

		m_program->release();
	}
}

void VoxelObject::createInstances(const VoxelGrid& voxelGrid)
{
	const auto& voxels = voxelGrid.voxels();
	
	// Compute positions of each instance
	m_instances.clear();
	m_instances.reserve(voxels.size());
	for (const auto& voxel : voxels)
	{
		const auto position = voxelGrid.voxel(voxel.x, voxel.y, voxel.z);
		m_instances.push_back(position);
	}

	// Estimate size of voxels
	m_voxelSize = std::min({ voxelGrid.voxelSizeX(), voxelGrid.voxelSizeY(), voxelGrid.voxelSizeZ() });
}

void VoxelObject::initializeVbo()
{
	const std::vector<QVector3D> vertices = {
		// Bottom
		{-0.5, -0.5, -0.5},
		{0.5, -0.5, -0.5},
		{-0.5, 0.5, -0.5},
		{0.5, 0.5, -0.5},
		// Top
		{-0.5, -0.5, 0.5},
		{0.5, -0.5, 0.5},
		{-0.5, 0.5, 0.5},
		{0.5, 0.5, 0.5}
	};
	
	// Init VBO
	m_vbo.create();
	m_vbo.bind();

	m_vbo.allocate(vertices.data(), vertices.size() * sizeof(QVector3D));

	m_vbo.release();
}

void VoxelObject::initializeEbo()
{
	const std::vector<GLuint> indices = {
		// Bottom cap
		0, 2, 3,
		0, 3, 1,
		// Top cap
		4, 7, 6,
		7, 4, 5,
		// Near cap
		0, 5, 4,
		5, 0, 1,
		// Far cap
		6, 7, 2,
		7, 3, 2,
		// Left cap
		4, 2, 0,
		6, 2, 4,
		// Right cap
		5, 1, 3,
		5, 3, 7
	};
	
	// Init EBO
	m_ebo.create();
	m_ebo.bind();

	m_ebo.allocate(indices.data(), indices.size() * sizeof(GLuint));
	m_numberTriangles = indices.size();

	m_ebo.release();
}

void VoxelObject::initializeIbo()
{
	// Init EBO
	m_ibo.create();
	
	m_ibo.bind();
	m_ibo.allocate(m_instances.data(), m_instances.size() * sizeof(QVector3D));
	m_ibo.release();

	m_numberInstances = m_instances.size();
}
