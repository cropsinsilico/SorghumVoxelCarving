#include "CvSkeletonBranchClassifier.h"

void CvSkeletonBranchClassifier::train(const std::vector<SkeletonPathPair>& pathPairs, float ratio)
{
	const auto trainData = convertToTrainData(pathPairs);

	if (ratio > 0.0f && ratio < 1.0f)
		trainData->setTrainTestSplitRatio(ratio, true);

	// Create and train the model
	m_model = createModel();
	trainModel(trainData);

	if (ratio > 0.0f && ratio < 1.0f)
	{
		cv::Mat results;
		qInfo() << "Percentage of misclassified branches in training set: "
			<< m_model->calcError(trainData, false, results);
		qInfo() << "Percentage of misclassified branches in validation set: "
			<< m_model->calcError(trainData, true, results);
	}
}

float CvSkeletonBranchClassifier::evaluateClassifier(const std::vector<SkeletonPathPair>& pathPairs) const
{
	const auto trainData = convertToTrainData(pathPairs);

	// By default 100% error
	float error = 100.0f;

	if (m_model)
	{
		// Evaluate the model on the training data
		cv::Mat results;
		error = m_model->calcError(trainData, false, results);
	}

	return error;
}

void CvSkeletonBranchClassifier::save(const std::string& filename) const
{
	if (m_model)
	{
		// Save the model
		m_model->save(filename);
	}
}

bool CvSkeletonBranchClassifier::load(const std::string& filename)
{
	m_model = cv::ml::SVM::load(filename);

	return !m_model.empty();
}

bool CvSkeletonBranchClassifier::predict(
	const std::vector<Voxel>& longerPath,
	const std::vector<Voxel>& shorterPath) const
{
	if (m_model)
	{
		// Compute features for classifying the path pair
		auto features = computePathPairFeatures(longerPath, shorterPath);
		features = selectFeatures(features);

		// Convert to the right format for OpenCV
		cv::Mat samplesMat(1, features.size(), CV_32F);

		for (int j = 0; j < features.size(); j++)
		{
			samplesMat.at<float>(0, j) = features[j];
		}

		// Run classifier: 1 => keep, -1 => discard
		cv::Mat results;
		m_model->predict(samplesMat, results);
		const auto response = results.at<float>(0, 0);

		if (response > 0.0)
		{
			return true;
		}
	}

	return false;
}

cv::Ptr<cv::ml::TrainData> CvSkeletonBranchClassifier::convertToTrainData(
	const std::vector<SkeletonPathPair>& pathPairs) const
{
	const auto firstFeatures = selectFeatures(pathPairs.front().features);
	
	cv::Mat trainingDataMat(pathPairs.size(), firstFeatures.size(), CV_32F);

	int positiveSamples = 0;
	int negativeSamples = 0;

	// Fill the training data and labels
	for (int i = 0; i < pathPairs.size(); i++)
	{
		const auto features = selectFeatures(pathPairs[i].features);
		
		for (int j = 0; j < features.size(); j++)
		{
			trainingDataMat.at<float>(i, j) = features[j];
		}

		// Label is 1 if we keep the shortest in pair, 0 if we discard the shortest in pair
		if (pathPairs[i].keep)
		{
			positiveSamples++;
		}
		else
		{
			negativeSamples++;
		}
	}

	// Change weights because classes are unbalanced
	cv::Mat sampleWeights(pathPairs.size(), 1, CV_32F);
	for (int i = 0; i < pathPairs.size(); i++)
	{
		if (pathPairs[i].keep)
		{
			sampleWeights.at<float>(i, 0) = 1.0f;
		}
		else
		{
			sampleWeights.at<float>(i, 0) = float(positiveSamples) / float(negativeSamples);
		}
	}

	const auto labelsMat = generateLabels(pathPairs);

	// Construct training data from samples
	return cv::ml::TrainData::create(trainingDataMat,
		cv::ml::ROW_SAMPLE,
		labelsMat,
		cv::noArray(),
		cv::noArray(),
		sampleWeights);
}

cv::Mat CvSkeletonBranchClassifier::generateLabels(const std::vector<SkeletonPathPair>& pathPairs) const
{
	cv::Mat labelsMat(pathPairs.size(), 1, CV_32SC1);

	for (int i = 0; i < pathPairs.size(); i++)
	{
		// Label is 1 if we keep the shortest in pair, 0 if we discard the shortest in pair
		if (pathPairs[i].keep)
		{
			labelsMat.at<int>(i, 0) = 1;
		}
		else
		{
			labelsMat.at<int>(i, 0) = -1;
		}
	}

	return labelsMat;
}

