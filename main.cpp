// Main function
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#include "graph.h"
#include "edmonds.h"

#include <string.h>
#include <fcntl.h>

#include <fstream>
#include <chrono>

int main(int argc, char** argv)
{
	if(argc != 2 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
	{
		fprintf(stderr, "Usage: edmonds <input DIMAC file>\n");
		return 1;
	}

	Graph graph;

	// Load the input graph. If we are running under UNIX, we can apply some
	// optimizations yielding half of the runtime on very big graphs.
	auto start = std::chrono::steady_clock::now();

#if __unix__
	int fd = open(argv[1], O_RDONLY);
	if(fd < 0)
	{
		perror("Could not open input file");
		return 1;
	}

	graph.loadDIMACFromFD(fd);
#else
	std::istream stream(argv[1]);

	if(stream.bad())
	{
		perror("Could not open input file");
		return 1;
	}

	graph.loadDIMAC(stream);
#endif

	auto end = std::chrono::steady_clock::now();

	std::cerr << "Loaded input graph with "
		<< graph.numNodes() << " nodes and "
		<< graph.numEdges() << " edges in "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";

	EdmondsCardinalityMatching edmond;

	Graph matching;
	edmond.calculateMatching(graph);

	edmond.writeMatchingDIMAC(std::cout);

	return 0;
}
