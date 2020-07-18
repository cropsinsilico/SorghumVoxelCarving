#pragma once

#include <vector>

#include "Camera.h"

/**
 * \brief Generate a set of camera
 * \param cameraAzimuthalAngles A vector of azimuthal angles in degrees
 * \param polarAngle The polar angle (deg) of the circle in which cameras are laid out (flat is 90 degrees)
 * \param includeTopCamera Whether or not to include a top camera
 * \return A vector of Cameras
 */
std::vector<Camera> generateCameras(
	const std::vector<float>& cameraAzimuthalAngles,
	float polarAngle,
	bool includeTopCamera
);

/**
 * \brief Generate a set of camera
 * \param imageAngles A table with a set of pair (angle, isTop)
 * \param polarAngle The polar angle (deg) of the circle in which cameras are laid out (flat is 90 degrees)
 * \return A vector of Cameras
 */
std::vector<Camera> generateCameras(
    const std::vector<std::pair<float, bool>>& imageAngles,
	float polarAngle
);