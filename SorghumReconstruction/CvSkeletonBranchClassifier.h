#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/ml.hpp>

#include "AbstractSkeletonBranchClassifier.h"

class CvSkeletonBranchClassifier : public AbstractSkeletonBranchClassifier
{
public:
	~CvSkeletonBranchClassifier() override = default;
	
	void train(const std::vector<SkeletonPathPair>& pathPairs, float ratio = 1.0f) override;

	float evaluateClassifier(const std::vector<SkeletonPathPair>& pathPairs) const override;

	void save(const std::string& filename) const override;

	bool load(const std::string& filename) override;

	bool predict(const std::vector<Voxel>& longerPath, const std::vector<Voxel>& shorterPath) const override;

protected:
	/**
	 * \brief Convert a SkeletonPathPair data set to OpenCV matrices for training the classifier
	 * \param pathPairs A list of annotated path pairs
	 * \return A TrainData object for training the classifier
	 */
	virtual cv::Ptr<cv::ml::TrainData> convertToTrainData(const std::vector<SkeletonPathPair>& pathPairs) const;

	/**
	 * \brief Generate labels for the data set. By default, integers 1 or -1
	 * \param pathPairs A list of annotated path pairs
	 * \return The labels fot the data set
	 */
	virtual cv::Mat generateLabels(const std::vector<SkeletonPathPair>& pathPairs) const;
	
	/**
	 * \brief Select only features relevant to this classifier
	 * \param features A vector of all features
	 * \return A vector with selected features
	 */
	virtual std::vector<float> selectFeatures(const std::vector<float>& features) const = 0;

	/**
	 * \brief Create the SVM object
	 * \return The SVM object
	 */
	virtual cv::Ptr<cv::ml::StatModel> createModel() const = 0;

	/**
	 * \brief Train the model 
	 * \param data A data set
	 */
	virtual void trainModel(const cv::Ptr<cv::ml::TrainData>& data) const;
	
	cv::Ptr<cv::ml::StatModel> m_model;
};

class SvmRbfSkeletonBranchClassifier final : public CvSkeletonBranchClassifier
{
public:
	~SvmRbfSkeletonBranchClassifier() override = default;
	
protected:

	std::vector<float> selectFeatures(const std::vector<float>& features) const override;

	cv::Ptr<cv::ml::StatModel> createModel() const override;

	void trainModel(const cv::Ptr<cv::ml::TrainData>& data) const override;
};

class SvmLinearSkeletonBranchClassifier final : public CvSkeletonBranchClassifier
{
public:
	~SvmLinearSkeletonBranchClassifier() override = default;

protected:

	std::vector<float> selectFeatures(const std::vector<float>& features) const override;

	cv::Ptr<cv::ml::StatModel> createModel() const override;

	void trainModel(const cv::Ptr<cv::ml::TrainData>& data) const override;
};

class MlpSkeletonBranchClassifier final : public CvSkeletonBranchClassifier
{
public:
	~MlpSkeletonBranchClassifier() override = default;

protected:
	
	cv::Mat generateLabels(const std::vector<SkeletonPathPair>& pathPairs) const override;
	
	std::vector<float> selectFeatures(const std::vector<float>& features) const override;

	cv::Ptr<cv::ml::StatModel> createModel() const override;

	void trainModel(const cv::Ptr<cv::ml::TrainData>& data) const override;
};
