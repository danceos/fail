#ifndef __BUFFER_CACHE_HPP__
  #define __BUFFER_CACHE_HPP__

#include <stdlib.h>
#include <list>

namespace fail {

class ListenerManager;

/**
 * \class BufferCache
 *
 * \brief A simple dynamic array
 *
 * This class is intended to serve as a kind of cache for the
 * untyped and therefore quite slow listener handling mechanism of Fail*.
 */
template<class T>
class BufferCache {
public:
	/**
	 * The list type inherent to this class. Like bufferlist_t in ListenerManager.hpp,
	 * but dynamically typed.
	 */
	typedef std::list<T> cachelist_t;
	/**
	 * The iterator of this class used to loop through the list of
	 * added listeners. To retrieve an iterator to the first element, call
	 * begin(). end() returns the iterator, pointing after the last element.
	 * (This behaviour equals the STL iterator in C++.)
	 */
	typedef typename cachelist_t::iterator iterator;
private:
	cachelist_t m_Buffer; //!< The list holding the cached elements
public:
	BufferCache() { }
	~BufferCache() { }
	/**
	 * Add an element to the array. The object pointed to remains untouched.
	 * @param val the element to add
	 */
	inline void add(const T &val) { m_Buffer.push_back(val); }
	/**
	 * Remove an element from the array. The object pointed to remains untouched.
	 * @param val the element to remove
	 */
	inline void remove(const T &val) { m_Buffer.remove(val); }
	/**
	 * Remove an element at a specific position. The object pointed to remains untouched.
	 * @param val the element to remove
	 * @return a pointer to the given element's successor if successful, a negative value otherwise
	 */
	inline iterator erase(iterator i) { return m_Buffer.erase(i); }
	/**
	 * Clears the array, removing all elements. The objects pointed to remain untouched.
	 */
	inline void clear() { m_Buffer.clear(); }
	/**
	 * Returns an iterator to the beginning of the internal data structure.
	 * Don't forget to update the returned iterator when calling one of the
	 * modifying methods like makeActive() or remove(). Therefore you need
	 * to call the iterator-based variants of makeActive() and remove().
	 * \code
	 * [X|1|2| ... |n]
	 *  ^
	 * \endcode
	 */
	inline iterator begin() { return m_Buffer.begin(); }
	/**
	 * Returns an iterator to the end of the interal data structure.
	 * Don't forget to update the returned iterator when calling one of the
	 * modifying methods like makeActive() or remove(). Therefore you need
	 * to call the iterator-based variants of makeActive() and remove().
	 * \code
	 * [1|2| ... |n]X
	 *              ^
	 * \endcode
	 */
	inline iterator end() { return m_Buffer.end(); }
	/**
	 * Acts as a replacement for ListenerManager::makeActive, manipulating
	 * the buffer cache exclusively. ListenerManager::triggerActiveListeners needs
	 * to be called to fire the active listeners (see there).
	 * This method is declared as a friend method in ListenerManager.
	 * @param idx the index of the listener to trigger
	 * @returns an updated index which can be used to update a loop counter
	 */
	iterator makeActive(ListenerManager &ev_list, iterator idx);
};

} // end-of-namespace: fail

#endif // __BUFFER_CACHE_HPP__
