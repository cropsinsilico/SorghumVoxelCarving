#include "MainWindow.h"

#include <array>
#include <vector>
#include <random>
#include <chrono>

#include <QTimer>
#include <QtMath>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Cylinder.h"
#include "PhysicalCamera.h"
#include "VoxelCarver.h"
#include "VoxelObject.h"
#include "MathUtils.h"
#include "Thinning.h"
#include "Skeletons.h"
#include "Calibration.h"
#include "IoUtils.h"
#include "OBJReader.h"
#include "MeshObject.h"
#include "Reconstruction.h"
#include "CvSkeletonBranchClassifier.h"
#include "StatsUtils.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	m_pathToImages("Images/plant_281_2_calibrated/"),
	m_objectBoundingBox({ -0.5, 5.0, -0.5 }, { 0.5, 6.0, 0.5 }),
	m_resolution(512),
	m_useTopImage(true), // Use the top image (needs to be manually calibrated)
	m_needsCalibration(false),
	m_extractMajorComponent(true),
	m_maximumRadiusAroundVoxel(0.000f) // 0 centimeters
{
	setupUi();

	QTimer::singleShot(0, this, &MainWindow::setupScene);
}

void MainWindow::setupScene()
{
	// Clear the view
	m_ui.viewerWidget->cleanup();

	// Setup the initial scene
	loadCameraImages(QString::fromStdString(m_pathToImages));
	setupCameras();
	setupPot();
	
	runVoxelCarving();
	// processSkeletonAndSetupVoxelGrid();
	// showVoxelizedMesh();
	// benchmarkNumberCameras();
	// benchmarkCameraOffset();
	// benchmarkMetrics();
	
	setupPhysicalCameras();
}

void MainWindow::openImages()
{
	// Prompt the user to open a directory and open images in it
	// Ask the user for a file to import
	const auto inputDir = QFileDialog::getExistingDirectory(this, tr("Load images in a directory"), "");

	// Clear the view
	m_ui.viewerWidget->cleanup();
	
	loadCameraImages(inputDir);
	setupCameras();
	setupPot();
	setupPhysicalCameras();
}

void MainWindow::openPlant()
{
	// Ask the user for a file to import
	const QString inputFile = QFileDialog::getOpenFileName(this,
				                                           tr("Load a plant"),
				                                           "",
				                                           tr("Plant (*.obj *.txt)"));

	const QFileInfo inputFileInfo(inputFile);

	// Check if file exists
	if (inputFileInfo.exists())
	{
		VoxelGrid grid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
		
		// Depending on the file extension, use a different loader
		if (inputFileInfo.suffix().toLower() == "txt")
		{
			grid.importVoxels(inputFileInfo.filePath().toStdString());
		}
		else if (inputFileInfo.suffix().toLower() == "obj")
		{
			grid.importOBJ(inputFileInfo.filePath().toStdString());
		}

		// If something has been successfully loaded
		if (!grid.empty())
		{
			// Voxel grid
			QMatrix4x4 voxelGridMatrix;
			voxelGridMatrix.translate(0.0, 0.0, 0.0);
			auto voxelGridObject = std::make_unique<VoxelObject>(1, voxelGridMatrix, grid);
			m_ui.viewerWidget->addObject(std::move(voxelGridObject));
			m_ui.viewerWidget->sortObjects();
		}
	}
}

void MainWindow::selectCameraClicked(int camera)
{
	if (camera >= 0 && camera < m_cameras.size())
	{
		// Set the camera in the viewer
		const OrbitCamera orbitCamera(m_cameras[camera]);
		m_ui.viewerWidget->setCamera(orbitCamera);
	}
}

void MainWindow::calibrateSideImage()
{
	// Ask the user for a file to import
	const QString inputFile = QFileDialog::getOpenFileName(this, 
		                                                   tr("Load an image"), 
		                                                   "", 
		                                                   tr("Images (*.png *.jpg)"));

	// Ask the use for a file to export
	const QString outputFile = QFileDialog::getSaveFileName(this, 
		                                                    tr("Save calibrated image"), 
		                                                    "", 
		                                                    tr("Images (*.png *.jpg)"));

	// Check if file exists
	if (QFileInfo::exists(inputFile) && !outputFile.isEmpty())
	{
		if (!autoCalibrationSideImage(inputFile, outputFile))
		{
			QMessageBox::critical(this,
			                      tr("Error while saving"),
			                      tr("Impossible to save the calibrated image"));
		}
	}
}

