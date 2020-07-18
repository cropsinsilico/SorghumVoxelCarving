#pragma once

#include <string>
#include <vector>

bool writeToFile(const std::string& filename, float number);

bool writeToFile(const std::string& filename, bool boolean);

bool writeToFile(const std::string& filename, const std::vector<bool>& booleans);

std::vector<bool> readBooleanArrayFromFile(const std::string& filename);
