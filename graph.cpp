// Undirected graph
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#include "graph.h"

#include <assert.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

Node::Node()
{
}

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

void Graph::parseDIMACLine(const char* line)
{
	if(line[0] == 'e' && line[1] == ' ')
	{
		NodeID v, w;

		// Use strtoul to parse the node indices since sscanf is
		// surprisingly slow

		// Format: e v w
		char* endptr = 0;
		v = strtoul(line + 2, &endptr, 10);

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

		m_edges.emplace_back(v, w);
		m_loadDegrees[v]++;
		m_loadDegrees[w]++;
	}
	else if(line[0] == 'c')
	{
	}
	else if(line[0] == 0)
	{
	}
	else if(strncmp(line, "p edge ", 7) == 0)
	{
		unsigned int n, m;
		if(sscanf(line, "p edge %u %u", &n, &m) != 2)
			throw LoadError("Could not parse DIMAC header");

		reset(n);
		m_edges.reserve(m);
		m_loadDegrees.clear();
		m_loadDegrees.resize(n, 0);
	}
	else
	{
		fprintf(stderr, "Warning: Unknown DIMAC line: '%s'\n", line);
	}
}

#if __unix__
void Graph::loadDIMACFromFD(int fd)
{
	// This function is optimized for speed. loadFromDIMAC() does exactly
	// the same using standard C++ and is probably more readable.

	// If possible, tell the Linux kernel that we are accessing the input file
	// in a sequential fashion
#if _XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L
	posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

	char buf[16*1024];
	std::string incompleteLine;

	while(1)
	{
		ssize_t bytes = read(fd, buf, sizeof(buf));
		if(bytes == 0)
			break;
		if(bytes < 0)
		{
			perror("Could not read from input file");
			throw LoadError(strerror(errno));
		}

		char* newline = reinterpret_cast<char*>(memchr(buf, '\n', bytes));
		if(!newline)
		{
			incompleteLine += std::string(buf, bytes);
			continue;
		}

		incompleteLine += std::string(buf, newline - buf);
		parseDIMACLine(incompleteLine.c_str());
		incompleteLine.clear();

		ssize_t parsedBytes = newline - buf + 1;
		while(parsedBytes != bytes)
		{
			char* begin = newline + 1;
			newline = reinterpret_cast<char*>(memchr(begin, '\n', bytes - parsedBytes));
			if(!newline)
			{
				incompleteLine = std::string(begin, bytes - parsedBytes);
				break;
			}

			*newline = 0;

			parseDIMACLine(begin);
			parsedBytes = newline - buf + 1;
		}
	}

	parseDIMACLine(incompleteLine.c_str());
	incompleteLine.clear();

	fillAdjacencyLists();
}
#endif

void Graph::loadDIMAC(std::istream& stream)
{
	std::ios_base::sync_with_stdio(false);

	while(!stream.eof())
	{
		std::string line;
		std::getline(stream, line);

		if(line.length() == 0 || line[0] == '\n')
			continue;

		parseDIMACLine(line.c_str());
	}

	std::ios_base::sync_with_stdio(true);

	fillAdjacencyLists();
}

void Graph::toDIMAC(std::ostream& stream)
{
	std::ios_base::sync_with_stdio(false);
	stream << "p edge " << m_nodes.size() << " " << m_edges.size() << "\n";

	for(const Edge& e : m_edges)
	{
		// DIMAC is 1-based, we are 0-based
		stream << "e " << (e.first+1) << " " << (e.second+1) << "\n";
	}
	std::ios_base::sync_with_stdio(true);
}

void Graph::fillAdjacencyLists()
{
	for(std::size_t v = 0; v < m_nodes.size(); ++v)
		m_nodes[v].m_adjacent.reserve(m_loadDegrees[v]);

	for(Edge& e : m_edges)
	{
		m_nodes[e.first].m_adjacent.push_back(e.second);
		m_nodes[e.second].m_adjacent.push_back(e.first);
	}
}
