#pragma once

#include <QtWidgets/QMainWindow>
#include <opencv2/core/core.hpp>

#include "ui_MainWindow.h"
#include "VoxelGrid.h"
#include "StatsUtils.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);

public slots:
	void setupScene();

	void openImages();

	void openPlant();

	void runVoxelCarving();

	void changeParameters();

	void selectCameraClicked(int camera);
	
	void calibrateSideImage();

	void calibrateTopImage();

	void interpolationSliderChanged();
	
private:
	void setupUi();

	void loadCameraImages(const QString& directory);
	void setupCameras();
	void setupPhysicalCameras();
	void setupPot();
	void processSkeletonAndSetupVoxelGrid();
	void showVoxelizedMesh();
	void benchmarkNumberCameras();
	void benchmarkCameraOffset();
	void benchmarkMetrics();
	
	static VoxelGrid carve(const QMatrix4x4& worldMatrix,
	                       const std::vector<QVector3D>& vertices,
	                       const std::vector<std::tuple<int, int, int>>& faces,
	                       float width,
	                       float height,
	                       const AABB& objectBoundingBox,
	                       int resolution,
	                       const std::vector<Camera>& cameras,
	                       int sideCamerasOffset,
	                       bool extractMajorComponent);

	static PrecisionRecall carveAndComputePrecisionRecall(
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
		bool extractMajorComponent
	);
	
	Ui::MainWindowClass m_ui;

	std::string m_pathToImages;

	AABB m_objectBoundingBox;
	int m_resolution;

	bool m_useTopImage;
	bool m_needsCalibration;
	bool m_extractMajorComponent;

	float m_maximumRadiusAroundVoxel;

	std::vector<Camera> m_cameras;
	std::vector<cv::Mat> m_cameraImages;
};
