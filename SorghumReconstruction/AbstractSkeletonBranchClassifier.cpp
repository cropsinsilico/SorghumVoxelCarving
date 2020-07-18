#include "AbstractSkeletonBranchClassifier.h"

#include <cmath>

#include "IoUtils.h"
#include "Skeletons.h"

AbstractSkeletonBranchClassifier::SkeletonPathPair::SkeletonPathPair(
	const std::vector<Voxel>& longerPath,
	const std::vector<Voxel>& shorterPath,
	bool keep) :
	longerPath(longerPath),
	shorterPath(shorterPath),
	keep(keep),
	features(computePathPairFeatures(longerPath, shorterPath))
{
	
}

AbstractSkeletonBranchClassifier::AnnotatedPath AbstractSkeletonBranchClassifier::readPathAndLabelFiles(
	const std::string& pathFile,
	const std::string& labelFile)
{
	// Read paths and labels
	auto paths = importPaths(pathFile);
	auto labels = readBooleanArrayFromFile(labelFile);

	// Reverse paths
	std::reverse(paths.begin(), paths.end());
	std::reverse(labels.begin(), labels.end());

	return {paths, labels};
}

std::vector<std::string> AbstractSkeletonBranchClassifier::listPathFilesInFolder(const std::string& folder)
{
	std::vector<std::string> files;

	const QDir directory(QString::fromStdString(folder));

	// Check if directory exists
	if (directory.exists())
	{
		const auto fileList = directory.entryInfoList(QStringList() << "*.path.txt",
			QDir::Files,
			QDir::SortFlag::Name);

		for (const auto& file : fileList)
		{
			// Read parameters in txt files
			const auto labelsFile = file.baseName() + ".label.txt";
			if (directory.exists(labelsFile))
			{
				files.push_back(file.baseName().toStdString());
			}
		}
	}

	return files;
}

std::vector<AbstractSkeletonBranchClassifier::AnnotatedPath>
AbstractSkeletonBranchClassifier::readPathAndLabelFilesFromFolder(const std::string& folder)
{
	std::vector<AnnotatedPath> files;

	const QDir directory(QString::fromStdString(folder));

	// Check if directory exists
	if (directory.exists())
	{
		const auto fileList = directory.entryInfoList(QStringList() << "*.path.txt",
			                                          QDir::Files,
			                                          QDir::SortFlag::Name);

		for (const auto& file : fileList)
		{
			// Read parameters in txt files
			const auto labelsFile = file.baseName() + ".label.txt";
			if (directory.exists(labelsFile))
			{
				std::vector<std::vector<Voxel>> paths;
				std::vector<bool> labels;

				// Read data from files
				std::tie(paths, labels) = readPathAndLabelFiles(
					file.filePath().toStdString(),
					directory.filePath(labelsFile).toStdString());

				// Add the pairs from the current files to the list of all files
				files.emplace_back(paths, labels);
			}
		}
	}

	return files;
}

std::pair<std::vector<AbstractSkeletonBranchClassifier::AnnotatedPath>, std::vector<AbstractSkeletonBranchClassifier::AnnotatedPath>>
AbstractSkeletonBranchClassifier::splitPathAndLabels(const std::vector<AnnotatedPath>& pathsAndLabels, float ratio)
{
	std::vector<AnnotatedPath> training;
	std::vector<AnnotatedPath> testing;
	
	const auto numberTraining = int(std::round(pathsAndLabels.size() * ratio));
	const auto numberTesting = pathsAndLabels.size() - numberTraining;

	training.reserve(numberTraining);
	testing.reserve(numberTesting);

	for (const auto& pathsAndLabel : pathsAndLabels)
	{
		if (int(training.size()) < numberTraining)
		{
			training.push_back(pathsAndLabel);
		}
		else
		{
			testing.push_back(pathsAndLabel);
		}
	}

	return { training, testing };
}

std::vector<AbstractSkeletonBranchClassifier::SkeletonPathPair>
AbstractSkeletonBranchClassifier::readPathPairsFromFolder(const std::string& folder)
{
	const auto pathsAnsLabels = readPathAndLabelFilesFromFolder(folder);
	
	return generatePathPairsFromGroundTruth(pathsAnsLabels);
}

