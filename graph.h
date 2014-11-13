// Undirected graph
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <iostream>

#include <stdexcept>

class Graph;

typedef std::size_t NodeID;

/**
 * Represents a node (vertex) in the graph.
 **/
class Node
{
friend Graph; // m_adjacent is modified by Graph::addEdge()
public:
	//! Return list of adjacent nodes
	const std::vector<NodeID>& adjacent() const
	{ return m_adjacent; }
private:
	std::vector<NodeID> m_adjacent;
};

class Graph
{
public:
	typedef std::pair<NodeID, NodeID> Edge;

	//! Thrown on error inside loadDIMAC()
	class LoadError : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

	Graph();

	//! Reset the graph structure and create @a numNodes unconnected nodes
	void reset(unsigned int numNodes);

	//! Add a new node and return its ID
	NodeID addNode();

	//! Add an edge connecting v and w
	void addEdge(NodeID v, NodeID w);

	/**
	 * Return the Node instance for a node ID
	 *
	 * @note NodeIDs are 0-based, so node(0) is the first node in a graph.
	 **/
	const Node& node(NodeID id) const
	{ return m_nodes[id]; }

	//! Return number of nodes in the graph
	unsigned int numNodes() const
	{ return m_nodeCount; }

	//! Return number of edges in the graph
	unsigned int numEdges() const
	{ return m_edges.size(); }

	const std::vector<Edge>& edges() const
	{ return m_edges; }

	//! Load a DIMAC graph from stream @a stream
	void loadDIMAC(std::istream& stream);

	//! Write a DIMAC graph into stream @a stream
	void toDIMAC(std::ostream& stream);
private:
	std::vector<Node> m_nodes;
	std::size_t m_nodeCount;

	std::vector<Edge> m_edges;
};

#endif