void MainWindow::calibrateTopImage()
{
	// Ask the user for a file to import
	const QString inputFile = QFileDialog::getOpenFileName(this, 
		                                                   tr("Load an image"), 
		                                                   "", 
		                                                   tr("Images (*.png *.jpg)"));

	// Ask the use for a file to export
	const QString outputFile = QFileDialog::getSaveFileName(this, 
		                                                    tr("Save calibrated image"), 
		                                                    "", 
		                                                    tr("Images (*.png *.jpg)"));
	
	// Check if file exists
	if (QFileInfo::exists(inputFile) && !outputFile.isEmpty())
	{
		if (!autoCalibrationTopImage(inputFile, outputFile))
		{
			QMessageBox::critical(this,
			                      tr("Error while saving"),
			                      tr("Impossible to save the calibrated image"));
		}
	}
}

void MainWindow::interpolationSliderChanged()
{
	// TODO
}

void MainWindow::setupUi()
{
	m_ui.setupUi(this);

	connect(m_ui.actionOpen_Images, &QAction::triggered, this, &MainWindow::openImages);
	connect(m_ui.actionOpen_Plant, &QAction::triggered, this, &MainWindow::openPlant);
	
	connect(m_ui.actionSelect_Camera_1, &QAction::triggered, [this]() { selectCameraClicked(0); });
	connect(m_ui.actionSelect_Camera_2, &QAction::triggered, [this]() { selectCameraClicked(1); });
	connect(m_ui.actionSelect_Camera_3, &QAction::triggered, [this]() { selectCameraClicked(2); });
	connect(m_ui.actionSelect_Camera_4, &QAction::triggered, [this]() { selectCameraClicked(3); });
	connect(m_ui.actionSelect_Camera_5, &QAction::triggered, [this]() { selectCameraClicked(4); });
	connect(m_ui.actionSelect_Camera_6, &QAction::triggered, [this]() { selectCameraClicked(5); });

	connect(m_ui.actionCalibrate_side_image, &QAction::triggered, this, &MainWindow::calibrateSideImage);
	connect(m_ui.actionCalibrate_top_image, &QAction::triggered, this, &MainWindow::calibrateTopImage);

	connect(m_ui.actionRun_voxel_carving, &QAction::triggered, this, &MainWindow::runVoxelCarving);
	connect(m_ui.actionUse_top_image, &QAction::triggered, this, &MainWindow::changeParameters);
	connect(m_ui.actionExtract_major_component, &QAction::triggered, this, &MainWindow::changeParameters);

	connect(m_ui.interpolationSlider, &QSlider::valueChanged, this, &MainWindow::interpolationSliderChanged);
}

void MainWindow::loadCameraImages(const QString& directory)
{
	const QFileInfo dirInfo(directory);

	// Check if directory exists
	if (dirInfo.isDir())
	{
		// Names of files in the directory
		std::vector<QString> fileNames = {
			"0_0_0.png",
			"0_72_0.png",
			"0_144_0.png",
			"0_216_0.png",
			"0_288_0.png"
		};

		// Whether we include the top image or not
		if (m_useTopImage)
		{
			fileNames.emplace_back("top_0_90_0.png");
		}

		// Clear current images
		m_cameraImages.clear();

		// Check that all files are present in the directory and load them
		for (const auto& file : fileNames)
		{
			const auto filePath = dirInfo.canonicalFilePath() + "/" + file;

			if (QFileInfo::exists(filePath))
			{
				m_cameraImages.push_back(cv::imread(filePath.toStdString()));
			}
			else
			{
				// File not present
				QMessageBox::critical(this,
					                  tr("Error while loading an image"),
					                  tr("Impossible to load the image: ") + file);
			}
		}
	}
}

void MainWindow::setupCameras()
{
	// Generate uniformly distributed angles for 5 cameras
	const auto cameraAngles = generateUniformAngles(5);

	m_cameras = generateCameras(cameraAngles, 90.f, true);

	selectCameraClicked(0);
}

