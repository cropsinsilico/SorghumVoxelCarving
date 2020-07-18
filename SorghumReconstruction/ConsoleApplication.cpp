#include "ConsoleApplication.h"


#include <fstream>
#include <random>
#include <utility>

#include <QDir>
#include <QFileInfo>

#include <opencv2/imgcodecs.hpp>


#include "AABB.h"
#include "Calibration.h"
#include "IoUtils.h"
#include "MathUtils.h"
#include "Reconstruction.h"
#include "Skeletons.h"
#include "CvSkeletonBranchClassifier.h"
#include "ThresholdSkeletonBranchClassifier.h"
#include "VoxelCarver.h"

CommandType readCommandTypeFromString(const std::string& command)
{
	if (command == "calibration")
	{
		return CommandType::Calibration;
	}
	else if (command == "calibration_top")
	{
		return CommandType::CalibrationTop;
	}
	else if (command == "reconstruction")
	{
		return CommandType::Reconstruction;
	}
	else if (command == "density")
	{
		return CommandType::Density;
	}
	else if (command == "directionality")
	{
		return CommandType::Directionality;
	}
	else if (command == "surface")
	{
		return CommandType::Surface;
	}
	else if (command == "height")
	{
		return CommandType::Height;
	}
	else if (command == "process_skeleton")
	{
		return CommandType::ProcessSkeleton;
	}
	else if (command == "train_classifier")
	{
		return CommandType::TrainSkeletonClassifier;
	}
	else
	{
		return CommandType::NoCommand;
	}
}

ConsoleApplication::ConsoleApplication(ConsoleApplicationParameters parameters, QObject *parent) :
	QObject(parent),
	m_parameters(std::move(parameters)),
	m_objectBoundingBox({ 0.0, 5.5, 0.0 }, 0.5),
	m_resolution(512),
	// Pair <angle, isTopCamera>
	m_imageAngles({
		{0.0, false},
		{36.0, false},
		{72.0, false},
		{108.0, false},
		{144.0, false},
		{180.0, false},
		{216.0, false},
		{252.0, false},
		{288.0, false},
		{324.0, false},
		{0.0, true}
	}),
	// Files containing the camera pictures
	m_imageNames({
		"0_0_0.png",
		"0_36_0.png",
		"0_72_0.png",
		"0_108_0.png",
		"0_144_0.png",
		"0_180_0.png",
		"0_216_0.png",
		"0_252_0.png",
		"0_288_0.png",
		"0_324_0.png",
		"top_0_90_0.png"
	})
{
	
}

void ConsoleApplication::exec()
{	
	// returnCode is either 0 if the output is successfully saved, or 1 if a problem occurred
	int returnCode = 1;

	if (m_parameters.commandType == CommandType::Calibration)
	{
		if (runCalibrateSideImage())
		{
			returnCode = 0;
		}
	}
	else if (m_parameters.commandType == CommandType::CalibrationTop)
	{
		if (runCalibrateTopImage())
		{
			returnCode = 0;
		}
	}
	else if (m_parameters.commandType == CommandType::Reconstruction)
	{
		if (runReconstruction())
		{
			returnCode = 0;
		}
	}
	else if (m_parameters.commandType == CommandType::Density)
	{
		if (runDensity())
		{
			returnCode = 0;
		}
	}
	else if (m_parameters.commandType == CommandType::Directionality)
	{
		if (runDirectionality())
		{
			returnCode = 0;
		}
	}
	else if (m_parameters.commandType == CommandType::Surface)
	{
		if (runSurface())
		{
			returnCode = 0;
		}
	}
	else if (m_parameters.commandType == CommandType::Height)
	{
		if (runHeight())
		{
			returnCode = 0;
		}
	}
	else if (m_parameters.commandType == CommandType::ProcessSkeleton)
	{
		if (processSkeleton())
		{
			returnCode = 0;
		}
	}
	else if (m_parameters.commandType == CommandType::TrainSkeletonClassifier)
	{
		if (trainSkeletonClassifier())
		{
			returnCode = 0;
		}
	}

	emit finished(returnCode);
}

