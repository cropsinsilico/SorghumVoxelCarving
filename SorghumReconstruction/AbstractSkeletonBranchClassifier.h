#pragma once

#include <vector>

#include "VoxelGrid.h"

class AbstractSkeletonBranchClassifier
{
public:
	/**
	 * \brief A pair of two paths in a skeleton, labeled for training
	 */
	struct SkeletonPathPair
	{
		std::vector<Voxel> longerPath;
		std::vector<Voxel> shorterPath;
		bool keep;
		std::vector<float> features;

		SkeletonPathPair(const std::vector<Voxel>& longerPath, const std::vector<Voxel>& shorterPath, bool keep);
	};

	/**
	 * \brief A list of annotated skeleton paths
	 */
	using AnnotatedPath = std::tuple<std::vector<std::vector<Voxel>>, std::vector<bool>>;

	AbstractSkeletonBranchClassifier() = default;

	virtual ~AbstractSkeletonBranchClassifier() = default;
	
	/**
	 * \brief Read a file containing skeleton paths and associated ground-truth labels.
	 *        Warning: paths are reversed so they are ordered by increased length
	 * \param pathFile File containing the paths
	 * \param labelFile File containing the labels 
	 * \return A tuple with paths and labels
	 */
	static AnnotatedPath
	readPathAndLabelFiles(const std::string& pathFile, const std::string& labelFile);

	/**
	 * \brief List all the path and labels files in a folder
	 * \param folder The folder in which files are
	 * \return A list of path and label names
	 */
	static std::vector<std::string> listPathFilesInFolder(const std::string& folder);

	/**
	 * \brief Read a data set of skeleton paths and associated ground-truth labels from a folder
	 * \param folder The folder in which files are
	 * \return A list of skeleton paths with associated labels
	 */
	static std::vector<AnnotatedPath> readPathAndLabelFilesFromFolder(const std::string& folder);

	/**
	 * \brief Split a data set into a training and a testing part
	 * \param pathsAndLabels A list of skeleton paths with associated labels
	 * \param ratio Ratio of training samples between 0.0 and 1.0
	 * \return A pair: training and testing set
	 */
	static std::pair<std::vector<AnnotatedPath>, std::vector<AnnotatedPath>>
	splitPathAndLabels(const std::vector<AnnotatedPath>& pathsAndLabels, float ratio);

	/**
	 * \brief Read a data set of skeleton paths and associated ground-truth labels from a folder
	 *        Output all path pairs from all files
	 * \param folder The folder in which files are
	 * \return A list of annotated skeleton path pairs
	 */
	static std::vector<SkeletonPathPair> readPathPairsFromFolder(const std::string& folder);

	/**
	 * \brief Compute features for choosing whether to keep or discard the shorterPath
	 *        - proportion of the shorter path in common with the longer path
	 * \param longerPath The longer of the two paths
	 * \param shorterPath The shorter of the two paths 
	 * \return A vector of features describing the two paths
	 */
	static std::vector<float> computePathPairFeatures(const std::vector<Voxel>& longerPath,
		                                              const std::vector<Voxel>& shorterPath);
	
    /**
	 * \brief Generate pairs of paths with annotation for training from ground truth paths and labels
	 * \param paths A list of skeleton paths, sorted from shortest to longest
	 * \param labels A list of label for each path: true we keep, false we discard
	 * \return A list of skeleton pairs for training
	 */
	 static std::vector<SkeletonPathPair> generatePathPairsFromGroundTruth(
		 const std::vector<std::vector<Voxel>>& paths,
         const std::vector<bool>& labels);

	/**
	  * \brief Generate pairs of paths with annotation for training from ground truth paths and labels
	  * \param pathsAndLabels A list of annotated skeleton paths, sorted from shortest to longest
	  * \return A list of skeleton pairs for training
	  */
	 static std::vector<SkeletonPathPair> generatePathPairsFromGroundTruth(
		 const std::vector<AnnotatedPath>& pathsAndLabels);

	 /**
	  * \brief Evaluate the classifier using whole plants
	  *	      Compute the precision recall of individual branches
	  * \param pathsAnsLabels A list of labeled skeleton paths
	  * \return A pair: precision, recall
	  */
	 std::pair<float, float> evaluateFilter(
		 const std::vector<AnnotatedPath>& pathsAnsLabels) const;

	 /**
	  * \brief Train a classifier using a data set of path pairs
	  * \param pathPairs A list of labeled skeleton paths
	  * \param ratio Ratio between training and validation set
	  */
	 virtual void train(const std::vector<SkeletonPathPair>& pathPairs, float ratio = 1.0f) = 0;

	 /**
	  * \brief Evaluate the classifier using a data set of path pairs
	  * \param pathPairs A list of labeled skeleton path pairs
	  * \return The percentage of miss-classified examples
	  */
	 virtual float evaluateClassifier(const std::vector<SkeletonPathPair>& pathPairs) const = 0;

	 /**
	  * \brief Save the current classifier in a YML file
	  * \param filename Filename in which to save the current classifier
	  */
	 virtual void save(const std::string& filename) const = 0;

	 /**
	  * \brief Load a classifier from a YML file
	  * \param filename Filename in which the classifier is saved
	  * \return True if successfully loaded.
	  */
	 virtual bool load(const std::string& filename) = 0;

	 /**
	  * \brief Predict whether to keep or discard a branch
	  * \param longerPath The longer branch, which is already selected in the skeleton
	  * \param shorterPath The shorter branch, for which we need to make a decision
	  * \return True: keep the shorter branch, false: discard the shorter branch
	  */
	 virtual bool predict(const std::vector<Voxel>& longerPath, const std::vector<Voxel>& shorterPath) const = 0;
};
