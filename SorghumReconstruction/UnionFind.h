#pragma once

#include <vector>

using namespace std;

class UnionFind
{
public:
	UnionFind(unsigned int nbNodes);
	
	/**
	 * @brief Find Return the root of a node.
	 * @param node A node.
	 * @return The root of the node.
	 */
	unsigned int findNode(unsigned int node);

	/**
	 * @brief Size Return the size of the subset in which a node is.
	 * @param node A node.
	 * @return The size of the subset in which the node is.
	 */
	unsigned int size(unsigned int node);

	/**
	 * @brief Union Unify two nodes.
	 * @param nodeX A node X.
	 * @param nodeY A node Y.
	 * @return The new root of the two nodes.
	 */
	unsigned int unionNodes(unsigned int nodeX, unsigned int nodeY);

private:
	struct Node
	{
		unsigned int parent;
		unsigned int rang;
		unsigned int children;
	};

	vector<Node> m_nodes;
};
