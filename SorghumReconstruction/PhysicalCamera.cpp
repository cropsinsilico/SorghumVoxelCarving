#include "PhysicalCamera.h"

#include <utility>

#include <QtMath>

PhysicalCamera::PhysicalCamera(int order, const Camera& camera, float focalLength, QImage image) :
	Renderable(order, QMatrix4x4()),
	m_imageNumberVertices(0),
	m_image(std::move(image)),
	m_texture(QOpenGLTexture::Target2D),
	m_frustumNumberVertices(0),
	m_eye(camera.eye()),
	m_at(camera.at()),
	m_up(camera.up()),
	m_focalLength(focalLength),
	m_fovy(camera.fovy()),
	m_aspectRatio(camera.aspectRatio())
{
	
}

void PhysicalCamera::initialize(QOpenGLContext* context)
{
	const QString shader_dir = ":/MainWindow/Shaders/";
	
	// Init Program
	m_imageProgram = std::make_unique<QOpenGLShaderProgram>();
	m_imageProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, shader_dir + "physical_camera_image_vs.glsl");
	m_imageProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, shader_dir + "physical_camera_image_fs.glsl");
	m_imageProgram->link();

	initializeImage();

	// Init Program
	m_frustumProgram = std::make_unique<QOpenGLShaderProgram>();
	m_frustumProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, shader_dir + "physical_camera_frustum_vs.glsl");
	m_frustumProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, shader_dir + "physical_camera_frustum_fs.glsl");
	m_frustumProgram->link();
	
	initializeFrustum();
}

void PhysicalCamera::cleanup()
{
	m_imageVao.destroy();
	m_imageVbo.destroy();
	m_texture.destroy();
	
	if (m_imageProgram)
	{
		m_imageProgram.reset(nullptr);
	}

	if (m_frustumProgram)
	{
		m_frustumProgram.reset(nullptr);
	}
}

void PhysicalCamera::paint(QOpenGLContext* context, const Camera& camera)
{
	if (m_imageProgram && m_imageNumberVertices > 0)
	{
		// Setup matrices
		const auto normalMatrix = worldMatrix().normalMatrix();
		const auto viewMatrix = camera.viewMatrix();
		const auto projectionMatrix = camera.projectionMatrix();
		const auto pvMatrix = projectionMatrix * viewMatrix;
		const auto pvmMatrix = pvMatrix * worldMatrix();

		m_imageProgram->bind();

		// Update matrices
		m_imageProgram->setUniformValue("P", projectionMatrix);
		m_imageProgram->setUniformValue("V", viewMatrix);
		m_imageProgram->setUniformValue("M", worldMatrix());
		m_imageProgram->setUniformValue("N", normalMatrix);
		m_imageProgram->setUniformValue("PV", pvMatrix);
		m_imageProgram->setUniformValue("PVM", pvmMatrix);

		// Bind the texture
		const auto textureUnit = 0;
		m_imageProgram->setUniformValue("image", textureUnit);
		m_texture.bind(textureUnit);

		// Bind the VAO containing the patches
		QOpenGLVertexArrayObject::Binder vaoBinder(&m_imageVao);

		glDrawArrays(GL_TRIANGLES, 0, m_imageNumberVertices);

		m_imageProgram->release();
	}

	if (m_frustumProgram && m_frustumNumberVertices > 0)
	{
		// Setup matrices
		const auto normalMatrix = worldMatrix().normalMatrix();
		const auto viewMatrix = camera.viewMatrix();
		const auto projectionMatrix = camera.projectionMatrix();
		const auto pvMatrix = projectionMatrix * viewMatrix;
		const auto pvmMatrix = pvMatrix * worldMatrix();

		m_frustumProgram->bind();

		// Update matrices
		m_frustumProgram->setUniformValue("P", projectionMatrix);
		m_frustumProgram->setUniformValue("V", viewMatrix);
		m_frustumProgram->setUniformValue("M", worldMatrix());
		m_frustumProgram->setUniformValue("N", normalMatrix);
		m_frustumProgram->setUniformValue("PV", pvMatrix);
		m_frustumProgram->setUniformValue("PVM", pvmMatrix);

		// Bind the VAO containing the patches
		QOpenGLVertexArrayObject::Binder vaoBinder(&m_frustumVao);

		glDrawArrays(GL_LINES, 0, m_frustumNumberVertices);

		m_frustumProgram->release();
	}
}

