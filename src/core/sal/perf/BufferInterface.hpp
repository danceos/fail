#ifndef __BUFFER_INTERFACE_HPP__
#define __BUFFER_INTERFACE_HPP__

#include <cstddef>
#include <vector>

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

/**
 * \class ResultSet
 *
 * Results (= indices of matching listeners) returned by the "gather"-method,
 * see below. (This class can be seen as a "temporary fire-list".)
 */
class BaseListener;
class ResultSet {
private:
	std::vector<BaseListener *> m_Res; //!< vector of pointers to matching listeners
public:
	ResultSet() { }
	bool hasMore() const { return !m_Res.empty(); }
	BaseListener *getNext() { BaseListener *l = m_Res.back(); m_Res.pop_back(); return l; }
	void add(BaseListener *l) { m_Res.push_back(l); }
	size_t size() const { return m_Res.size(); }
	void clear() { m_Res.clear(); }
};

/**
 * \class DefPerfVector
 *
 * Default \c std::vector based performance implementation (abstract)
 */
template<class T>
class DefPerfVector : public PerfBufferBase {
protected:
	std::vector<index_t> m_BufList; //!< the performance buffer-list
public:
	void add(index_t idx) { m_BufList.push_back(idx); }
	void remove(index_t idx)
	{
		for (std::vector<index_t>::iterator it = m_BufList.begin();
		     it != m_BufList.end(); ++it) {
			if (*it == idx) {
				m_BufList.erase(it);
				break;
			}
		}
	}
	void clear() { m_BufList.clear(); }
	size_t size() const { return m_BufList.size(); }
	virtual ResultSet& gather(T* pData) = 0;
};

} // end-of-namespace: fail

#endif // __BUFFER_INTERFACE_HPP__
