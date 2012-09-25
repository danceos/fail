#ifndef __BUFFER_INTERFACE_HPP__
  #define __BUFFER_INTERFACE_HPP__

#include <cstddef>

namespace fail {

typedef std::size_t index_t; //!< the index type of elements, stored in the buffer-list
const index_t INVALID_INDEX = static_cast<index_t>(-1); //!< a constant, representing an invalid idx

/**
 * \class PerfBufferBase
 *
 * Generic performance data structure for storing often used listener objects
 * (This class can be seen as a "fast, type specific buffer-list".)
 */
class PerfBufferBase {
public:
	/**
	 * Adds the index \c idx to the performance buffer-list.
	 * @param idx the element to be added
	 */
	virtual void add(index_t idx) = 0;
	/**
	 * Removes the specified index \c idx.
	 * @param idx the element to be deleted
	 */
	virtual void remove(index_t idx) = 0;
	/**
	 * Removes all elements from the perf. buffer-list.
	 */
	virtual void clear() = 0;
	/**
	 * Retrieves the number of elements in this data structure.
	 * @return the number of elements in the perf. buffer-list
	 */
	virtual std::size_t size() const = 0;
};

} // end-of-namespace: fail

#endif // __BUFFER_INTERFACE_HPP__
