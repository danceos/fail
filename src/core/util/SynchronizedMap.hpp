/**
 * \brief Map class that has thread synchronisation
 */

#ifndef __SYNCHRONIZED_MAP_HPP__
#define __SYNCHRONIZED_MAP_HPP__

#include <map>

#ifndef __puma
#include <boost/thread.hpp>
#endif

// TODO: We should consider to use Aspects for synchronisation primitives.

namespace fail {

template <typename Tkey, typename Tvalue>
class SynchronizedMap {
private:
	typedef std::map< Tkey, Tvalue > Tmap;
	typedef typename Tmap::iterator Tit;

	Tmap m_map; //! Use STL map to store data
#ifndef __puma
	boost::mutex m_mutex; //! The mutex to synchronise on
#endif

	int nextpick;
	// We need a window at least as wide as the number of clients we serve.
	// FIXME better solution: when inbound queue is empty, *copy* in-flight map
	// to a vector, iterate but don't delete; when at the end, copy in-flight
	// map again and repeat
	enum { pick_window_size = 50000 };

public:
	SynchronizedMap() : nextpick(0) { }
	int Size()
	{
#ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
		return m_map.size();
	}
	/**
	 * Retrieves one element from the map from a small window at the beginning.
	 * @return a pointer to the element, or \c NULL if empty
	 */
	Tvalue pickone()
	{
	  #ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
	  #endif
		if (m_map.size() == 0) {
			return NULL;
		}

		// XXX not really elegant: linear complexity
		typename Tmap::iterator it = m_map.begin();
		for (int i = 0; i < nextpick; ++i) {
			++it;
			if (it == m_map.end()) {
				it = m_map.begin();
				nextpick = 0;
				break;
			}
		}
		++nextpick;
		if (nextpick >= pick_window_size) {
			nextpick = 0;
		}
		return it->second;
	} // Lock is automatically released here
	/**
	 * Add data to the map, return false if already present
	 * @param key Map key
	 * @param value value according to key
	 * @return false if key already present
	 */
	bool insert(const Tkey& key, const Tvalue& value)
	{
		// Acquire lock on the queue
#ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
		if ( m_map.find(key) == m_map.end() ) { // not present, add it
			m_map[key] = value;
			return true;
		} else { // item is already in, oops
			return false;
		}

	} // Lock is automatically released here

	/**
	 * Remove value from the map.
	 * @param key The Map key to remove
	 * @return false if key was not present
	 *
	 */
	bool remove(const Tkey& key, Tvalue& value)
	{
		// Acquire lock on the queue
#ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
		Tit iterator;
		if ((iterator = m_map.find(key)) != m_map.end()) {
			value = iterator->second;
			m_map.erase(iterator);
			return true;
		} else {
			return false;
		}
	} // Lock is automatically released here
};

} // end-of-namespace: fail

#endif // __SYNCHRONIZED_MAP_HPP__