std::vector<float> AbstractSkeletonBranchClassifier::computePathPairFeatures(
	const std::vector<Voxel>& longerPath,
	const std::vector<Voxel>& shorterPath)
{
	std::vector<float> features;
	features.reserve(6);

	const auto numberCommonVoxels = numberCommonVoxelsInPaths(longerPath, shorterPath);

	// Absolute length of the longer path
	// features.push_back(float(longerPath.size()));
	
	// Absolute length of the shorter path
	// features.push_back(float(shorterPath.size()));

	// Height of the branching point
	// features.push_back(float(shorterPath[numberCommonVoxels].z) / 512.0f);
	
	// Height of the tip voxel
	// features.push_back(float(shorterPath.back().z) / 512.0f);

	// Proportion of the longer path that is shared with the shorter path
	// features.push_back(float(numberCommonVoxels) / float(longerPath.size()));
	
	// Proportion of the shorter path that is shared with the longer path
	features.push_back(float(numberCommonVoxels) / float(shorterPath.size()));
	
	// Absolute length of the shorter path that is not shared with the longer path
	features.push_back(float(shorterPath.size()) - float(numberCommonVoxels));
	
	// Absolute length of the shorter path that is shared with the longer path
	features.push_back(float(numberCommonVoxels));

	return features;
}

std::vector<AbstractSkeletonBranchClassifier::SkeletonPathPair> AbstractSkeletonBranchClassifier::generatePathPairsFromGroundTruth(
	const std::vector<std::vector<Voxel>>& paths,
	const std::vector<bool>& labels)
{
	std::vector<SkeletonPathPair> pathPairs;

	// Find relevant pairs of paths for training
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		// If the path is selected
		if (labels[i])
		{
			// Output all paths that are selected and longer than this path
			for (unsigned int j = i + 1; j < paths.size(); j++)
			{
				if (labels[j])
				{
					// We found a pair: path j, should not discard path i
					pathPairs.emplace_back(paths[j], paths[i], true);
				}
			}
		}
		else // The path is discarded
		{
			unsigned int mostCommonPath = i;
			int mostNumberCommonVoxels = 0;

			// We find the shortest selected path with the maximum of common voxels with this path
			for (unsigned int j = i + 1; j < paths.size(); j++)
			{
				if (labels[j])
				{
					// Compute the number of common voxels in the two paths
					const auto numberCommonVoxels = numberCommonVoxelsInPaths(paths[j], paths[i]);

					// If it is more than the maximum
					if (numberCommonVoxels > mostNumberCommonVoxels)
					{
						mostNumberCommonVoxels = numberCommonVoxels;
						mostCommonPath = j;
					}
				}
			}

			if (mostCommonPath > i)
			{
				// We found a pair: path mostCommonPath, should discard path i
				pathPairs.emplace_back(paths[mostCommonPath], paths[i], false);
			}
		}
	}

	return pathPairs;
}

std::vector<AbstractSkeletonBranchClassifier::SkeletonPathPair>
AbstractSkeletonBranchClassifier::generatePathPairsFromGroundTruth(
	const std::vector<AnnotatedPath>& pathsAndLabels)
{
	std::vector<SkeletonPathPair> pathPairs;

	for (const auto& file : pathsAndLabels)
	{
		// Generate pairs of paths
		const auto currentPairs = generatePathPairsFromGroundTruth(std::get<0>(file), std::get<1>(file));

		// Add the pairs from the current files to the list of all files
		pathPairs.insert(pathPairs.end(), currentPairs.begin(), currentPairs.end());
	}

	return pathPairs;
}

std::pair<float, float> AbstractSkeletonBranchClassifier::evaluateFilter(
	const std::vector<AnnotatedPath>& pathsAnsLabels) const
{
	long long truePositive = 0;
	long long falsePositive = 0;
	long long falseNegative = 0;
	long long trueNegative = 0;

	for (const auto& sample : pathsAnsLabels)
	{
		// Copy paths and labels
		auto paths = std::get<0>(sample);
		auto labels = std::get<1>(sample);

		// Sort by increasing length
		std::reverse(paths.begin(), paths.end());
		std::reverse(labels.begin(), labels.end());

		// Run filtering on paths
		std::vector<std::vector<Voxel>> filteredPaths;
		std::vector<bool> pathSelected;
		std::tie(filteredPaths, pathSelected) = filterShortestPathsClassifier(paths, *this);

		assert(labels.size() == pathSelected.size());

		// Check that labels are correct
		for (unsigned int i = 0; i < labels.size(); i++)
		{
			if (pathSelected[i] && labels[i])
			{
				truePositive++;
			}
			else if (pathSelected[i] && !labels[i])
			{
				falsePositive++;
			}
			else if (!pathSelected[i] && labels[i])
			{
				falseNegative++;
			}
			else if (!pathSelected[i] && !labels[i])
			{
				trueNegative++;
			}
		}
	}

	qInfo() << "truePositive = " << truePositive;
	qInfo() << "falsePositive = " << falsePositive;
	qInfo() << "falseNegative = " << falseNegative;
	qInfo() << "trueNegative = " << trueNegative;

	const auto precision = float(truePositive) / (float(truePositive) + float(falsePositive));
	const auto recall = float(truePositive) / (float(truePositive) + float(falseNegative));

	return { precision, recall };
}
