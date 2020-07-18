#include "IoUtils.h"

#include <fstream>

bool writeToFile(const std::string& filename, float number)
{
	std::ofstream file(filename, std::fstream::out);

	if (!file.is_open())
	{
		return false;
	}

	file << number << std::endl;
	
	file.close();

	return true;
}

bool writeToFile(const std::string& filename, bool boolean)
{
	std::ofstream file(filename, std::fstream::out);

	if (!file.is_open())
	{
		return false;
	}

	if (boolean)
	{
		file << "true" << std::endl;
	}
	else
	{
		file << "false" << std::endl;
	}

	file.close();

	return true;
}

bool writeToFile(const std::string& filename, const std::vector<bool>& booleans)
{
	std::ofstream file(filename, std::fstream::out);

	if (!file.is_open())
	{
		return false;
	}

	for (const auto value: booleans)
	{
		if (value)
		{
			file << "1" << std::endl;
		}
		else
		{
			file << "0" << std::endl;
		}
	}

	file.close();

	return true;
}

std::vector<bool> readBooleanArrayFromFile(const std::string& filename)
{
	std::vector<bool> booleans;
	
	// Read file
	std::ifstream file(filename);

	if (file.is_open())
	{
		int number;

		// For each voxel compute the coordinates in 3D space
		while (file >> number)
		{
			// O means false
			if (number == 0)
			{
				booleans.push_back(false);
			}
			// 1 means true
			else if (number == 1)
			{
				booleans.push_back(true);
			}
		}
	}

	file.close();

	return booleans;
}
