// Verify that a given .dmx file describes a perfect matching in G
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#include "graph.h"

#include <fstream>

#include <queue>
#include <algorithm>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/max_cardinality_matching.hpp>

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: verifier <input graph> <matching>\n");
		return 1;
	}

	Graph graph;
	std::ifstream stream(argv[1]);
	graph.loadDIMAC(stream);

	printf("Loaded graph with %u nodes and %u edges\n", graph.numNodes(), graph.numEdges());

	Graph matching;
	std::ifstream matchingStream(argv[2]);
	matching.loadDIMAC(matchingStream);

	printf("Loaded matching with %u nodes and %u edges\n", matching.numNodes(), matching.numEdges());

	// Basic cardinality checks
	if(graph.numNodes() != matching.numNodes())
	{
		fprintf(stderr, "Matching has a different number of nodes than the input graph: %u != %u\n", matching.numNodes(), graph.numNodes());
		return 1;
	}

	if(matching.numEdges() > graph.numNodes()/2)
	{
		fprintf(stderr, "Matching has more edges than possible! (%u > 2*%u)\n", matching.numNodes(), graph.numNodes());
		return 1;
	}

	// Check that matching is indeed a matching
	std::vector<bool> covered(graph.numNodes(), false);
	for(const Graph::Edge& e : matching.edges())
	{
		// Check that e is in the input graph
		const Node& n = graph.node(e.first);
		auto it = std::find(n.adjacent().begin(), n.adjacent().end(), e.second);
		if(it == n.adjacent().end())
		{
			fprintf(stderr, "The matching contains an edge %lu-%lu, which is not in the graph\n", e.first, e.second);
			return 1;
		}

		if(covered[e.first])
		{
			fprintf(stderr, "Node %lu is covered twice by the matching!\n", e.first);
			return 1;
		}
		covered[e.first] = true;

		if(covered[e.second])
		{
			fprintf(stderr, "Node %lu is covered twice by the matching!\n", e.second);
			return 1;
		}
		covered[e.second] = true;
	}

	printf("The matching is valid.\n");

	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> BGraph;
	BGraph bGraph(graph.numNodes());

	for(const Graph::Edge& e : graph.edges())
	{
		boost::add_edge(e.first, e.second, bGraph);
	}

	typedef std::vector<boost::graph_traits<BGraph>::vertex_descriptor> MateMap;
	MateMap mate(graph.numNodes());

	if(!boost::checked_edmonds_maximum_cardinality_matching(bGraph, &mate[0]))
		std::abort();

	printf("boost found matching of cardinality %lu\n", boost::matching_size(bGraph, &mate[0]));
	printf("Our cardinality is %u\n", matching.numEdges());

	if(boost::matching_size(bGraph, &mate[0]) != matching.numEdges())
		return 1;
	else
		return 0;
}
