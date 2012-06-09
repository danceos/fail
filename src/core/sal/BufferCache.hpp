#ifndef __BUFFER_CACHE_HPP__
  #define __BUFFER_CACHE_HPP__

#include <stdlib.h>

namespace fail {

class EventList;

/**
 * \class BufferCache
 *
 * \brief A simple dynamic array
 *
 * This class is intended to serve as a kind of cache for the entirely STL-based,
 * untyped and therefore quite slow event handling mechanism of Fail*.
 * To keep the code easily readable, some buffer management methods
 * perform suboptimally (remove() and erase() have linear complexity).
 * 
 * FIXME: Why not using std::vector? ("A simple dynamic array")
 */
template<class T>
class BufferCache {
private:
	// TODO: comments ("//!<") needed!
	T *m_Buffer;
	int m_BufferCount;
	/**
	 * Changes m_BufferCount. Should be inlined.
	 * @param new_count the new array length
	 */
	inline void setCount(int new_count) { if(new_count >= 0) m_BufferCount = new_count; }
protected:
	/**
	 * Reallocates the buffer. This implementation is extremely primitive,
	 * but since the amount of entries is small,
	 * this will not be significant, hopefully. Should be inlined.
	 * @param new_size the new number of elements in the array
	 * @return 0 if successful, an error code otherwise (10 if realloc() fails, 20 for an invalid new size)
	 */
	inline int reallocate_buffer(int new_size);
public:
	BufferCache()
		: m_Buffer(NULL), m_BufferCount(0) {}
	~BufferCache() {}
	/**
	 * Add an element to the array. The object pointed to remains untouched.
	 * @param val the element to add
	 */
	void add(T val);
	/**
	 * Remove an element from the array. The object pointed to remains untouched.
	 * @param val the element to remove
	 */
	void remove(T val);
	/**
	 * Remove an element at a specific position. The object pointed to remains untouched.
	 * @param val the element to remove
	 * @return a pointer to the given element's successor if successful, a negative value otherwise
	 */
	int erase(int i);
	/**
	 * Clears the array, removing all elements. The objects pointed to remain untouched.
	 */
	void clear();
	/**
	 * Retrieve an element from the array. Should be inlined.
	 * @param idx the position to retrieve the element from
	 * @return the element at the given position
	 */
	inline T get(int idx) { return (idx >= 0 && idx < getCount() ? m_Buffer[idx] : NULL); }
	/**
	 * Set an element at a given position. Should be inlined.
	 * @param idx the position to change an element at
	 * @param val the new value of the given element
	 */
	inline void set(int idx, T val) { if(idx >= 0 && idx < getCount()) m_Buffer[idx] = val; }
	/**
	 * Retrieves the current length of the array. Should be inlined.
	 * @return the array length
	 */
	inline int getCount() { return m_BufferCount; }
	/**
	 * Acts as a replacement for EventList::makeActive, manipulating
	 * the buffer cache exclusively. EventList::fireActiveEvents needs
	 * to be called to fire the active events (see there).
	 * This method is declared as a friend method in EventList.
	 * @param idx the index of the event to trigger
	 * @returns an updated index which can be used to update a loop counter
	 */
	int makeActive(EventList &ev_list, int idx);
};

} // end-of-namespace: fail

#endif // __BUFFER_CACHE_HPP__
