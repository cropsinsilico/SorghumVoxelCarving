#include "OBJReader.h"

#include <algorithm>
#include <fstream>

bool OBJReader::load(const std::string& filename)
{
	// Read file
	std::ifstream file(filename);

	if (!file.is_open())
	{
		return false;
	}

	// Clear the existing object
	clear();

	std::string command;

	// For each voxel compute the coordinates in 3D space
	while (file >> command)
	{
		// If the line is a vertex
		if (command.size() == 1 && command.front() == 'v')
		{
			// Coordinates of the vertices in 3D space
			float x, y, z;
			file >> x >> y >> z;

			m_vertices.emplace_back(x, y, z);
		}
		// If the line is a face
		else if (command.size() == 1 && command.front() == 'f')
		{
			// Indices of the face
			int i, j, k;
			file >> i >> j >> k;

			m_faces.emplace_back(i - 1, j - 1, k - 1);
		}
	}

	file.close();

	return true;
}

void OBJReader::clear()
{
    m_vertices.clear();
    m_faces.clear();
}

const std::vector<QVector3D>& OBJReader::vertices() const
{
	return m_vertices;
}

const std::vector<std::tuple<int, int, int>>& OBJReader::faces() const
{
	return m_faces;
}
