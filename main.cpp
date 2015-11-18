// Main function
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#include "graph.h"
#include "edmonds.h"

#include <string.h>

#include <fstream>

int main(int argc, char** argv)
{
	if(argc != 2 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
	{
		fprintf(stderr, "Usage: edmonds <input DIMAC file>\n");
		return 1;
	}

	Graph graph;

	std::ifstream input(argv[1]);
	if(input.bad())
	{
		perror("Could not open input file");
		return 1;
	}

	graph.loadDIMAC(input);

	EdmondsCardinalityMatching edmond;

	Graph matching;
	edmond.calculateMatching(graph);

	edmond.writeMatchingDIMAC(std::cout);

	return 0;
}