void CvSkeletonBranchClassifier::trainModel(const cv::Ptr<cv::ml::TrainData>& data) const
{
	m_model->train(data);
}

std::vector<float> SvmRbfSkeletonBranchClassifier::selectFeatures(const std::vector<float>& features) const
{
	std::vector<float> selectFeatures;
	selectFeatures.reserve(3);

	// Proportion of the shorter path that is shared with the longer path
	selectFeatures.push_back(features[0]);

	// Absolute length of the shorter path that is not shared with the longer path
	selectFeatures.push_back(features[1]);

	// Absolute length of the shorter path that is shared with the longer path
	selectFeatures.push_back(features[2]);

	return selectFeatures;
}

cv::Ptr<cv::ml::StatModel> SvmRbfSkeletonBranchClassifier::createModel() const
{
	auto svm = cv::ml::SVM::create();
	svm->setType(cv::ml::SVM::C_SVC);
	svm->setKernel(cv::ml::SVM::RBF);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));

	return svm;
}

void SvmRbfSkeletonBranchClassifier::trainModel(const cv::Ptr<cv::ml::TrainData>& data) const
{
	// Cast to cv::Ptr<cv::ml::SVM> to use the specific train function
	auto svm = m_model.dynamicCast<cv::ml::SVM>();

	if (svm)
	{
		svm->trainAuto(data);
	}
}

std::vector<float> SvmLinearSkeletonBranchClassifier::selectFeatures(const std::vector<float>& features) const
{
	std::vector<float> selectFeatures;
	selectFeatures.reserve(1);

	// Proportion of the shorter path that is shared with the longer path
	selectFeatures.push_back(features[0]);

	return selectFeatures;
}

cv::Ptr<cv::ml::StatModel> SvmLinearSkeletonBranchClassifier::createModel() const
{
	auto svm = cv::ml::SVM::create();
	svm->setType(cv::ml::SVM::C_SVC);
	svm->setKernel(cv::ml::SVM::LINEAR);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));

	return svm;
}

void SvmLinearSkeletonBranchClassifier::trainModel(const cv::Ptr<cv::ml::TrainData>& data) const
{
	// Cast to cv::Ptr<cv::ml::SVM> to use the specific train function
	auto svm = m_model.dynamicCast<cv::ml::SVM>();

	if (svm)
	{
		svm->trainAuto(data);
	}
}

cv::Mat MlpSkeletonBranchClassifier::generateLabels(const std::vector<SkeletonPathPair>& pathPairs) const
{
	cv::Mat labelsMat(pathPairs.size(), 1, CV_32F);

	for (int i = 0; i < pathPairs.size(); i++)
	{
		// Label is 1 if we keep the shortest in pair, 0 if we discard the shortest in pair
		if (pathPairs[i].keep)
		{
			labelsMat.at<float>(i, 0) = 1.0f;
		}
		else
		{
			labelsMat.at<float>(i, 0) = -1.0f;
		}
	}

	return labelsMat;
}

std::vector<float> MlpSkeletonBranchClassifier::selectFeatures(const std::vector<float>& features) const
{
	return features;
}

cv::Ptr<cv::ml::StatModel> MlpSkeletonBranchClassifier::createModel() const
{
	auto mlp = cv::ml::ANN_MLP::create();

	cv::Mat layersSize = cv::Mat(4, 1, CV_16U);
	layersSize.row(0) = cv::Scalar(3);
	layersSize.row(1) = cv::Scalar(8);
	layersSize.row(2) = cv::Scalar(8);
	layersSize.row(3) = cv::Scalar(1);
	mlp->setLayerSizes(layersSize);

	mlp->setActivationFunction(cv::ml::ANN_MLP::ActivationFunctions::SIGMOID_SYM, 0.0, 0.0);

	mlp->setTermCriteria(cv::TermCriteria(
		cv::TermCriteria::Type::COUNT + cv::TermCriteria::Type::EPS,
		100,
		1e-4
	));

	mlp->setTrainMethod(cv::ml::ANN_MLP::TrainingMethods::RPROP);

	return mlp;
}

void MlpSkeletonBranchClassifier::trainModel(const cv::Ptr<cv::ml::TrainData>& data) const
{
	m_model->train(data, cv::ml::ANN_MLP::TrainFlags::NO_OUTPUT_SCALE);
}
