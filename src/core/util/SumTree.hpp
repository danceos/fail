#ifndef __SUM_TREE_HPP__
#define __SUM_TREE_HPP__

#include <assert.h>
#include <stdint.h>
#include <vector>
#include <stack>

// The SumTree implements an efficient tree data structure for
// "roulette-wheel" sampling, or "sampling with fault expansion", i.e.,
// sampling of trace entries / pilots with/without replacement and with a
// picking probability proportional to the entries' sizes.
//
// For every sample, the naive approach picks a random number between 0
// and the sum of all entry sizes minus one.  It then iterates over all
// entries and sums their sizes until the sum exceeds the random number.
// The current entry gets picked.  The main disadvantage is the linear
// complexity, which gets unpleasant for millions of entries.
//
// The core idea behind the SumTree implementation is to maintain the
// size sum of groups of entries, kept in "buckets".  Thereby, a bucket
// can be quickly jumped over.  To keep bucket sizes (and thereby linear
// search times) bounded, more bucket hierarchy levels are introduced
// when a defined bucket size limit is reached.
//
// Note that the current implementation is built for a pure growth phase
// (when the tree gets filled with pilots from the database), followed by
// a sampling phase when the tree gets sampled from (with replacement) or
// emptied (without replacement).  It does not handle a mixed add/remove case
// very smartly, although it should remain functional.

namespace fail {

template <typename T, unsigned BUCKETSIZE = 1024>
class SumTree {
	//! Bucket data structure for tree nodes
	struct Bucket {
		Bucket() : size(0) {}
		~Bucket();
		//! Sum of all children / elements
		typename T::size_type size;
		//! Sub-buckets, empty for leaf nodes
		std::vector<Bucket *> children;
		//! Contained elements, empty for inner nodes
		std::vector<T> elements;
	};

public:
	//! Iterator
	class TreeIterator : public std::iterator<std::input_iterator_tag, T> {
		//! Buckets and corresponding element indexes down the tree
		std::stack<std::pair<Bucket *, unsigned> > hierarchy;
	public:
		TreeIterator() {}
//MyIterator(int* x) :p(x) {}
		TreeIterator(const TreeIterator& i) : hierarchy(i.hierarchy) { }
		TreeIterator(const SumTree<T, BUCKETSIZE>& tree)
		{
			// go down until we see leaves
			hierarchy.push(std::pair<Bucket *, unsigned>(tree.m_root, 0));
			while (!hierarchy.top().first->elements.size() && hierarchy.top().first->children.size() > 0) {
				hierarchy.push(std::pair<Bucket *, unsigned>(hierarchy.top().first->children[hierarchy.top().second], 0));
			}
		}
		TreeIterator& operator++()
		{
			// advance index in the current level
			hierarchy.top().second++;
			if (hierarchy.top().second < hierarchy.top().first->elements.size()) {
				return *this;
			}
			// current level is exhausted, go back up to a not yet finished level
			do {
				hierarchy.pop();
			} while (!hierarchy.empty()
				&& ++hierarchy.top().second >= hierarchy.top().first->children.size());
			// at the end?
			if (hierarchy.empty()) {
				return *this;
			}
			// go down until we see leaves again
			do {
				hierarchy.push(std::pair<Bucket *, unsigned>(hierarchy.top().first->children[hierarchy.top().second], 0));
			} while (!hierarchy.top().first->elements.size() && hierarchy.top().first->children.size() > 0);
			return *this;
		}
		TreeIterator operator++(int) { TreeIterator tmp(*this); operator++(); return tmp; }
	    bool operator==(const TreeIterator& rhs) { return hierarchy == rhs.hierarchy; }
		bool operator!=(const TreeIterator& rhs) { return hierarchy != rhs.hierarchy; }
		T& operator*() { return hierarchy.top().first->elements[hierarchy.top().second]; }
		T *operator->() { return &(operator*()); }
	};
	typedef TreeIterator iterator;

	iterator begin()
	{
		return iterator(*this);
	}

