#include "Cylinder.h"

Cylinder::Cylinder(int order, const QMatrix4x4& worldMatrix) :
	Renderable(order, worldMatrix),
	m_numberPatches(0)
{
	
}

void Cylinder::initialize(QOpenGLContext* context)
{
	// Init Program
	m_program = std::make_unique<QOpenGLShaderProgram>();

	const QString shader_dir = ":/MainWindow/Shaders/";
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, shader_dir + "cylinder_vs.glsl");
	m_program->addShaderFromSourceFile(QOpenGLShader::TessellationControl, shader_dir + "cylinder_tsc.glsl");
	m_program->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, shader_dir + "cylinder_tse.glsl");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, shader_dir + "cylinder_fs.glsl");

	m_program->link();

	m_program->bind();

	// Init VAO
	m_vao.create();
	QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

	// Init VBO
	m_vbo.create();
	m_vbo.bind();

	const std::vector<QVector3D> patches = {
		// Right
		{ 1.0, 0.0, 0.5},
		{ 0.0, -1.0, 0.5},
		{ 0.0, -1.0, -0.5},
		{ 1.0, 0.0, -0.5},

		// Top
		{ 0.0, 1.0, 0.5},
		{ 1.0, 0.0, 0.5},
		{ 1.0, 0.0, -0.5},
		{ 0.0, 1.0, -0.5},

		// Left
		{ 0.0, 1.0, 0.5},
		{ -1.0, 0.0, 0.5},
		{ -1.0, 0.0, -0.5},
		{ 0.0, 1.0, -0.5},

		// Bottom
		{ 0.0, -1.0, 0.5},
		{ -1.0, 0.0, 0.5},
		{ -1.0, 0.0, -0.5},
		{ 0.0, -1.0, -0.5},

		// Top cap
		{ 1.0, 0.0, 0.5},
		{ 0.0, 1.0, 0.5},
		{ -1.0, 0.0, 0.5},
		{ 0.0, -1.0, 0.5},

		// Bottom cap
		{ 1.0, 0.0, -0.5},
		{ 0.0, -1.0, -0.5},
		{ -1.0, 0.0, -0.5},
		{ 0.0, 1.0, -0.5},
	};

	const auto posLoc = 0;
	m_program->enableAttributeArray(posLoc);
	m_program->setAttributeArray(posLoc, nullptr, 3, 0);

	m_vbo.allocate(patches.data(), patches.size() * sizeof(QVector3D));
	m_numberPatches = patches.size() / 4;

	m_vbo.release();
	m_program->release();
}

void Cylinder::cleanup()
{
	if (m_program)
	{
		m_vao.destroy();
		m_vbo.destroy();
		m_program.reset(nullptr);
	}
}

void Cylinder::paint(QOpenGLContext* context, const Camera& camera)
{
	if (m_program && m_numberPatches > 0)
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

		const auto verticesPerPatch = 4;
		m_program->setPatchVertexCount(verticesPerPatch);

		glDrawArrays(GL_PATCHES, 0, verticesPerPatch * m_numberPatches);

		m_program->release();
	}
}