void MainWindow::setupPhysicalCameras()
{
	// Arbitrary distance from eye to image plane in the 3D representation
	const float focalLength = 0.5f;
	// Physical cameras in the viewer
	for (unsigned int i = 0; i < m_cameraImages.size(); i++)
	{
		auto physicalCamera = std::make_unique<PhysicalCamera>(2, m_cameras[i], focalLength, convertToQtImage(m_cameraImages[i]));
		m_ui.viewerWidget->addObject(std::move(physicalCamera));
	}
}

void MainWindow::setupPot()
{
	// Pot
	QMatrix4x4 cylinderWorldMatrix;
	cylinderWorldMatrix.translate(0.0, 5.5, -0.60534);
	cylinderWorldMatrix.scale(0.12, 0.12, 0.26);
	auto cylinder = std::make_unique<Cylinder>(1, cylinderWorldMatrix);
	m_ui.viewerWidget->addObject(std::move(cylinder));
}

void MainWindow::runVoxelCarving()
{
	// 3D reconstruction
	VoxelCarver carver(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);

	carver.clearCameras();
	for (unsigned int i = 0; i < m_cameraImages.size(); i++)
	{
		carver.addCameraImage(m_cameras[i], m_cameraImages[i], 0, 0);
	}
	carver.setMaximumRadiusAroundVoxel(m_maximumRadiusAroundVoxel);

	// Process the voxel grid and save it
	carver.process();
	// Extract the major connected component if needed
	auto grid = (m_extractMajorComponent) ? extractMajorConnectedComponent(carver.voxelGrid()) : carver.voxelGrid();

	// Compute and display the reprojection error
	qInfo() << "Reprojection error: " << carver.reprojectionError(grid);
	
	// grid.saveAsPgm3d("C:\\Code\\voxels.pgm3d");

	// Export voxels and import them if needed, to save computation time
	// grid.exportVoxels("C:\\Code\\voxels.txt");
	// grid.importVoxels("C:\\Code\\voxels.txt");

	// Save the voxel plant as OBJ
	// grid.saveAsOBJ("C:\\Code\\voxels.obj");

	// Voxel grid
	QMatrix4x4 voxelGridMatrix;
	voxelGridMatrix.translate(0.0, 0.0, 0.0);
	auto voxelGridObject = std::make_unique<VoxelObject>(1, voxelGridMatrix, grid);
	m_ui.viewerWidget->addObject(std::move(voxelGridObject));
	m_ui.viewerWidget->sortObjects();
}

void MainWindow::changeParameters()
{
	m_useTopImage = m_ui.actionUse_top_image->isChecked();
	m_extractMajorComponent = m_ui.actionExtract_major_component->isChecked();
}

