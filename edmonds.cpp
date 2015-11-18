// Edmonds' cardinality matching algorithm
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

// Hint: It's best to read this file bottom-up to go from general steps
//  to specific methods.

#include "edmonds.h"

#include <stdarg.h>
#include <assert.h>

#include <algorithm>
#include <chrono>

////////////////////////////////////////////////////////////////////////////////
// VERTEX TYPE

bool EdmondsCardinalityMatching::isOuterVertex(NodeID v) const
{
	return m_mu[v] == v || m_phi[m_mu[v]] != m_mu[v];
}

bool EdmondsCardinalityMatching::isInnerVertex(NodeID v) const
{
	return m_phi[m_mu[v]] == m_mu[v] && m_phi[v] != v;
}

bool EdmondsCardinalityMatching::isOutOfForest(NodeID v) const
{
	return m_mu[v] != v && m_phi[v] == v && m_phi[m_mu[v]] == m_mu[v];
}

EdmondsCardinalityMatching::VertexType EdmondsCardinalityMatching::vertexType(NodeID v) const
{
	if(isOuterVertex(v))
		return OUTER;
	else if(isInnerVertex(v))
		return INNER;
	else
	{
		assert(isOutOfForest(v));
		return OUT_OF_FOREST;
	}
}

////////////////////////////////////////////////////////////////////////////////

void EdmondsCardinalityMatching::reset()
{
	m_rho.reset(m_graph->numNodes());

	// Empty the outer vertex candidate queue
	while(!m_outerVertices.empty())
		m_outerVertices.pop();

	for(NodeID v = 0; v < m_graph->numNodes(); ++v)
	{
		m_phi[v] = v;
		m_tree[v] = v;
		m_forest[v].clear();
		m_scanned[v] = false;

		if(isOuterVertex(v))
			m_outerVertices.push(v);
	}
}

bool EdmondsCardinalityMatching::findUnscannedOuterVertex(NodeID* dest)
{
	// Pop elements from the candidate queue until we find one which
	// is an unscanned outer vertex.
	do
	{
		if(m_outerVertices.empty())
			return false;

		*dest = m_outerVertices.front();
		m_outerVertices.pop();
	}
	while(m_scanned[*dest] || !isOuterVertex(*dest));

	return true;
}

bool EdmondsCardinalityMatching::neighborSearch(NodeID x, NodeID* y, VertexType* type) const
{
	const Node& nx = m_graph->node(x);
	NodeID xRho = m_rho.find(x);

	for(NodeID w : nx.adjacent())
	{
		VertexType t = vertexType(w);
		if(t == OUT_OF_FOREST || (t == OUTER && m_rho.find(w) != xRho))
		{
			*y = w;
			*type = t;
			return true;
		}
	}

	return false;
}

std::vector<NodeID> EdmondsCardinalityMatching::pathToRoot(NodeID v) const
{
	assert(isOuterVertex(v));

	std::vector<NodeID> ret;
	ret.push_back(v);

	// Just follow the mu,phi mappings and construct the path until
	// we hit an outer vertex with m_mu[v] == v.
	while(v != m_mu[v])
	{
		v = m_mu[v];
		ret.push_back(v);

		v = m_phi[v];
		ret.push_back(v);
	}

	return ret;
}

void EdmondsCardinalityMatching::removeVertexFromTree(NodeID v)
{
	m_phi[v] = v;
	m_tree[v] = v;

	m_rho.fastDisconnectElement(v);

	// If this vertex is unmatched, it is now an outer vertex and
	// might be interesting for the outer vertex search
	// (if it is matched, it is now out-of-forest)
	if(m_mu[v] == v)
	{
		m_outerVertices.push(v);
		m_scanned[v] = false;
	}

	// All adjacent outer vertices need to be reconsidered as their type
	// of neighbor has changed.
	for(NodeID w : m_graph->node(v).adjacent())
	{
		// If m_scanned[w] == false, this vertex is still in the queue
		if(m_scanned[w])
		{
			m_outerVertices.push(w);
			m_scanned[w] = false;
		}
	}
}

