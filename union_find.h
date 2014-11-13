// Union-Find data structure
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <vector>

#include <assert.h>
#include <string.h>

template<class T>
class UnionFind
{
public:
	UnionFind();
	~UnionFind();

	/**
	 * Reset the structure to @a num classes C_i with labels [0..num-1].
	 *
	 * Runtime: O(n).
	 **/
	void reset(unsigned int num);

	/**
	 * Unite classes with representants @a a and @a b. The new class
	 * is guarenteed to have representant @a a.
	 *
	 * Runtime: O(1).
	 **/
	void unite(T a, T b);

	/**
	 * Find representant for class @a v.
	 *
	 * Runtime: O(log n) (amortized: O(alpha(n)))
	 **/
	T find(T v) const;

	/**
	 * Determine whether @a v is its own representant.
	 *
	 * Runtime: O(1).
	 **/
	bool isRepresentant(T v) const;

	/**
	 * Dissolve a class into singletons. This method has to be called
	 * for each member of the class.
	 *
	 * @warning If you do not call this method for each member, the resulting
	 *   structure is undefined.
	 *
	 * Runtime: O(1).
	 **/
	void fastDisconnectElement(T v);

	/**
	 * Dissolve a class into singletons.
	 *
	 * Runtime: O(|values|).
	 *
	 * @param values All members of the class
	 **/
	template<class Container>
	void dissolve(const Container& values);
private:
	struct Node
	{
		Node();
		~Node();

		Node* parent;       //!< pointer to parent node
		T value;            //!< value
		unsigned int depth; //!< upper bound on depth of the tree below
	};

	// Allocated memory for all nodes
	std::vector<Node> m_pool;

	// Pointer to the corresponding node for each value v
	// We use a separate pointer array for fast node swap in unite().
	std::vector<Node*> m_forest;
};

// IMPLEMENTATION

template<class T>
UnionFind<T>::Node::Node()
 : parent(0), depth(0)
{
}

template<class T>
UnionFind<T>::Node::~Node()
{
}

template<class T>
UnionFind<T>::UnionFind()
{
}

template<class T>
UnionFind<T>::~UnionFind()
{
}

template<class T>
void UnionFind<T>::reset(unsigned int num)
{
	if(num != m_pool.size())
	{
		m_pool.resize(num);
		m_forest.resize(num);
	}

	for(unsigned int i = 0; i < num; ++i)
	{
		m_pool[i].depth = 0;
		m_pool[i].parent = nullptr;
		m_pool[i].value = i;
		m_forest[i] = &m_pool[i];
	}
}

template<class T>
void UnionFind<T>::unite(T a, T b)
{
	assert(isRepresentant(a));
	assert(isRepresentant(b));
	assert(a != b);

	// Choose the larger tree as the top tree and make it the parent
	// of the smaller tree.
	if(m_forest[a]->depth > m_forest[b]->depth)
		m_forest[b]->parent = m_forest[a];
	else if(m_forest[a]->depth < m_forest[b]->depth)
	{
		m_forest[a]->parent = m_forest[b];

		// Since we guarantee that a is the representant of the class,
		// we need to swap the nodes.
		std::swap(m_forest[b]->value, m_forest[a]->value);
		std::swap(m_forest[a], m_forest[b]);
	}
	else
	{
		// Tie-break: use a as parent to avoid the swap.
		m_forest[b]->parent = m_forest[a];

		// This is the only situation in which the tree depth increases!
		m_forest[a]->depth++;
	}
}

template<class T>
T UnionFind<T>::find(T a) const
{
	Node* n = m_forest[a];

	// Trivial case: a is its own representant
	if(!n->parent)
		return n->value;

	// Path compression: while traversing the tree upwards, set each node's
	// parent pointer to the parent pointer of its parent. This way, the paths
	// get shorter in each call to find().
	// NOTE: This actually makes the depth of the tree smaller, so
	//  Node::depth is now only an upper bound on the tree depth.
	while(n->parent && n->parent->parent)
	{
		n = n->parent = n->parent->parent;
	}

	if(n->parent)
		return n->parent->value;
	else
		return n->value;
}

template<class T>
bool UnionFind<T>::isRepresentant(T v) const
{
	return !m_forest[v]->parent;
}

template<class T>
void UnionFind<T>::fastDisconnectElement(T v)
{
	// Just reset all tree-related information in v.
	m_forest[v]->parent = 0;
	m_forest[v]->depth = 0;
}

template<class T>
template<class Container>
void UnionFind<T>::dissolve(const Container& values)
{
	for(T val : values)
		fastDisconnectElement(val);
}

#endif
