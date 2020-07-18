#pragma once

#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>

class Camera
{
public:

	/**
	 * \brief Create a camera
	 * \param eye The position of the eye
	 * \param at The position of the at, where the camera is looking
	 * \param up The up vector
	 * \param fovy Fovy of the camera
	 * \param aspectRatio Aspect ratio of the camera
	 * \param nearPlane Distance to the near plane
	 * \param farPlane Distance to the far plane
	 */
	Camera(const QVector3D& eye,
		   const QVector3D& at,
		   const QVector3D& up,
		   float fovy,
		   float aspectRatio,
		   float nearPlane,
		   float farPlane);

	/**
	 * \brief Return the view matrix
	 * \return The view matrix
	 */
	const QMatrix4x4& viewMatrix() const;

	/**
	 * \brief Return the projection matrix
	 * \return The projection matrix
	 */
	const QMatrix4x4& projectionMatrix() const;

	/**
	 * \brief Return the position of the eye
	 * \return The position of the eye
	 */
	const QVector3D& eye() const;

	/**
	 * \brief Return the position of the at, where the camera is looking
	 * \return The position of the at, where the camera is looking
	 */
	const QVector3D& at() const;

	/**
	 * \brief Return the up vector
	 * \return The up vector
	 */
	const QVector3D& up() const;

	/**
	 * \brief Compute and return the right vector
	 * \return The right vector
	 */
	QVector3D right() const;

	/**
	 * \brief Return the fovy of the camera
	 * \return The fovy of the camera
	 */
	float fovy() const;

	/**
	 * \brief Return the aspect ratio of the camera
	 * \return The aspect ratio of the camera
	 */
	float aspectRatio() const;

	/**
	 * \brief Return the distance to the near plane
	 * \return The distance to the near plane
	 */
	float nearPlane() const;

	/**
	 * \brief Return the distance to the far plane
	 * \return The distance to the far plane
	 */
	float farPlane() const;

	/**
	 * \brief Set the position of the eye
	 * \param eye The position of the eye
	 */
	void setEye(const QVector3D& eye);

	/**
	 * \brief Set the position of the at, where the camera is looking
	 * \param at The position of the at, where the camera is looking
	 */
	void setAt(const QVector3D& at);

	/**
	 * \brief Set the up vector
	 * \param up The up vector
	 */
	void setUp(const QVector3D& up);

	/**
	 * \brief Change the fovy of the camera
	 * \param fovy The fovy in radians
	 */
	void setFovy(float fovy);

	/**
	 * \brief Set the aspect ratio of the camera
	 * \param aspectRatio The aspect ratio of the camera
	 */
	void setAspectRatio(float aspectRatio);

	/**
	 * \brief Set the distance to the near plane
	 * \param nearPlane The distance to the near plane
	 */
	void setNearPlane(float nearPlane);

	/**
	 * \brief Set the distance to the far plane
	 * \param farPlane The distance to the far plane
	 */
	void setFarPlane(float farPlane);

	/**
	 * \brief Rotate the camera in the left right direction, around the up vector
	 * \param angle The angle of the rotation in radians
	 */
	void roundLeftRight(float angle);

	/**
	 * \brief Rotate the camera in the up down direction
	 * \param angle The angle of the rotation in radians
	 */
	void roundUpDown(float angle);

	/**
	 * \brief Move the camera in the left right direction
	 * \param distance The distance the camera will move
	 */
	void moveLeftRight(float distance);

	/**
	 * \brief Move the camera in the direction of the up vector
	 * \param distance The distance the camera will move
	 */
	void moveUpDown(float distance);

	/**
	 * \brief Move the camera in the direction of the at point
	 * \param distance The distance the camera will move
	 */
	void moveForth(float distance);

	/**
	 * \brief Project a point in 3D to the camera
	 * \param point A point in the 3D space
	 * \param viewportWidth Width of the viewport
	 * \param viewportHeight Height of the viewport
	 * \return The 2D coordinates of the point on the 2D screen of the camera
	 */
	QVector2D project(const QVector3D& point, float viewportWidth, float viewportHeight) const;

	/**
	 * \brief Render a mesh to an image. The background is white and the mesh is black.
	 * \param worldMatrix A transformation matrix for the mesh
	 * \param vertices Vertices of the mesh
	 * \param faces Faces of the mesh
	 * \param viewportWidth Width of the rendered image
	 * \param viewportHeight Height of the rendered image 
	 * \return 
	 */
	QImage render(const QMatrix4x4& worldMatrix,
				  const std::vector<QVector3D>& vertices,
				  const std::vector<std::tuple<int, int, int>>& faces,
				  float viewportWidth,
				  float viewportHeight) const;

private:

	/**
	 * \brief Update the view and projection matrices
	 */
	void updateMatrices();
	
	QVector3D m_eye;
	QVector3D m_at;
	QVector3D m_up;

	float m_fovy;
	float m_aspectRatio;
	float m_near;
	float m_far;

	QMatrix4x4 m_viewMatrix;
	QMatrix4x4 m_projectionMatrix;
};

class OrbitCamera : public Camera
{
public:

	/**
	 * \brief Create an Orbit Camera from an abstract camera
	 * \param camera A Camera
	 */
	explicit OrbitCamera(const Camera& camera);

	/**
	 * \brief Create an Orbit Camera
	 * \param eye The position of the eye
	 * \param at The position of the at, where the camera is looking
	 * \param up The up vector
	 * \param fovy Fovy of the camera
	 * \param aspectRatio Aspect ratio of the camera
	 * \param nearPlane Distance to the near plane
	 * \param farPlane Distance to the far plane
	 */
	OrbitCamera(const QVector3D& eye,
				const QVector3D& at,
				const QVector3D& up,
				float fovy,
				float aspectRatio,
				float nearPlane,
				float farPlane);

	/**
	 * \brief Call this function when the user press the left mouse button
	 * \param x X coordinates of the mouse
	 * \param y Y coordinates of the mouse
	 */
	void mouseLeftButtonPressed(int x, int y);

	/**
	 * \brief Call this function when the user press the right mouse button
	 * \param x X coordinates of the mouse
	 * \param y Y coordinates of the mouse
	 */
	void mouseRightButtonPressed(int x, int y);

	/**
	 * \brief Call this function each time the user move the mouse
	 * \param x X coordinates of the mouse
	 * \param y Y coordinates of the mouse
	 */
	void mouseMoved(int x, int y);

	/**
	 * \brief Call this function when the user release the mouse button
	 */
	void mouseReleased();

	/**
	 * \brief Move the camera in the direction of the at point
	 * \param distance The distance the camera will move
	 */
	void zoom(float distance);

private:
	float m_roundMotionSensitivity;
	float m_moveMotionSensitivity;

	bool m_mouseLeftButtonHold;
	bool m_mouseRightButtonHold;
	int m_x0;
	int m_y0;
};