void EdmondsCardinalityMatching::augment(const std::vector<NodeID>& Px, const std::vector<NodeID>& Py)
{
	NodeID x = Px.front();
	NodeID y = Py.front();

	// change matching by M-alternating path Px, Py
	for(unsigned int i = 1; i < Px.size(); i += 2)
	{
		NodeID v = Px[i];
		m_mu[m_phi[v]] = v;
		m_mu[v] = m_phi[v];
	}
	for(unsigned int i = 1; i < Py.size(); i += 2)
	{
		NodeID v = Py[i];
		m_mu[m_phi[v]] = v;
		m_mu[v] = m_phi[v];
	}

	// add edge {x,y} to matching
	m_mu[x] = y;
	m_mu[y] = x;

	// reset phi, rho, scanned in the affected trees
	NodeID rx = Px.back(); // root of x tree
	NodeID ry = Py.back(); // root of y tree

	// reset the root rx
	removeVertexFromTree(rx);

	// ... and all its descendants
	for(NodeID v : m_forest[rx])
		removeVertexFromTree(v);
	m_forest[rx].clear();

	// reset the root ry
	removeVertexFromTree(ry);

	// ... and all its descendants
	for(NodeID v : m_forest[ry])
		removeVertexFromTree(v);
	m_forest[ry].clear();
}

void EdmondsCardinalityMatching::convertPathToEar(const std::vector<NodeID>& P, unsigned int rIdx)
{
	// Search backwards in the path until we exit the blossom at r
	int i = P.size() - rIdx - 2;
	for(; i > 0; i -= 2)
	{
		NodeID v = P[i];
		if(m_rho.isRepresentant(v))
			break;
	}

	if(i < 0)
		return;

	// We are at an inner node v, which is it's own representant.
	// This means we exited the blossom belonging to base r.
	// Go one inner node further.
	m_outerVertices.push(P[i]);
	i -= 2;
	for(; i > 0; i -= 2)
	{
		NodeID v = P[i];

		// Modify the phi pointer of our phi neighbor (outer vertex) to point
		// back at us.
		m_phi[m_phi[v]] = v;

		// Old inner vertices become outer vertices in the blossom, so consider
		// them during the next outer vertex search
		m_outerVertices.push(v);
	}
}

void EdmondsCardinalityMatching::uniteBasesAlongPath(const std::vector<NodeID>& P, NodeID r)
{
	NodeID v = P.front();
	while(v != r)
	{
		assert(isOuterVertex(v));
		assert(v != m_phi[m_mu[v]]);

		if(m_rho.isRepresentant(v))
		{
			// Outer nodes, which are their own representants, are bases of
			// blossoms (maybe one-vertex-blossoms)
			// => Unite the rho class with rho(r).
			m_rho.unite(r, v);

			// Our matching partner is surely not part of a blossom
			// => it is its own representant
			// => also merge it
			m_rho.unite(r, m_mu[v]);
		}

		v = m_phi[m_mu[v]];
	}
}

void EdmondsCardinalityMatching::shrink(const std::vector<NodeID>& Px, const std::vector<NodeID>& Py)
{
	// SHRINK
	NodeID x = Px.front();
	NodeID y = Py.front();

	// Find first vertex r in intersection of P(x) and P(y) which has
	// m_rho[r] == r (is the base of a blossom or inner vertex).
	// We do this by scanning the paths backwards.

	NodeID r = 0;
	int rIdx = -1;

	for(unsigned int i = 0; i < std::min(Px.size(), Py.size()); ++i)
	{
		NodeID nx = Px[Px.size()-1-i];
		NodeID ny = Py[Py.size()-1-i];

		// If the paths disagree, we exit the intersection, so stop here.
		if(nx != ny)
			break;

		if(m_rho.isRepresentant(nx))
		{
			r = nx;
			rIdx = i;
		}
	}
	assert(rIdx >= 0);

	// Fix the phi mapping to convert the path to an ear with base r
	convertPathToEar(Px, rIdx);
	convertPathToEar(Py, rIdx);

	// Close phi over {x,y}
	if(m_rho.find(x) != r)
		m_phi[x] = y;

	if(m_rho.find(y) != r)
		m_phi[y] = x;

	// Unite all rho classes we encounter along the way (include all ear
	// decompositions our paths runs through into the new ear decomposition)
	uniteBasesAlongPath(Px, r);
	uniteBasesAlongPath(Py, r);
}

