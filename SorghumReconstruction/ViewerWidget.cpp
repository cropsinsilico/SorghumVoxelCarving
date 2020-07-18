#include "ViewerWidget.h"

#include <QWheelEvent>

ViewerWidget::ViewerWidget(QWidget *parent) :
	QOpenGLWidget(parent),
	m_logger(new QOpenGLDebugLogger(this)),
	m_camera({ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 }, 45.0f, 1.0f, 0.01f, 100.0f)
{
}

ViewerWidget::~ViewerWidget()
{
	cleanup();
}

void ViewerWidget::addObject(std::unique_ptr<Renderable> object)
{
	makeCurrent();
	object->initialize(context());
	doneCurrent();
	
	m_objects.push_back(std::move(object));

	update();
}

void ViewerWidget::sortObjects()
{
	// Sort renderable by increasing order
	std::sort(m_objects.begin(), m_objects.end(),
		      [](const std::unique_ptr<Renderable>& a,
			          const std::unique_ptr<Renderable>& b) -> bool
		{
			return a->order() < b->order();
		}
	);
}

void ViewerWidget::setCamera(const OrbitCamera& camera)
{
	m_camera = camera;

	// Update the aspect ratio
	m_camera.setAspectRatio(float(width()) / height());

	update();
}

void ViewerWidget::cleanup()
{
	makeCurrent();
	for (auto& object : m_objects)
	{
		object->cleanup();
	}
	doneCurrent();
}

void ViewerWidget::printInfo()
{
	qDebug() << "Vendor: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
	qDebug() << "Renderer: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
	qDebug() << "Version: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	qDebug() << "GLSL Version: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
}

void ViewerWidget::initializeGL()
{
	connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ViewerWidget::cleanup);

	initializeOpenGLFunctions();
	m_logger->initialize();
	glClearColor(0.5, 0.5, 0.5, 1.0);

	// Print OpenGL info and Debug messages
	printInfo();
}

void ViewerWidget::resizeGL(int w, int h)
{
	m_camera.setAspectRatio(float(w) / h);
}

void ViewerWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (auto& object : m_objects)
	{
		object->paint(context(), m_camera);
	}
}

void ViewerWidget::mousePressEvent(QMouseEvent* event)
{
	const auto x = event->globalX();
	const auto y = event->globalY();

	if (event->button() == Qt::LeftButton)
	{
		m_camera.mouseLeftButtonPressed(x, y);
	}
	else if (event->button() == Qt::RightButton)
	{
		m_camera.mouseRightButtonPressed(x, y);
	}

	update();
}

void ViewerWidget::mouseReleaseEvent(QMouseEvent* event)
{
	m_camera.mouseReleased();

	update();
}

void ViewerWidget::mouseMoveEvent(QMouseEvent* event)
{
	const auto x = event->globalX();
	const auto y = event->globalY();

	m_camera.mouseMoved(x, y);

	update();
}

void ViewerWidget::wheelEvent(QWheelEvent* event)
{
	// Default speed
	float speed = 1.0;

	if (event->modifiers() & Qt::ShiftModifier)
	{
		// If shift is used, zooming is 4 times faster
		speed = 4.0;
	}
	else if (event->modifiers() & Qt::ControlModifier)
	{
		// If control is used, zooming is twice slower
		speed = 0.5;
	}

	const auto numDegrees = event->angleDelta() / 8;
	const auto numSteps = numDegrees / 15;
	m_camera.zoom(speed * numSteps.y());

	update();
}