void MainWindow::processSkeletonAndSetupVoxelGrid()
{
	const std::string folder = "4-9-18_Schnable_49-324-JS247-395_2018-04-11_01-43-36_9973100";

	SvmRbfSkeletonBranchClassifier classifier;
	classifier.load("Models\\svm.yml");
	
	VoxelGrid carvingGrid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
	carvingGrid.importVoxels("E:\\CIS\\reconstructed\\" + folder + "\\voxels.txt");
	carvingGrid.saveVoxelsAsOBJ("C:\\Users\\gaill\\Desktop\\skeletons\\voxels.obj");
	
	VoxelGrid skeletonGrid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
	skeletonGrid.importVoxels("E:\\CIS\\skeletons\\" + folder + "\\skeleton.txt");

	// Find endpoints in the skeleton
	// Sorting voxels is mandatory to accelerate algorithms
	skeletonGrid.sortVoxels();
	auto endpoints = extractSkeletonEndpoints(skeletonGrid);

	// Find the lowest endpoint, which is most probably the root of the plant
	// TODO: Add the constraint that the endpoint is next to the pot
	const auto rootVoxel = findAndRemoveLowestEndpoint(endpoints);
	generateGridFromSkeleton(skeletonGrid, std::vector<std::vector<Voxel>>(1, endpoints)).saveVoxelsAsOBJ("C:\\Users\\gaill\\Desktop\\skeletons\\endpoints.obj");
	std::vector<std::vector<Voxel>> skeletonPaths;
	std::vector<int> voxelPrecedence;
	std::tie(skeletonPaths, voxelPrecedence) = shortestPathsFromSkeleton(skeletonGrid, rootVoxel, endpoints);	
	std::vector<std::vector<Voxel>> filteredSkeletonPaths;
	std::vector<bool> skeletonPathSelected;
	std::tie(filteredSkeletonPaths, skeletonPathSelected) = filterShortestPathsClassifier(skeletonPaths, classifier);
	const auto segmentedSkeletonPaths = segmentShortestPaths(filteredSkeletonPaths);
	const auto voxelSegmentation = assignVoxelsToNearestPath(carvingGrid, segmentedSkeletonPaths);

	exportSegmentedLeaves(carvingGrid, segmentedSkeletonPaths.size(), voxelSegmentation, "C:\\Users\\gaill\\Desktop\\skeletons");
	// Save each leaf and the trunk (last leaf) separately
	// exportSegmentedSkeleton(skeletonGrid, segmentedSkeletonPaths, "C:\\Code");

	// Init the trunk spline
	// auto sorghum = initializeSorghumProceduralSkeleton(carvingGrid, segmentedSkeletonPaths);
	// auto sorghum = fitSorghumProceduralSkeleton(carvingGrid, segmentedSkeletonPaths);
	// sorghum.exportSkeleton("C:\\Code\\skeleton_procedural.txt");
	// sorghum.importSkeleton("C:\\Code\\skeleton_procedural.txt");
	// sorghum.saveAsOBJ("C:\\Code\\sorghum_", 32);

	// Display the splines in the viewer
	// auto splineObject = std::make_unique<BezierSplineObject>(1, QMatrix4x4(), sorghum.leavesAndTrunkAsSplines());
	// m_ui.viewerWidget->addObject(std::move(splineObject));

	// Check the topology of the 
	if (checkPathHasNoJunction(segmentedSkeletonPaths.back(), skeletonGrid, voxelPrecedence))
	{
		qInfo() << "Correct topology";
	}
	else
	{
		qInfo() << "Wrong topology: two hierarchies of branching";
	}
	
	// skeletonGrid.saveVoxelsAsOBJ("C:\\Users\\gaill\\Desktop\\skeletons\\rawSkeleton.obj");
	//generateGridFromSkeleton(skeletonGrid, std::vector<std::vector<Voxel>>(1, endpoints)).saveVoxelsAsOBJ("C:\\Users\\gaill\\Desktop\\skeletons\\final_endpoints.obj");
	generateGridFromSkeleton(skeletonGrid, skeletonPaths).saveVoxelsAsOBJ("C:\\Users\\gaill\\Desktop\\skeletons\\skeleton_paths.obj");
	exportPaths(skeletonPaths, "C:\\Users\\gaill\\Desktop\\skeletons\\" + folder + ".path.txt");
	writeToFile("C:\\Users\\gaill\\Desktop\\skeletons\\" + folder + ".label.txt", skeletonPathSelected);
	exportSegmentedSkeleton(skeletonGrid, skeletonPaths, "C:\\Users\\gaill\\Desktop\\skeletons");

	// Display the skeleton as a voxel grid
	skeletonGrid = generateGridFromSkeleton(skeletonGrid, segmentedSkeletonPaths);
	skeletonGrid.saveVoxelsAsOBJ("C:\\Users\\gaill\\Desktop\\skeletons\\optimSkeleton.obj");

	// Voxel grid
	QMatrix4x4 voxelGridMatrix;
	voxelGridMatrix.translate(0.0, 0.0, 0.0);
	auto voxelGridObject = std::make_unique<VoxelObject>(1, voxelGridMatrix, skeletonGrid);
	m_ui.viewerWidget->addObject(std::move(voxelGridObject));

	/*
	// Fit Bezier splines on leaves of the plant
	std::vector<BezierSpline> splines;
	for (int branchIndex = 0; branchIndex < segmentedSkeletonPaths.size() - 1; branchIndex++)
	{
		const auto branchIndices = indicesOfVoxels(voxelSegmentation, branchIndex);
		splines.push_back(fitBezierSpline(carvingGrid, branchIndices, segmentedSkeletonPaths[branchIndex]));
	}

	// Display leaves without optimizing for the width
	for (int branchIndex = 0; branchIndex < splines.size(); branchIndex++)
	{
		// spline.saveAsOBJ("C:\\Code\\localSpline.obj", 32);
		// spline.saveControlPointsAsOBJ("C:\\Code\\localSplineControlPoints.obj");

		const QVector3D up(0.0, 0.0, 1.0);
		// const auto branchIndices = indicesOfVoxels(voxelSegmentation, branchIndex);
		// const auto leaf = fitLeaf(carvingGrid, branchIndices, splines[branchIndex], up);
		
		const float startWidth = 0.01f;
		const float middleWidth = 0.02f;
		const SorghumLeaf leaf(splines[branchIndex], up, startWidth, middleWidth);
		
		std::array<std::array<BezierSurface, 2>, 2> surface;
		surface[0][0] = leaf.bottomLeftPatch();
		surface[1][0] = leaf.topLeftPatch();
		surface[0][1] = leaf.bottomRightPatch();
		surface[1][1] = leaf.topRightPatch();

		// surface[0][0].saveAsOBJ("C:\\Code\\leaf_surface_00.obj", 32, 32);
		// surface[1][0].saveAsOBJ("C:\\Code\\leaf_surface_10.obj", 32, 32);
		// surface[0][1].saveAsOBJ("C:\\Code\\leaf_surface_01.obj", 32, 32);
		// surface[1][1].saveAsOBJ("C:\\Code\\leaf_surface_11.obj", 32, 32);

		for (const auto& rowSurface : surface)
		{
			for (const auto& s : rowSurface)
			{
				auto surfaceObject = std::make_unique<BezierSurfaceObject>(1, voxelGridMatrix, s);
				m_ui.viewerWidget->addObject(std::move(surfaceObject));
			}
		}
	}
	*/
}

