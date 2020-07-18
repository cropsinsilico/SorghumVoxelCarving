#pragma once

#include <vector>

#include <QVector3D>

class OBJWriter
{
public:
	OBJWriter() = default;

	/**
	 * \brief Return the vertices of the OBJ object
	 * \return The vertices of the OBJ object
	 */
	const std::vector<QVector3D>& vertices() const;

	/**
	 * \brief Return the normals of the OBJ object
	 * \return The normals of the OBJ object
	 */
	const std::vector<QVector3D>& normals() const;

	/**
	 * \brief Return the lines of the OBJ object
	 * \return The lines of the OBJ object
	 */
	const std::vector<std::pair<int, int>>& lines() const;

	/**
	 * \brief Return the faces of the OBJ object
	 * \return The faces of the OBJ object
	 */
	const std::vector<std::tuple<int, int, int>>& faces() const;

	/**
	 * \brief Save the content of the OBJ object to a file
	 * \param filename The path to the file
	 * \return True if successfully saved
	 */
	bool save(const std::string& filename) const;

	/**
	 * \brief Clear the current OBJ object
	 */
	void clear();

	/**
	 * \brief Set the vertices
	 * \param vertices The new vertices
	 */
	void setVertices(std::vector<QVector3D> vertices);

	/**
	 * \brief Set the normals
	 * \param normals The new normals
	 */
	void setNormals(std::vector<QVector3D> normals);

	/**
	 * \brief Set the lines
	 * \param lines The new lines
	 */
	void setLines(std::vector<std::pair<int, int>> lines);

	/**
	 * \brief Set the faces
	 * \param faces The new faces
	 */
	void setFaces(std::vector<std::tuple<int, int, int>> faces);
	
	/**
	 * \brief Return the index of a vertex. If not present return -1.
	 * \param vertex 3D coordinates of the vertex
	 * \return The index of the vertex in the list of vertices, or -1
	 */
	int findVertex(const QVector3D& vertex) const;

	/**
	 * \brief Add a vertex and return its position. If the vertex is already existing, simply return its index.
	 * \param vertex 3D coordinates of the vertex
	 * \return The index of the vertex in the list of vertices
	 */
	int addVertexSafe(const QVector3D& vertex);

	/**
	 * \brief Add a vertex and return its position. Does not ensure that there are no doubles
	 * \param vertex 3D coordinates of the vertex
	 * \return The index of the vertex in the list of vertices
	 */
	int addVertex(const QVector3D& vertex);

	/**
	 * \brief Add a normal and return its position. Does not ensure that there are no doubles
	 * \param normal 3D coordinates of the normal vector
	 * \return The index of the normal in the list of normals
	 */
	int addNormal(const QVector3D& normal);

	/**
	 * \brief Add a line between two vertices per index
	 * \param a Index of the first vertex
	 * \param b Index of the second vertex
	 */
	void addLine(int a, int b);
	
	/**
	 * \brief Add a line between two vertices. Create vertices if needed
	 * \param a The first vertex
	 * \param b The second vertex
	 */
	void addLine(const QVector3D& a, const QVector3D& b);

	/**
	 * \brief Add a face from 3 vertices per index
	 * \param a Index of the first vertex
	 * \param b Index of the second vertex
	 * \param c Index of the third vertex
	 */
	void addFace(int a, int b, int c);

	/**
	 * \brief Add a face from 3 vertices. Create vertices if needed
	 * \param a The first vertex
	 * \param b The second vertex
	 * \param c The third vertex
	 */
	void addFace(const QVector3D& a, const QVector3D& b, const QVector3D& c);
	
private:
	std::vector<QVector3D> m_vertices;

	std::vector<QVector3D> m_normals;

	std::vector<std::pair<int, int>> m_lines;

	std::vector<std::tuple<int, int, int>> m_faces;
};

