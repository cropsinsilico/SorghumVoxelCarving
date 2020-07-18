#include "Reconstruction.h"

#include <QtMath>

std::vector<Camera> generateCameras(
	const std::vector<float>& cameraAzimuthalAngles,
	float polarAngle,
	bool includeTopCamera)
{
	const float sensorAspectRatio = 2454.0 / 2056.0;
	// Sensor width in mm
	const float sensorWidth = 8.466;
	const float sensorHeight = sensorWidth / sensorAspectRatio;

	// Focal in mm
	const float sideFocal = 26.5;
	const float topFocal = 26.5;
	
	const auto sideFovy = qRadiansToDegrees(2.0f * std::atan(sensorHeight / (2.0f * sideFocal)));
	const auto topFovy = qRadiansToDegrees(2.0f * std::atan(sensorHeight / (2.0f * topFocal)));
	
	const auto radius = 5.5f;
	const auto phi = qDegreesToRadians(polarAngle);

	std::vector<Camera> cameras;

	// Camera settings
	for (float angle : cameraAzimuthalAngles)
	{
		const QVector3D eye(radius * std::sin(qDegreesToRadians(angle)) * std::sin(phi),
			radius * (1.f - std::cos(qDegreesToRadians(angle)) * std::sin(phi)),
			radius * std::cos(phi));

		const QVector3D at(0.0, 5.5, 0.0);

		// Similar to the eye vector, with counter clockwise rotation (x'=-y, y'=x)
		const QVector3D right(std::cos(qDegreesToRadians(angle)) * std::sin(phi),
			std::sin(qDegreesToRadians(angle)) * std::sin(phi),
			0.0);

		const auto eyeToAt = (at - eye).normalized();

		cameras.emplace_back(
			eye,
			at,
			QVector3D::crossProduct(right, eyeToAt).normalized(),
			sideFovy,
			sensorAspectRatio,
			0.01f,
			10.0f
		);
	}

	if (includeTopCamera)
	{
		// Top camera
		cameras.emplace_back(
			QVector3D(
				0.0f,
				5.5f,
				3.6f),
			QVector3D(0.0, 5.5, 0.0),
			QVector3D(0.0, 1.0, 0.0),
			topFovy,
			sensorAspectRatio,
			0.01f,
			10.0f
		);
	}

	return cameras;
}

std::vector<Camera> generateCameras(
	const std::vector<std::pair<float, bool>>& imageAngles,
	float polarAngle)
{
	// Camera images
	bool includeTopCamera = false;
	std::vector<float> cameraAngles;
	for (auto& imageAngle : imageAngles)
	{
		// If the current camera is the top camera
		if (imageAngle.second)
		{
			includeTopCamera = true;
		}
		else
		{
			// If it is a side camera, take the angle in account
			cameraAngles.push_back(imageAngle.first);
		}
	}

	return generateCameras(cameraAngles, polarAngle, includeTopCamera);
}