	iterator end()
	{
		return iterator();
	}

private:
	//! Root node
	Bucket *m_root;
	//! Tree depth: nodes at level m_depth are leaf nodes, others are inner nodes
	unsigned m_depth;
public:
	T notfound; //! Returned if no element was found.
	SumTree() : m_root(new Bucket), m_depth(0) {}
	~SumTree() { delete m_root; }
	//! Adds a copy of a new element to the tree.  The copy is created internally.
	void add(const T& element);
	//! Retrieves and removes element at random number position.
	T remove(typename T::size_type pos) { return remove(pos, m_root, 0); }
	//! Retrieves reference to element at random number position.
	T& get(typename T::size_type pos) { return get(pos, m_root, 0); }
	//! Yields the sum over all elements in the tree.
	typename T::size_type get_size() const { return m_root->size; }
private:
	//! Internal, recursive version of add().
	bool add(Bucket **node, const T& element, unsigned depth_remaining);
	//! Internal, recursive version of remove().
	T remove(typename T::size_type pos, Bucket *node, typename T::size_type sum);
	//! Internal, recursive version of get().
	T& get(typename T::size_type pos, Bucket *node, typename T::size_type sum);
};

// template implementation

template <typename T, unsigned BUCKETSIZE>
SumTree<T, BUCKETSIZE>::Bucket::~Bucket()
{
	for (typename std::vector<Bucket *>::const_iterator it = children.begin();
		it != children.end(); ++it) {
		delete *it;
	}
}

template <typename T, unsigned BUCKETSIZE>
void SumTree<T, BUCKETSIZE>::add(const T& element)
{
	if (element.size() == 0) {
		// pilots with size == 0 cannot be picked anyways
		return;
	}

	if (add(&m_root, element, m_depth)) {
		// tree wasn't full yet, add succeeded
		return;
	}

	// tree is full, move everything one level down
	++m_depth;
	Bucket *b = new Bucket;
	b->children.push_back(m_root);
	b->size = m_root->size;
	m_root = b;

	// retry
	add(&m_root, element, m_depth);
}

template <typename T, unsigned BUCKETSIZE>
bool SumTree<T, BUCKETSIZE>::add(Bucket **node, const T& element, unsigned depth_remaining)
{
	// non-leaf node?
	if (depth_remaining) {
		// no children yet?  create one.
		if ((*node)->children.size() == 0) {
			(*node)->children.push_back(new Bucket);
		}

		// adding to newest child worked?
		if (add(&(*node)->children.back(), element, depth_remaining - 1)) {
			(*node)->size += element.size();
			return true;
		}

		// newest child full, may we create another one?
		if ((*node)->children.size() < BUCKETSIZE) {
			(*node)->children.push_back(new Bucket);
			add(&(*node)->children.back(), element, depth_remaining - 1);
			(*node)->size += element.size();
			return true;
		}
		// recursive add ultimately failed, subtree full
		return false;

	// leaf node
	} else {
		if ((*node)->elements.size() < BUCKETSIZE) {
			(*node)->elements.push_back(element);
			(*node)->size += element.size();
			return true;
		}
		return false;
	}
}

template <typename T, unsigned BUCKETSIZE>
T SumTree<T, BUCKETSIZE>::remove(typename T::size_type pos, Bucket *node, typename T::size_type sum)
{
	// sanity check
	assert(pos >= sum && pos < sum + node->size);

	// will only be entered for inner nodes
	for (typename std::vector<Bucket *>::iterator it = node->children.begin();
		it != node->children.end(); ) {
		sum += (*it)->size;
		if (sum <= pos) {
			++it;
			continue;
		}

		// found containing bucket, recurse
		sum -= (*it)->size;
		T e = remove(pos, *it, sum);
		node->size -= e.size();
		// remove empty (or, at least, zero-sized) child?
		if ((*it)->size == 0) {
			delete *it;
			node->children.erase(it);
		}
		return e;
	}

	// will only be entered for leaf nodes
	for (typename std::vector<T>::iterator it = node->elements.begin();
		it != node->elements.end(); ) {
		sum += it->size();
		if (sum <= pos) {
			++it;
			continue;
		}

		// found pilot
		T e = *it;
		node->size -= e.size();
		node->elements.erase(it);
		return e;
	}

	// this should never happen
	assert(0);
	return notfound;
}

template <typename T, unsigned BUCKETSIZE>
T& SumTree<T, BUCKETSIZE>::get(typename T::size_type pos, Bucket *node, typename T::size_type sum)
{
	// sanity check
	assert(pos >= sum && pos < sum + node->size);

	// will only be entered for inner nodes
	for (typename std::vector<Bucket *>::iterator it = node->children.begin();
		it != node->children.end(); ) {
		sum += (*it)->size;
		if (sum <= pos) {
			++it;
			continue;
		}

		// found containing bucket, recurse
		sum -= (*it)->size;
		return get(pos, *it, sum);
	}

	// will only be entered for leaf nodes
	for (typename std::vector<T>::iterator it = node->elements.begin();
		it != node->elements.end(); ) {
		sum += it->size();
		if (sum <= pos) {
			++it;
			continue;
		}

		// found pilot
		return *it;
	}

	// this should never happen
	assert(0);
	return notfound;
}

} // namespace

#endif
