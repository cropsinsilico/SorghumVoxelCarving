#include "ThresholdSkeletonBranchClassifier.h"

ThresholdSkeletonBranchClassifier::ThresholdSkeletonBranchClassifier(float threshold) :
	AbstractSkeletonBranchClassifier(),
	m_threshold(threshold)
{
	
}

void ThresholdSkeletonBranchClassifier::train(const std::vector<SkeletonPathPair>& pathPairs, float ratio)
{
	// No training
}

float ThresholdSkeletonBranchClassifier::evaluateClassifier(const std::vector<SkeletonPathPair>& pathPairs) const
{
	int truePositive = 0;
	
	for (const auto& pathPair : pathPairs)
	{
		if (pathPair.features[0] <= m_threshold)
		{
			truePositive++;
		}
	}
	
	return 100.f * float(truePositive) / float(pathPairs.size());
}

void ThresholdSkeletonBranchClassifier::save(const std::string& filename) const
{
	// No save
}

bool ThresholdSkeletonBranchClassifier::load(const std::string& filename)
{
	// No load
	return true;
}

bool ThresholdSkeletonBranchClassifier::predict(
	const std::vector<Voxel>& longerPath,
	const std::vector<Voxel>& shorterPath) const
{
	// Compute features for classifying the path pair
	auto features = computePathPairFeatures(longerPath, shorterPath);

	const auto proportionSharedShorter = features[0];

	return proportionSharedShorter <= m_threshold;
}