void MainWindow::showVoxelizedMesh()
{
	// Read mesh
	OBJReader reader;
	reader.load("C:\\Code\\maize_mesh_1.obj");

	// Display mesh
	QMatrix4x4 meshWorldMatrix;
	meshWorldMatrix.translate(0.0, 5.5, -0.5);
	meshWorldMatrix.scale(0.4, 0.4, 0.4);
	// auto mesh = std::make_unique<MeshObject>(1, meshWorldMatrix, reader.vertices(), reader.faces());
	// m_ui.viewerWidget->addObject(std::move(mesh));

	// Voxelize mesh
	VoxelGrid meshGrid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
	voxelizeMesh(meshGrid, meshWorldMatrix, reader.vertices(), reader.faces());
	fillCavitiesInGrid(meshGrid);
	meshGrid.sortVoxels();
	meshGrid = extractMajorConnectedComponent(meshGrid);
	// meshGrid.saveVoxelsAsOBJ("C:\\Code\\mesh_voxels.obj", false);
	// meshGrid.exportVoxels("C:\\Code\\mesh_voxels.txt");

	// Display voxels
	QMatrix4x4 voxelGridMatrix;
	voxelGridMatrix.translate(0.0, 0.0, 0.0);
	auto voxelGridObject = std::make_unique<VoxelObject>(1, voxelGridMatrix, meshGrid);
	m_ui.viewerWidget->addObject(std::move(voxelGridObject));
}

