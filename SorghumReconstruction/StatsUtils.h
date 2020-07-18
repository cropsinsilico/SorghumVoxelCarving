#pragma once

#include <vector>

struct PrecisionRecall
{
	float precision;
	float recall;

	PrecisionRecall() : precision(0.0f), recall (0.0f) {}

	PrecisionRecall(float precision, float recall) : precision(precision), recall(recall) {}
};

struct Statistics
{
	float meanPrecision;
	float meanRecall;
	float meanFmeasure;
	
	float stdPrecision;
	float stdRecall;
	float stdFmeasure;

	Statistics() :
		meanPrecision(0.0f), meanRecall(0.0f), meanFmeasure(0.0f),
		stdPrecision(0.0f), stdRecall(0.0f), stdFmeasure(0.0f) {}
};

/**
 * \brief Compute the F-measure associated to a precision and a recall
 * \tparam T A real type (float or double)
 * \param precision A precision value
 * \param recall A recall value
 * \return The F-measure
 */
template<typename T>
T fmeasure(const T& precision, const T& recall)
{
	return 2.0 * (precision * recall) / (precision + recall);
}

/**
 * \brief Compute the fmeasure based on a PrecisionRecall struct
 * \param stat A struct containing precision and recall
 * \return The fmeasure
 */
float fmeasure(const PrecisionRecall& stat);

/**
 * \brief Compute the mean and standard deviation of a list of precision and recall values
 * \param statistics A list of precision and recall values
 * \return The mean and standard deviation of values in the list
 */
Statistics computeMeanStdPrecisionRecall(const std::vector<PrecisionRecall>& statistics);
