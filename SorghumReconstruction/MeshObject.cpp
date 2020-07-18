#include "MeshObject.h"

MeshObject::MeshObject(int order,
					   const QMatrix4x4& worldMatrix,
	                   std::vector<QVector3D> vertices,
	                   std::vector<std::tuple<int, int, int>> faces) :
	Renderable(order, worldMatrix),
	m_vbo(QOpenGLBuffer::VertexBuffer),
	m_ebo(QOpenGLBuffer::IndexBuffer),
	m_vertices(std::move(vertices)),
	m_faces(std::move(faces))
{
	
}

void MeshObject::initialize(QOpenGLContext* context)
{
	const QString shader_dir = ":/MainWindow/Shaders/";

	// Init Program
	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, shader_dir + "mesh_vs.glsl");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, shader_dir + "mesh_fs.glsl");
	m_program->link();

	m_program->bind();

	// Initialize vertices and indices
	initializeVbo();
	initializeEbo();

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

	m_program->release();
}

void MeshObject::cleanup()
{
	if (m_program)
	{
		m_vao.destroy();
		m_vbo.destroy();
		m_ebo.destroy();
		m_program.reset(nullptr);
	}
}

void MeshObject::paint(QOpenGLContext* context, const Camera& camera)
{
	if (m_program)
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

		// Bind the VAO containing the patches
		QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

		glDrawElements(GL_TRIANGLES,
					   m_ebo.size(),
					   GL_UNSIGNED_INT,
					   nullptr);

		m_program->release();
	}
}

void MeshObject::initializeVbo()
{
	// Init VBO
	m_vbo.create();
	m_vbo.bind();

	m_vbo.allocate(m_vertices.data(), m_vertices.size() * sizeof(QVector3D));

	m_vbo.release();
}

void MeshObject::initializeEbo()
{
	// Indices of faces
	std::vector<GLuint> indices;

	// Convert to unsigned int
	indices.reserve(3 * m_faces.size());
	for (unsigned int i = 0; i < m_faces.size(); i++)
	{
		indices.push_back(std::get<0>(m_faces[i]));
		indices.push_back(std::get<1>(m_faces[i]));
		indices.push_back(std::get<2>(m_faces[i]));
	}
	
	// Init VBO
	m_ebo.create();
	m_ebo.bind();

	m_ebo.allocate(indices.data(), indices.size() * sizeof(GLuint));

	m_ebo.release();
}
