#include "UnionFind.h"

UnionFind::UnionFind(unsigned int nbNodes)
{
	m_nodes.resize(nbNodes);

	for (unsigned int i = 0; i < nbNodes; i++)
	{
		m_nodes[i].parent = i;
		m_nodes[i].rang = 0;
		m_nodes[i].children = 1;
	}
}

unsigned int UnionFind::findNode(unsigned int node)
{
	if (m_nodes[node].parent != node)
	{
		m_nodes[node].parent = findNode(m_nodes[node].parent);
	}

	return m_nodes[node].parent;
}

unsigned int UnionFind::size(unsigned int node)
{
	const auto root = findNode(node);

	return m_nodes[root].children;
}

unsigned int UnionFind::unionNodes(unsigned int nodeX, unsigned int nodeY)
{
	const auto rootX = findNode(nodeX);
	const auto rootY = findNode(nodeY);
	auto newRoot = rootX;

	if (rootX != rootY)
	{
		if (m_nodes[rootX].rang < m_nodes[rootY].rang)
		{
			m_nodes[rootX].parent = rootY;
			m_nodes[rootY].children += m_nodes[rootX].children;
			newRoot = rootY;
		}
		else if (m_nodes[rootX].rang > m_nodes[rootY].rang)
		{
			m_nodes[rootY].parent = rootX;
			m_nodes[rootX].children += m_nodes[rootY].children;
		}
		else
		{
			m_nodes[rootY].parent = rootX;
			m_nodes[rootX].rang++;
			m_nodes[rootX].children += m_nodes[rootY].children;
		}
	}

	return newRoot;
}