void MainWindow::benchmarkNumberCameras()
{
	const int width = 2454;
	const int height = 2056;

	// Position of the plant in world
	QMatrix4x4 meshWorldMatrix;
	meshWorldMatrix.translate(0.0, 5.5, -0.5);
	meshWorldMatrix.scale(0.4, 0.4, 0.4);

	// List of meshes on which to run the experiment
	const std::vector<std::string> meshFiles = {
		"Meshes/maize_mesh_61.obj",
		"Meshes/maize_mesh_97.obj",
		"Meshes/maize_mesh_240.obj",
		"Meshes/maize_mesh_332.obj",
		"Meshes/maize_mesh_468.obj",
		"Meshes/maize_mesh_472.obj",
		"Meshes/maize_mesh_913.obj",
		"Meshes/maize_mesh_985.obj",
		"Meshes/maize_mesh_1004.obj",
		"Meshes/maize_mesh_1285.obj"
	};

	struct CameraSetup
	{
		int cameraNumber;
		bool includePolarCamera;
		bool includeTopCamera;

		CameraSetup(int cameraNumber, bool includePolarCamera, bool includeTopCamera) :
			cameraNumber(cameraNumber),
			includePolarCamera(includePolarCamera),
			includeTopCamera(includeTopCamera)
		{
			
		}
	};

	std::vector<CameraSetup> cameraSetups = {
		// Side camera only
		{3, false, false},
		{5, false, false},
		{7, false, false},
		{9, false, false},
		{11, false, false},
		{13, false, false},
		{15, false, false},
		// Side + top camera
		{3, false, true},
		{5, false, true},
		{7, false, true},
		{9, false, true},
		{11, false, true},
		{13, false, true},
		{15, false, true},
		// Side + polar camera
		{3, true, false},
		{5, true, false},
		{7, true, false},
		{9, true, false},
		{11, true, false},
		{13, true, false},
		{15, true, false},
		// Side + polar + top camera
		{3, true, true},
		{5, true, true},
		{7, true, true},
		{9, true, true},
		{11, true, true},
		{13, true, true},
		{15, true, true},
	};
	
	// For each camera setup, and for each mesh, the precision and recall
	std::vector<std::vector<PrecisionRecall>> results(cameraSetups.size());

	for (const auto& meshFile: meshFiles)
	{
		qInfo() << "Loading mesh " << QString::fromStdString(meshFile);
		
		// Load the plant mesh
		OBJReader reader;
		reader.load(meshFile);

		// Generate a reference grid for this mesh
		VoxelGrid meshGrid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
		voxelizeMesh(meshGrid, meshWorldMatrix, reader.vertices(), reader.faces());
		fillCavitiesInGrid(meshGrid);
		meshGrid.sortVoxels();
		meshGrid = extractMajorConnectedComponent(meshGrid);

		for (unsigned int c = 0; c < cameraSetups.size(); c++)
		{
			qInfo() << "Number of cameras = " << cameraSetups[c].cameraNumber;

			// Generate uniformly distributed angles for cameras
			const auto cameraAngles = generateUniformAngles(cameraSetups[c].cameraNumber);
			auto cameras = generateCameras(cameraAngles, 90.f, cameraSetups[c].includeTopCamera);

			if (cameraSetups[c].includePolarCamera)
			{
				const auto polarCameras = generateCameras(cameraAngles, 45.f, false);
				cameras.insert(cameras.end(), polarCameras.begin(), polarCameras.end());
			}

			const auto result = carveAndComputePrecisionRecall(
				meshGrid,
				meshWorldMatrix,
				reader.vertices(),
				reader.faces(),
				width,
				height,
				m_objectBoundingBox,
				m_resolution,
				cameras,
				0,
				m_extractMajorComponent
			);
			
			results[c].push_back(result);
		}
	}

	// For each camera setup, compute the average precision and recall
	for (unsigned int i = 0; i < results.size(); i++)
	{
		const auto stats = computeMeanStdPrecisionRecall(results[i]);

		qInfo() << fixed << qSetRealNumberPrecision(4)
		        << "Number of cameras = " << cameraSetups[i].cameraNumber
				<< " with polar camera = " << cameraSetups[i].includePolarCamera
				<< " with top camera = " << cameraSetups[i].includeTopCamera
		        << "\tPrecision = " << stats.meanPrecision << "(+- " << stats.stdPrecision << ")"
			    << "\tRecall = " << stats.meanRecall << "(+- " << stats.stdRecall << ")"
			    << "\tF-measure = " << stats.meanFmeasure << "(+- " << stats.stdFmeasure << ")";
	}
}

