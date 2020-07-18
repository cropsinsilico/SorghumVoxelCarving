#pragma once

#include "AbstractSkeletonBranchClassifier.h"

class ThresholdSkeletonBranchClassifier : public AbstractSkeletonBranchClassifier
{
public:

	explicit ThresholdSkeletonBranchClassifier(float threshold = 0.65f);
	
	~ThresholdSkeletonBranchClassifier() override = default;
	
	void train(const std::vector<SkeletonPathPair>& pathPairs, float ratio) override;
	
	float evaluateClassifier(const std::vector<SkeletonPathPair>& pathPairs) const override;
	
	void save(const std::string& filename) const override;
	
	bool load(const std::string& filename) override;
	
	bool predict(const std::vector<Voxel>& longerPath, const std::vector<Voxel>& shorterPath) const override;

private:

	float m_threshold;
};

