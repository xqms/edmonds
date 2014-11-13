// Undirected graph
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#include "graph.h"

#include <assert.h>
#include <string.h>

Graph::Graph()
 : m_nodeCount(0)
{
}

void Graph::reset(unsigned int numNodes)
{
	m_nodes.clear();
	m_nodes.resize(numNodes);
	m_nodeCount = numNodes;
	m_edges.clear();
}

NodeID Graph::addNode()
{
	m_nodes.push_back(Node());
	return m_nodeCount++;
}

void Graph::addEdge(NodeID v, NodeID w)
{
	assert(v < m_nodes.size());
	assert(w < m_nodes.size());

	m_nodes[v].m_adjacent.push_back(w);
	m_nodes[w].m_adjacent.push_back(v);
	m_edges.emplace_back(v, w);
}

void Graph::loadDIMAC(std::istream& stream)
{
	bool initialized = false;

	while(!stream.eof())
	{
		std::string line;
		std::getline(stream, line);

		if(line.length() == 0 || line[0] == '\n')
			continue;

		if(line.substr(0, 7) == "p edge ")
		{
			if(initialized)
				throw LoadError("Found more than one DIMAC header (p ...)");

			unsigned int n, m;
			if(sscanf(line.c_str(), "p edge %u %u", &n, &m) != 2)
				throw LoadError("Could not parse DIMAC header");

			reset(n);
			initialized = true;
		}
		else if(line[0] == 'e' && line[1] == ' ')
		{
			NodeID v, w;

			// Use strtoul to parse the node indices since sscanf is
			// surprisingly slow

			// Format: e v w
			char* endptr = 0;
			v = strtoul(line.data() + 2, &endptr, 10);

			if(*endptr != ' ')
				throw LoadError("Invalid edge specification");

			// Skip whitespace between v and w
			while(*endptr == ' ')
				endptr++;

			// Parse w
			w = strtoul(endptr, &endptr, 10);
			if(*endptr != 0 && *endptr != ' ')
				throw LoadError("Invalid edge specification");

			// Sanity check
			if(v == 0 || w == 0)
				throw LoadError("Zero node indices in edge spec");

			// DIMAC is 1-based, we are 0-based
			v -= 1;
			w -= 1;

			if(v >= numNodes() || w >= numNodes())
				throw LoadError("Node indices out of bounds in edge spec");

			addEdge(v, w);
		}
		else if(line[0] == 'c')
		{
		}
		else
		{
			fprintf(stderr, "Warning: Unknown DIMAC line: '%s'\n", line.c_str());
		}
	}
}

void Graph::toDIMAC(std::ostream& stream)
{
	stream << "p edge " << m_nodes.size() << " " << m_edges.size() << "\n";

	for(const Edge& e : m_edges)
	{
		// DIMAC is 1-based, we are 0-based
		stream << "e " << (e.first+1) << " " << (e.second+1) << "\n";
	}
}