void MainWindow::benchmarkCameraOffset()
{
	const int width = 2454;
	const int height = 2056;
	const int cameraNumber = 5;
	const bool includeTopCamera = true;

	// Position of the plant in world
	QMatrix4x4 meshWorldMatrix;
	meshWorldMatrix.translate(0.0, 5.5, -0.5);
	meshWorldMatrix.scale(0.4, 0.4, 0.4);

	// List of meshes on which to run the experiment
	const std::vector<std::string> meshFiles = {
		"Meshes/maize_mesh_61.obj",
		"Meshes/maize_mesh_97.obj",
		"Meshes/maize_mesh_240.obj",
		"Meshes/maize_mesh_332.obj",
		"Meshes/maize_mesh_468.obj",
		"Meshes/maize_mesh_472.obj",
		"Meshes/maize_mesh_913.obj",
		"Meshes/maize_mesh_985.obj",
		"Meshes/maize_mesh_1004.obj",
		"Meshes/maize_mesh_1285.obj"
	};

	// Offset on the width axis in pixels
	const std::vector<int> offsets = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	std::vector<std::vector<PrecisionRecall>> results(offsets.size());

	for (const auto& meshFile : meshFiles)
	{
		qInfo() << "Loading mesh " << QString::fromStdString(meshFile);

		// Load the plant mesh
		OBJReader reader;
		reader.load(meshFile);

		// Generate a reference grid for this mesh
		VoxelGrid meshGrid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
		voxelizeMesh(meshGrid, meshWorldMatrix, reader.vertices(), reader.faces());
		fillCavitiesInGrid(meshGrid);
		meshGrid.sortVoxels();
		meshGrid = extractMajorConnectedComponent(meshGrid);

		for (unsigned int o = 0; o < offsets.size(); o++)
		{
			// Generate uniformly distributed cameras with top camera
			const auto cameraAngles = generateUniformAngles(cameraNumber);
			auto cameras = generateCameras(cameraAngles, 90.f, includeTopCamera);

			const auto result = carveAndComputePrecisionRecall(
				meshGrid,
				meshWorldMatrix,
				reader.vertices(),
				reader.faces(),
				width,
				height,
				m_objectBoundingBox,
				m_resolution,
				cameras,
				offsets[o],
				m_extractMajorComponent
			);

			results[o].push_back(result);
		}
	}

	// For each offset, compute the average precision and recall
	for (unsigned int i = 0; i < results.size(); i++)
	{
		const auto stats = computeMeanStdPrecisionRecall(results[i]);

		qInfo() << fixed << qSetRealNumberPrecision(4)
			    << "Offset = " << offsets[i]
			    << "\tPrecision = " << stats.meanPrecision << "(+- " << stats.stdPrecision << ")"
			    << "\tRecall = " << stats.meanRecall << "(+- " << stats.stdRecall << ")"
			    << "\tF-measure = " << stats.meanFmeasure << "(+- " << stats.stdFmeasure << ")";
	}
}

void MainWindow::benchmarkMetrics()
{
	const int width = 2454;
	const int height = 2056;
	const int cameraNumber = 5;
	const bool includeTopCamera = true;

	// Position of the plant in world
	QMatrix4x4 meshWorldMatrix;
	meshWorldMatrix.translate(0.0, 5.5, -0.5);
	meshWorldMatrix.scale(0.4, 0.4, 0.4);

	// List of meshes on which to run the experiment
	const std::vector<std::string> meshFiles = {
		"Meshes/maize_mesh_61.obj",
		"Meshes/maize_mesh_97.obj",
		"Meshes/maize_mesh_240.obj",
		"Meshes/maize_mesh_332.obj",
		"Meshes/maize_mesh_468.obj",
		"Meshes/maize_mesh_472.obj",
		"Meshes/maize_mesh_913.obj",
		"Meshes/maize_mesh_985.obj",
		"Meshes/maize_mesh_1004.obj",
		"Meshes/maize_mesh_1285.obj"
	};

	std::vector<std::pair<int, int>> resultNumberVoxels;
	std::vector<std::pair<float, float>> resultDensity;
	std::vector<std::pair<float, float>> resultDirectionality;
	std::vector<std::pair<int, int>> resultNumberSurfaceVoxels;
	std::vector<std::pair<float, float>> resultHeight;

	for (const auto& meshFile : meshFiles)
	{
		qInfo() << "Loading mesh " << QString::fromStdString(meshFile);

		// Load the plant mesh
		OBJReader reader;
		reader.load(meshFile);

		// Generate a reference grid for this mesh
		VoxelGrid meshGrid(m_objectBoundingBox, m_resolution, m_resolution, m_resolution);
		voxelizeMesh(meshGrid, meshWorldMatrix, reader.vertices(), reader.faces());
		fillCavitiesInGrid(meshGrid);
		meshGrid.sortVoxels();
		meshGrid = extractMajorConnectedComponent(meshGrid);

		// Generate uniformly distributed cameras with top camera
		const auto cameraAngles = generateUniformAngles(cameraNumber);
		auto cameras = generateCameras(cameraAngles, 90.f, includeTopCamera);
		
		const auto carvedGrid = carve(
			meshWorldMatrix,
			reader.vertices(),
			reader.faces(),
			width,
			height,
			m_objectBoundingBox,
			m_resolution,
			cameras,
			0,
			m_extractMajorComponent
		);

		// Measure both reference and carved grids

		// Number of voxels
		resultNumberVoxels.emplace_back(
			meshGrid.voxels().size(),
			carvedGrid.voxels().size()
		);

		// Density
		resultDensity.emplace_back(
			boundingCylinderVolume(meshGrid),
			boundingCylinderVolume(carvedGrid)
		);

		// Directionality
		resultDirectionality.emplace_back(
			computeDirectionality(meshGrid),
			computeDirectionality(carvedGrid)
		);

		// Surface voxels
		resultNumberSurfaceVoxels.emplace_back(
			countNumberSurfaceVoxels(meshGrid),
			countNumberSurfaceVoxels(carvedGrid)
		);

		// Height
		resultHeight.emplace_back(
			computeHeight(meshGrid),
			computeHeight(carvedGrid)
		);
	}

	qInfo() << "Number of voxels";
	for (const auto& result : resultNumberVoxels)
	{
		qInfo() << result.first << result.second;
	}

	qInfo() << "Density";
	for (const auto& result : resultDensity)
	{
		qInfo() << fixed << qSetRealNumberPrecision(4)
			    << result.first << result.second;
	}

	qInfo() << "Directionality";
	for (const auto& result : resultDirectionality)
	{
		qInfo() << fixed << qSetRealNumberPrecision(4)
			    << result.first << result.second;
	}

	qInfo() << "Number of surface voxels";
	for (const auto& result : resultNumberSurfaceVoxels)
	{
		qInfo() << result.first << result.second;
	}

	qInfo() << "Height";
	for (const auto& result : resultHeight)
	{
		qInfo() << fixed << qSetRealNumberPrecision(4)
			    << result.first << result.second;
	}
}

