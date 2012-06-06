#ifndef __BUFFERCACHE_HPP__
#define __BUFFERCACHE_HPP__

#include <stdlib.h>

namespace fi {

/**
 * \class BufferCache
 *
 * \brief A simple dynamic array
 *
 * This class is intended to serve as a kind of cache for the entirely STL-based,
 * untyped and therefore quite slow event handling mechanism of Fail*.
 * To keep the code easily readable, the buffer management methods
 * are less performant than the could be (remove() and erase() have linear complexity).
 */
template<class T> class BufferCache {
public:
	BufferCache()
		: m_Buffer(NULL), m_Buffer_count(0) {}
	~BufferCache() {}
	/**
	 * Add an element to the array. The object pointed to remains untouched.
	 * @param val the element to add
	 * @returns 0 if successful, an error code otherwise (ATM only 10 if malloc() fails)
	 */
	int add(T val);
	/**
	 * Remove an element from the array. The object pointed to remains untouched.
	 * @param val the element to remove
	 * @returns 0 if successful, an error code otherwise (ATM only 10 if malloc() fails)
	 */
	int remove(T val);
	/**
	 * Remove an element at a specific position. The object pointed to remains untouched.
	 * @param val the element to remove
	 * @returns a pointer to the given element's successor if successful, -1 otherwise
	 */
	int erase(int i);
	/**
	 * Clears the array, removing all elements. The objects pointed to remain untouched.
	 */
	void clear();
	/**
	 * Retrieve an element from the array. Should be inlined.
	 * @param idx the position to retrieve the element from
	 * @returns the element at the given position
	 */
	inline T get(size_t idx) { return m_Buffer[idx]; }
	/**
	 * Set an element at a given position. Should be inlined.
	 * @param idx the position to change an element at
	 * @param val the new value of the given element
	 */
	inline void set(size_t idx, T val) { m_Buffer[idx] = val; }
	/**
	 * Retrieves the current length of the array. Should be inlined.
	 * @returns the array length
	 */
	inline size_t get_count() { return m_Buffer_count; }
protected:
	/**
	 * Changes the current length of the array. Should be inlined.
	 * @param new_count the new array length
	 */
	inline void set_count(size_t new_count) { m_Buffer_count = new_count; }
	/**
	 * Reallocates the buffer. This implementation is extremely primitive,
	 * but since the amount of entries is small,
	 * this will not be significant, hopefully. Should be inlined.
	 * @param new_size the new number of elements in the array
	 * @returns 0 if successful, an error code otherwise (ATM only 10 if malloc() fails)
	 */
	inline int reallocate_buffer(size_t new_size);
private:
	T *m_Buffer;
	size_t m_Buffer_count;
};

} /* namespace fi */
#endif /* BUFFERCACHE_H_ */