bool ConsoleApplication::runCalibrateSideImage()
{
	return autoCalibrationSideImage(m_parameters.inputFile, m_parameters.outputFile);
}

bool ConsoleApplication::runCalibrateTopImage()
{
	return autoCalibrationTopImage(m_parameters.inputFile, m_parameters.outputFile);
}

bool ConsoleApplication::runReconstruction()
{
	const bool extractMajorComponent = false;

	assert(m_imageNames.size() == m_imageAngles.size());

	// Input/Output directory
	const QDir inputDir(m_parameters.inputFile);
	const QDir outputDir(m_parameters.outputFile);

	if (!outputDir.exists())
	{
		qWarning() << "Output directory does not exist";
		return false;
	}
	
	// Camera images
	std::vector<std::pair<float, bool>> availableAngles;
	std::vector<cv::Mat> cameraImages;
	for (unsigned int i = 0; i < m_imageNames.size(); i++)
	{
		if (inputDir.exists(QString::fromStdString(m_imageNames[i])))
		{
			const auto fileName = inputDir.filePath(QString::fromStdString(m_imageNames[i]));
			const auto cvImage = cv::imread(fileName.toStdString());
			if (!cvImage.empty())
			{
				// Add the image
				cameraImages.push_back(cvImage);
				availableAngles.push_back(m_imageAngles[i]);
			}
		}
	}
	
	if (cameraImages.empty())
	{
		qWarning() << "No image available for reconstruction";
		return false;
	}

	const auto cameras = generateCameras(availableAngles, 90.f);
	
	// 3D reconstruction
	VoxelCarver carver(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
	
	carver.clearCameras();
	for (unsigned int i = 0; i < cameraImages.size(); i++)
	{
		carver.addCameraImage(cameras[i], cameraImages[i], 0, 0);
	}
	carver.setMaximumRadiusAroundVoxel(std::max({
		carver.voxelGrid().voxelSizeX() / 4.0f,
		carver.voxelGrid().voxelSizeY() / 4.0f,
		carver.voxelGrid().voxelSizeZ() / 4.0f
	}));

	// Process the voxel grid and save it
	carver.process();
	// Extract the major connected component if needed
	const auto grid = (extractMajorComponent) ? extractMajorConnectedComponent(carver.voxelGrid()) : carver.voxelGrid();
	
	// Save the voxel plant as OBJ
	grid.exportVoxels(outputDir.absoluteFilePath("voxels.txt").toStdString());
	grid.saveAsOBJ(outputDir.absoluteFilePath("voxel_centers.obj").toStdString());
	grid.saveVoxelsAsOBJ(outputDir.absoluteFilePath("voxel_cubes.obj").toStdString());
	grid.exportMesh(outputDir.absoluteFilePath("plant_mesh.obj").toStdString());

	// Compute the re-projection error 
	std::vector<cv::Mat> reprojections;
	const auto error = carver.reprojectionError(grid, reprojections);
	// The output error in a text file
	writeToFile(outputDir.absoluteFilePath("error.txt").toStdString(), error);
	// Output reprojection images
	for (unsigned int i = 0; i < reprojections.size(); i++)
	{
		// The filename contains the angle of the camera
		QString filename = "reprojection_";
		if (availableAngles[i].second)
		{
			filename += "top.png";
		}
		else
		{
			filename += QString::number(availableAngles[i].first) + ".png";
		}
		
		cv::imwrite(outputDir.absoluteFilePath(filename).toStdString(), reprojections[i]);
	}

	return true;
}

bool ConsoleApplication::runDensity()
{
	VoxelGrid grid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
	grid.importVoxels(m_parameters.inputFile.toStdString());

	const auto volume = boundingCylinderVolume(grid);

	// Display the volume in the console
	qInfo() << volume;

	return true;
}

bool ConsoleApplication::runDirectionality()
{
	VoxelGrid grid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
	grid.importVoxels(m_parameters.inputFile.toStdString());

	const auto directionality = computeDirectionality(grid);

	// Display the directionality in the console
	qInfo() << directionality;

	return true;
}

bool ConsoleApplication::runSurface()
{
	VoxelGrid grid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
	grid.importVoxels(m_parameters.inputFile.toStdString());

	const auto numberVoxels = countNumberSurfaceVoxels(grid);

	// Display the number of voxels in the console
	qInfo() << numberVoxels;

	return true;
}

bool ConsoleApplication::runHeight()
{
	VoxelGrid grid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
	grid.importVoxels(m_parameters.inputFile.toStdString());

	float topVoxelAltitude = 0.0f;
	
	if (!grid.empty())
	{
		topVoxelAltitude = computeHeight(grid);
	}

	// Display the maximum Z coordinate number of a voxel in the grid
	qInfo() << topVoxelAltitude;

	return true;
}

bool ConsoleApplication::processSkeleton()
{
	const QString voxelFilename = "voxels.txt";
	const QString skeletonFilename = "skeleton.txt";
	const int width = 2454;
	const int height = 2056;
	
	const QDir inputDir(m_parameters.inputFile);
	const QDir outputDir(m_parameters.outputFile);

	if (!inputDir.exists(voxelFilename) || !inputDir.exists(skeletonFilename))
	{
		qWarning() << "Some input files are missing";
		return false;
	}
	if (!outputDir.exists())
	{
		qWarning() << "Output directory does not exist";
		return false;
	}
	if (!QFileInfo::exists("model.yml"))
	{
		qWarning() << "Cannot load the classifier model";
		return false;
	}

	SvmRbfSkeletonBranchClassifier classifier;
	classifier.load("model.yml");

	VoxelGrid carvingGrid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
	carvingGrid.importVoxels(inputDir.filePath(voxelFilename).toStdString());
	
	VoxelGrid skeletonGrid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
	skeletonGrid.importVoxels(inputDir.filePath(skeletonFilename).toStdString());

	if (skeletonGrid.empty())
	{
		qWarning() << "The raw skeleton is empty";
		return false;
	}
	
	// Run voxel skeleton optimization
	// Find endpoints in the skeleton
	// Sorting voxels is mandatory to accelerate algorithms
	skeletonGrid.sortVoxels();
	auto endpoints = extractSkeletonEndpoints(skeletonGrid);

	// Find the lowest endpoint, which is most probably the root of the plant
	// TODO: Add the constraint that the endpoint is next to the pot
	const auto rootVoxel = findAndRemoveLowestEndpoint(endpoints);
	std::vector<std::vector<Voxel>> skeletonPaths;
	std::vector<int> voxelPrecedence;
	std::tie(skeletonPaths, voxelPrecedence) = shortestPathsFromSkeleton(skeletonGrid, rootVoxel, endpoints);
	std::vector<std::vector<Voxel>> filteredSkeletonPaths;
	std::vector<bool> skeletonPathSelected;
	std::tie(filteredSkeletonPaths, skeletonPathSelected) = filterShortestPathsClassifier(skeletonPaths, classifier);
	const auto segmentedSkeletonPaths = segmentShortestPaths(filteredSkeletonPaths);

	// Check that the plant has the correct topology
	const auto plantTopology = checkPathHasNoJunction(segmentedSkeletonPaths.back(), skeletonGrid, voxelPrecedence);
	writeToFile(outputDir.absoluteFilePath("topology.txt").toStdString(), plantTopology);

	// Convert the skeleton to a voxel grid
	const auto optimSkeletonGrid = generateGridFromSkeleton(skeletonGrid, segmentedSkeletonPaths);

	// Export the skeleton
	optimSkeletonGrid.exportVoxels(outputDir.absoluteFilePath("optim_skeleton.txt").toStdString());
	optimSkeletonGrid.saveVoxelsAsOBJ(outputDir.absoluteFilePath("optim_skeleton.obj").toStdString());

	// Compute the maximum distance between the voxels and the skeleton and output it in a file
	const auto error = maximumNearestDistanceFromGridToGrid(carvingGrid, optimSkeletonGrid);
	writeToFile(outputDir.filePath("error.txt").toStdString(), error);
	// Render the raw skeleton
	renderGridAndSave(skeletonGrid, m_imageAngles, width, height, "raw_skeleton", outputDir);
	// Render the optimized skeleton
	renderGridAndSave(optimSkeletonGrid, m_imageAngles, width, height, "optim_skeleton", outputDir);
	
	return true;
}

bool ConsoleApplication::trainSkeletonClassifier()
{
	const float ratioValidationSet = 0.8f;
	const float ratioTestSet = 0.8f;

	using BranchClassifier = SvmRbfSkeletonBranchClassifier;
	
	const auto inputDir = m_parameters.inputFile.toStdString();
	
	// Load the data set
	auto fileNames = BranchClassifier::listPathFilesInFolder(inputDir);
	auto pathsLabels = BranchClassifier::readPathAndLabelFilesFromFolder(inputDir);

	if (pathsLabels.empty())
	{
		qWarning() << "Could not load the training set";
		return false;
	}
	
	// Split the data set
	std::seed_seq seed{ 11, 16, 1994 };
	std::mt19937 randomGenerator(seed);
	std::shuffle(pathsLabels.begin(), pathsLabels.end(), randomGenerator);
	decltype(pathsLabels) pathsLabelsTrain;
	decltype(pathsLabels) pathsLabelsTest;
	std::tie(pathsLabelsTrain, pathsLabelsTest) = BranchClassifier::splitPathAndLabels(pathsLabels,
		                                                                               ratioTestSet);
	
	// Generate pairs of labeled paths for training
	const auto pathPairsTrain = BranchClassifier::generatePathPairsFromGroundTruth(pathsLabelsTrain);
	const auto pathPairsTest = BranchClassifier::generatePathPairsFromGroundTruth(pathsLabelsTest);

	// Display data set size
	qInfo() << "Training + Validation samples:" << pathsLabelsTrain.size();
	qInfo() << "Test samples: " << pathsLabelsTest.size();
	qInfo() << "Training + Validation pairs:" << pathPairsTrain.size();
	qInfo() << "Test pairs: " << pathPairsTest.size();
	// For reproducibility, output the names of files in the data set.
	// The first files are in the training set, the last files are in the test set.
	randomGenerator.seed(seed);
	std::shuffle(fileNames.begin(), fileNames.end(), randomGenerator);

	// Display files in the data set.
	// The first part belong to the training set, the last part belong to the test set.
	qInfo() << "Files in the data set:";
	for (const auto& file : fileNames)
	{
		qInfo() << QString::fromStdString(file);
	}
	
	BranchClassifier classifier;
	classifier.train(pathPairsTrain, ratioValidationSet);
	classifier.save(m_parameters.outputFile.toStdString());

	// Evaluation on test set
	qInfo() << "Percentage of misclassified branches in test set: "
	        << classifier.evaluateClassifier(pathPairsTest);
	
	// Evaluate on whole skeletons
	const auto evalResultTrain = classifier.evaluateFilter(pathsLabelsTrain);
	const auto evalResultTest = classifier.evaluateFilter(pathsLabelsTest);

	const auto precisionTrain = evalResultTrain.first;
	const auto precisionTest = evalResultTest.first;
	const auto recallTrain = evalResultTrain.second;
	const auto recallTest = evalResultTest.second;
	const auto fmeasureTrain = 2.0f * (precisionTrain * recallTrain) / (precisionTrain + recallTrain);
	const auto fmeasureTest = 2.0f * (precisionTest * recallTest) / (precisionTest + recallTest);

	qInfo() << "Training set";
	qInfo() << "\tprecision = " << precisionTrain;
	qInfo() << "\trecall = " << recallTrain;
	qInfo() << "\tfmeasure = " << fmeasureTrain;

	qInfo() << "Test set";
	qInfo() << "\tprecision = " << precisionTest;
	qInfo() << "\trecall = " << recallTest;
	qInfo() << "\tfmeasure = " << fmeasureTest;
	
	return true;
}
