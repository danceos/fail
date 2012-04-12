// Map class that has thread synchronisation
// TODO We should consider to use Aspects for synchronisation primitives..

#ifndef __SYNCHRONIZED_MAP_HPP__
#define __SYNCHRONIZED_MAP_HPP__

#include <map>
#ifndef __puma
#include <boost/thread.hpp>
#endif

template <typename Tkey, typename Tvalue>
class SynchronizedMap
{
private:
	typedef std::map< Tkey, Tvalue > Tmap;
	typedef typename Tmap::iterator Tit;
	
	Tmap m_map;				// Use STL map to store data
#ifndef __puma
	boost::mutex m_mutex;				// The mutex to synchronise on
#endif
	public:
		int Size(){
#ifndef __puma
			boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
			return m_map.size();
		}
	/**
	 * Retrieves the first element in the map.
	 * @return a pointer to the first element, or \c NULL if empty
	 */
	Tvalue first()
	{
	  #ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
	  #endif
		if(m_map.size() > 0)
			return m_map.begin()->second;
		else
			return NULL;
	} // Lock is automatically released here
	/**
	 * Add data to the map, return false if already present
	 * @param key Map key
	 * @param value value according to key
	 * @return false if key already present 
	 */
	bool insert(const Tkey& key, const Tvalue& value )
	{
		// Acquire lock on the queue
#ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
		if( m_map.find(key) == m_map.end() ){ // not present, add it
			m_map[key] = value;
			return true;
		}else{ // item is already in, oops
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
		if ( (iterator = m_map.find(key)) != m_map.end() ) {
			value = iterator->second;
			m_map.erase(iterator);
			return true;
		}else{
			return false;
		}
	} // Lock is automatically released here


};
#endif