void PhysicalCamera::computeFrustumLocation(
	QVector3D& topLeft,
	QVector3D& topRight,
	QVector3D& bottomLeft,
	QVector3D& bottomRight) const
{
	const QVector3D eyeToAt((m_at - m_eye).normalized());
	const auto right = QVector3D::crossProduct(eyeToAt, m_up).normalized();

	const float halfHeight = m_focalLength * std::tan(qDegreesToRadians(m_fovy) / 2.f);
	const float halfWidth = m_aspectRatio * halfHeight;
	const auto imageCenter = m_eye + m_focalLength * eyeToAt;

	topLeft = imageCenter + m_up * halfHeight - right * halfWidth;
	topRight = imageCenter + m_up * halfHeight + right * halfWidth;
	bottomLeft = imageCenter - m_up * halfHeight - right * halfWidth;
	bottomRight = imageCenter - m_up * halfHeight + right * halfWidth;
}

void PhysicalCamera::initializeImage()
{
	QVector3D topLeft, topRight, bottomLeft, bottomRight;
	computeFrustumLocation(topLeft, topRight, bottomLeft, bottomRight);
	
	// A quad with UV coordinates
	std::vector<QVector3D> vertices = {
		// First triangle
		topRight,                      // Position
		{1.0, 1.0, 0.0},  // UV
		topLeft,                       // Position
		{0.0, 1.0, 0.0},  // UV
		bottomLeft,                    // Position
		{0.0, 0.0, 0.0},  // UV
		// Second triangle
		topRight,                      // Position
		{1.0, 1.0, 0.0},  // UV
		bottomLeft,                    // Position
		{0.0, 0.0, 0.0},  // UV
		bottomRight,                   // Position
		{1.0, 0.0, 0.0}   // UV
	};
	
	m_imageProgram->bind();
	
	// Init VAO
	m_imageVao.create();
	QOpenGLVertexArrayObject::Binder vaoBinder(&m_imageVao);

	// Init VBO
	m_imageVbo.create();
	m_imageVbo.bind();

	// Enable attributes
	const auto posLoc = 0;
	const auto uvLoc = 1;
	m_imageProgram->enableAttributeArray(posLoc);
	m_imageProgram->enableAttributeArray(uvLoc);
	m_imageProgram->setAttributeBuffer(posLoc, GL_FLOAT, 0, 3, 2 * sizeof(QVector3D));
	m_imageProgram->setAttributeBuffer(uvLoc, GL_FLOAT, sizeof(QVector3D), 3, 2 * sizeof(QVector3D));
	m_imageVbo.allocate(vertices.data(), vertices.size() * sizeof(QVector3D));
	m_imageNumberVertices = vertices.size() / 2;

	m_imageVbo.release();
	m_imageProgram->release();

	m_texture.destroy();
	m_texture.create();
	m_texture.setFormat(QOpenGLTexture::RGBA32F);
	m_texture.setMinificationFilter(QOpenGLTexture::Linear);
	m_texture.setMagnificationFilter(QOpenGLTexture::Linear);
	m_texture.setWrapMode(QOpenGLTexture::ClampToEdge);
	m_texture.setSize(m_image.width(), m_image.height());
	m_texture.setData(m_image.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
}

void PhysicalCamera::initializeFrustum()
{
	QVector3D topLeft, topRight, bottomLeft, bottomRight;
	computeFrustumLocation(topLeft, topRight, bottomLeft, bottomRight);

	// Lines
	std::vector<QVector3D> vertices = {
		// Quad
		topRight, topLeft,
		topLeft, bottomLeft,
		bottomLeft, bottomRight,
		bottomRight, topRight,
		// Frustum
		m_eye, topRight,
		m_eye, topLeft,
		m_eye, bottomLeft,
		m_eye, bottomRight
	};

	m_frustumProgram->bind();

	// Init VAO
	m_frustumVao.create();
	QOpenGLVertexArrayObject::Binder vaoBinder(&m_frustumVao);

	// Init VBO
	m_frustumVbo.create();
	m_frustumVbo.bind();

	// Enable attributes
	const auto posLoc = 0;
	m_frustumProgram->enableAttributeArray(posLoc);
	m_frustumProgram->setAttributeArray(posLoc, nullptr, 3, 0);
	m_frustumVbo.allocate(vertices.data(), vertices.size() * sizeof(QVector3D));
	m_frustumNumberVertices = vertices.size();

	m_frustumVbo.release();
	m_frustumProgram->release();
}
