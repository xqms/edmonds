// Edmond's cardinality matching algorithm
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#ifndef EDMOND_H
#define EDMOND_H

#include <queue>

#include "graph.h"
#include "union_find.h"

class EdmondsCardinalityMatching
{
public:
	/**
	 * Calculate a maximum matching in graph @a input and return it.
	 *
	 * Runtime: O(n^3), where n is the number of vertices.
	 **/
	void calculateMatching(const Graph& input);

	void writeMatching(Graph& output);
	void writeMatchingDIMAC(std::ostream& stream);
private:
	//! Type of vertices in our graph: inner/outer/out-of-tree.
	enum VertexType
	{
		INNER,
		OUTER,
		OUT_OF_FOREST
	};

	//! Determine the vertex type of @a v (O(1))
	VertexType vertexType(NodeID v) const;

	//! Check if @a v is outer vertex (even distance from root in contracted graph)
	bool isOuterVertex(NodeID v) const;

	//! Check if @a v is inner vertex (odd distance from root in contracted graph)
	bool isInnerVertex(NodeID v) const;

	//! Check if @a v is outside of our forest (=> matched)
	bool isOutOfForest(NodeID v) const;

	////////////////////////////////////////////////////////////////////////////
	// Algorithm steps

	/**
	 * Find an unscanned outer vertex (see m_scanned)
	 *
	 * @param v Output destination
	 * @return true iff an unscanned outer vertex was found
	 **/
	bool findUnscannedOuterVertex(NodeID* v);

	/**
	 * Search for an outer vertex or an out-of-forest vertex @a y adjacent
	 * to @a x.
	 *
	 * @param y Output for the node ID
	 * @param type Output for the vertex type
	 **/
	bool neighborSearch(NodeID x, NodeID* y, VertexType* type) const;

	void removeVertexFromTree(NodeID v);

	/**
	 * Augment the matching along the path created by the union of
	 * Px and Py (and the edge between Px.front() and Py.front()).
	 **/
	void augment(const std::vector<NodeID>& Px, const std::vector<NodeID>& Py);

	/**
	 * Follow the path @a path up to rIdx and make m_phi consistent with an
	 * ear decomposition for the base at path[rIdx].
	 **/
	void convertPathToEar(const std::vector<NodeID>& path, unsigned int rIdx);

	/**
	 * Follow the path @a path and unite all ear decompositions with the
	 * one at base r. This modifies m_rho accordingly.
	 **/
	void uniteBasesAlongPath(const std::vector<NodeID>& path, NodeID r);

	/**
	 * Apply the SHRINK operation to the blossom formed by Px,Py.
	 **/
	void shrink(const std::vector<NodeID>& Px, const std::vector<NodeID>& Py);

	/**
	 * Iterate on outer vertex x, until we cannot find any adjacent interesting
	 * vertices anymore.
	 **/
	void step(NodeID x);

	/**
	 * Calculate the path to the root of the tree containing the outer vertex
	 * @a v.
	 **/
	std::vector<NodeID> pathToRoot(NodeID v) const;

	/**
	 * Reset the tree structure completely.
	 **/
	void reset();

	//! Our input graph
	const Graph* m_graph;

	//! mu mapping: {v,w} in matching <=> m_mu[v] == w.
	std::vector<NodeID> m_mu;

	/**
	 * phi mapping as in lecture. In particular, phi and mu are associated
	 * with an M-alternating ear-decomposition in each blossom. Furthermore,
	 * phi points towards the tree root for all inner vertices.
	 **/
	std::vector<NodeID> m_phi;

	/**
	 * Current outer vertex candidates.
	 *
	 * Instead of restarting the search in each outer iteration, we keep
	 * a queue of candidates. This saves us from re-examining non-outer
	 * vertices. Of course, we have to add vertices when they become outer
	 * vertices.
	 **/
	std::queue<NodeID> m_outerVertices;

	//! Has the vertex v been scanned completely?
	std::vector<bool> m_scanned;

	/**
	 * Also record for each vertex to which tree root it belongs.
	 **/
	std::vector<NodeID> m_tree;

	/**
	 * Keep track of the forest explicitly for fast tree deletion in augment().
	 * This array contains an array of nodes belonging to the root v for each
	 * node v in the graph.
	 **/
	std::vector<std::vector<NodeID>> m_forest;

	/**
	 * Union-Find structure for the blossom mapping rho.
	 *
	 * v and w are in the same blossom, iff they are in the same class in rho.
	 **/
	UnionFind<NodeID> m_rho;
};

#endif
