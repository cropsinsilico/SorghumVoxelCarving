#include "StatsUtils.h"

#include <cmath>

float fmeasure(const PrecisionRecall& stat)
{
	return 2.0f * (stat.precision * stat.recall) / (stat.precision + stat.recall);
}

Statistics computeMeanStdPrecisionRecall(const std::vector<PrecisionRecall>& statistics)
{
	Statistics aggregatedStats;
	
	const auto numberSamples = int(statistics.size());

	float sumPrecision = 0.0;
	float sumRecall = 0.0;
	float sumFmeasure = 0.0;

	// Compute the average precision and recall over all meshes
	for (const auto& stat : statistics)
	{
		sumPrecision += stat.precision;
		sumRecall += stat.recall;
		sumFmeasure += fmeasure(stat);
	}

	aggregatedStats.meanPrecision = sumPrecision / numberSamples;
	aggregatedStats.meanRecall = sumRecall / numberSamples;
	aggregatedStats.meanFmeasure = sumFmeasure / numberSamples;

	float varPrecision = 0.0f;
	float varRecall = 0.0f;
	float varFmeasure = 0.0f;

	// Compute the standard deviation over all meshes
	for (const auto& stat : statistics)
	{
		varPrecision += std::pow(stat.precision - aggregatedStats.meanPrecision, 2.0f);
		varRecall += std::pow(stat.recall - aggregatedStats.meanRecall, 2.0f);
		varFmeasure += std::pow(fmeasure(stat) - aggregatedStats.meanFmeasure, 2.0f);
	}

	// Compute sample standard deviation
	aggregatedStats.stdPrecision = std::sqrt(varPrecision / float(numberSamples - 1));
	aggregatedStats.stdRecall = std::sqrt(varRecall / float(numberSamples - 1));
	aggregatedStats.stdFmeasure = std::sqrt(varFmeasure / float(numberSamples - 1));

	return aggregatedStats;
}
