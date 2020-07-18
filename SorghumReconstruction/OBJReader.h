#pragma once

#include <vector>

#include <QVector3D>


/**
 * \brief Simple OBJ reader, only works for vertices and faces
 */
class OBJReader
{
public:
	OBJReader() = default;

	
	/**
	 * \brief Read an OBJ file. Only load vertices and faces.
	 * \param filename The path to the file
	 * \return True if successfully load
	 */
	bool load(const std::string& filename);
	
	/**
	 * \brief Clear the current OBJ mesh
	 */
	void clear();

	/**
	 * \brief Return the vertices of the OBJ mesh
	 * \return The vertices of the OBJ mesh
	 */
	const std::vector<QVector3D>& vertices() const;

	/**
	 * \brief Return the faces of the OBJ mesh
	 * \return The faces of the OBJ mesh
	 */
	const std::vector<std::tuple<int, int, int>>& faces() const;
	
private:
	std::vector<QVector3D> m_vertices;

	std::vector<std::tuple<int, int, int>> m_faces;
};