void EdmondsCardinalityMatching::step(NodeID x)
{
	// As long as the tree is not reset and x is not exhausted, we
	// can operate on x.
	while(1)
	{
		assert(isOuterVertex(x) && !m_scanned[x]);

		// Find a neighbor of x which is either out-of-tree
		// or outer and part of different tree
		NodeID y;
		VertexType yType;

		if(!neighborSearch(x, &y, &yType))
		{
			m_scanned[x] = true;
			return;
		}

		if(yType == OUT_OF_FOREST)
		{
			// Grow
			m_phi[y] = x;

			// Mark the two nodes as belonging to the current tree
			m_tree[y] = m_tree[x];
			m_tree[m_mu[y]] = m_tree[x];

			m_forest[m_tree[x]].push_back(y);
			m_forest[m_tree[x]].push_back(m_mu[y]);

			// We got a new outer vertex
			m_outerVertices.push(m_mu[y]);

			continue;
		}

		// Calculate P(x) and P(y)
		std::vector<NodeID> Px = pathToRoot(x);
		std::vector<NodeID> Py = pathToRoot(y);

		// P(x) and P(y) are not vertex-disjoint iff they have the same
		// "tail" ending in the shared tree root.
		if(Px.back() != Py.back())
		{
			// The paths end in different trees -> AUGMENT along Px,Py
			augment(Px, Py);

			// After the augment the current tree is destroyed
			// -> exit the current iteration and continue with the outer scan
			return;
		}
		else
		{
			// The paths end in the same tree -> SHRINK the blossom
			shrink(Px, Py);
		}
	}
}

void EdmondsCardinalityMatching::calculateMatching(
	const Graph& input, Graph& matching
)
{
	// Setup pointer for other member methods
	m_graph = &input;

	// Setup mu, phi, rho pointers and reset m_scanned
	m_mu.resize(input.numNodes());
	m_phi.resize(input.numNodes());
	m_rho.reset(input.numNodes());
	m_scanned.resize(input.numNodes());
	m_tree.resize(input.numNodes());
	m_forest.resize(input.numNodes());

	// Some basic profiling
	auto start = std::chrono::steady_clock::now();

	// Initialize empty matching
	std::vector<NodeID> sorting(input.numNodes());

	for(NodeID v = 0; v < input.numNodes(); ++v)
	{
		m_mu[v] = v;
		sorting[v] = v;
	}

	// Sort the graph by vertex degree. This makes the initial greedy matching
	// much more effective.
	std::sort(sorting.begin(), sorting.end(), [&](NodeID v, NodeID w) {
		return input.node(v).adjacent().size() < input.node(w).adjacent().size();
	});

	// Start the algorithm with a greedy matching (takes O(m))
	for(NodeID i = 0; i < input.numNodes(); ++i)
	{
		NodeID v = sorting[i];

		if(m_mu[v] != v)
			continue;

		const Node& nv = m_graph->node(v);
		for(NodeID w : nv.adjacent())
		{
			if(m_mu[w] == w)
			{
				m_mu[w] = v;
				m_mu[v] = w;
				break;
			}
		}
	}

	auto afterInitialMatching = std::chrono::steady_clock::now();
	std::cerr << "Initial matching took " << std::chrono::duration_cast<std::chrono::milliseconds>(afterInitialMatching - start).count() << "ms\n";

	// Reset the forest pointers and init the outer vertex queue
	reset();

	// While there is an unscanned outer vertex x, call step(x)
	NodeID x;
	while(findUnscannedOuterVertex(&x))
	{
		step(x);
	}

	auto afterComputation = std::chrono::steady_clock::now();
	std::cerr << "Matching computation took " << std::chrono::duration_cast<std::chrono::milliseconds>(afterComputation - afterInitialMatching).count() << "ms\n";

	// Recover matching from m_mu
	matching.reset(m_graph->numNodes());

	std::fill(m_scanned.begin(), m_scanned.end(), false);
	for(NodeID v = 0; v < m_graph->numNodes(); ++v)
	{
		if(!m_scanned[v] && m_mu[v] != v)
		{
			matching.addEdge(v, m_mu[v]);
			m_scanned[m_mu[v]] = true;
		}
	}
}