VoxelGrid MainWindow::carve(
	const QMatrix4x4& worldMatrix,
    const std::vector<QVector3D>& vertices,
    const std::vector<std::tuple<int, int, int>>& faces,
    float width,
    float height,
    const AABB& objectBoundingBox,
    int resolution,
    const std::vector<Camera>& cameras,
    int sideCamerasOffset,
    bool extractMajorComponent)
{
	// Render images
	std::vector<cv::Mat> renderedImages(cameras.size());
	#pragma omp parallel for
	for (int i = 0; i < renderedImages.size(); i++)
	{
		renderedImages[i] = convertToCvMat(cameras[i].render(worldMatrix, vertices, faces, width, height));
	}

	// Setup the voxel carver for this plant
	VoxelCarver carver(objectBoundingBox, resolution, resolution, resolution);
	carver.clearCameras();
	for (unsigned int i = 0; i < renderedImages.size(); i++)
	{
		const auto offsetX = (i < renderedImages.size() - 1) ? sideCamerasOffset : 0;
		carver.addCameraImage(cameras[i], renderedImages[i], offsetX, 0);
	}
	carver.setMaximumRadiusAroundVoxel(std::max({
		carver.voxelGrid().voxelSizeX() / 4.0f,
		carver.voxelGrid().voxelSizeY() / 4.0f,
		carver.voxelGrid().voxelSizeZ() / 4.0f
	}));

	// Process the voxel grid and save it
	carver.process();

	return (extractMajorComponent) ? extractMajorConnectedComponent(carver.voxelGrid()) : carver.voxelGrid();
}

PrecisionRecall MainWindow::carveAndComputePrecisionRecall(
	const VoxelGrid& reference,
	const QMatrix4x4& worldMatrix,
	const std::vector<QVector3D>& vertices,
	const std::vector<std::tuple<int, int, int>>& faces,
	float width,
	float height,
	const AABB& objectBoundingBox,
	int resolution,
	const std::vector<Camera>& cameras,
	int sideCamerasOffset,
	bool extractMajorComponent)
{
	const auto grid = carve(worldMatrix,
	                        vertices,
	                        faces,
	                        width,
	                        height,
	                        objectBoundingBox,
	                        resolution,
	                        cameras,
	                        sideCamerasOffset,
	                        extractMajorComponent);

	// Compute statistics with regards to the reference grid
	return computePrecisionRecall(grid, reference);
}
