#pragma once

#include <QObject>

#include "AABB.h"

enum class CommandType
{
	NoCommand,
	Calibration,
	CalibrationTop,
	Reconstruction,
	Density,
	Directionality,
	Surface,
	Height,
	ProcessSkeleton,
	TrainSkeletonClassifier
};

CommandType readCommandTypeFromString(const std::string& command);

struct ConsoleApplicationParameters
{
	CommandType commandType;
	QString inputFile;
	QString outputFile;
};

class ConsoleApplication : public QObject
{
	Q_OBJECT

public:
	explicit ConsoleApplication(ConsoleApplicationParameters parameters, QObject *parent = Q_NULLPTR);

signals:
	/**
	 * \brief This signal is emitted when the application has finished
	 * \param returnCode Return code of the application
	 */
	void finished(int returnCode);

public slots:
	/**
	 * \brief Execute the console application
	 */
	void exec();

private:
	/**
	 * \brief Run the calibration for a side image
	 * \return True if calibration was successful, otherwise false
	 */
	bool runCalibrateSideImage();

	/**
	 * \brief Run the calibration for a top image
	 * \return True if calibration was successful, otherwise false
	 */
	bool runCalibrateTopImage();

	/**
	 * \brief Run the voxel reconstruction
	 * \return True if reconstruction was successful, otherwise false
	 */
	bool runReconstruction();

	/**
	 * \brief Run the density evaluation
	 * \return True if the density evaluation was successful
	 */
	bool runDensity();

	/**
	 * \brief Run the directionality evaluation
	 * \return True if the directionality evaluation was successful
	 */
	bool runDirectionality();

	/**
	 * \brief Run the surface evaluation
	 * \return True if the surface evaluation was successful
	 */
	bool runSurface();

	/**
	 * \brief Run the height evaluation
	 * \return True if the height evaluation was successful
	 */
	bool runHeight();

	/**
	 * \brief Run the skeleton improvement
	 * \return True if the skeleton improvement was successful
	 */
	bool processSkeleton();

	/**
	 * \brief Run the skeleton classifier training 
	 * \return True if the training was successful
	 */
	bool trainSkeletonClassifier();
	
	ConsoleApplicationParameters m_parameters;

	AABB m_objectBoundingBox;
	int m_resolution;

	const std::vector<std::pair<float, bool>> m_imageAngles;
	const std::vector<std::string> m_imageNames;
};
