#ifndef __MEMORYMAP_HPP__
#define __MEMORYMAP_HPP__

#ifdef BOOST_1_46_OR_NEWER
#include <boost/icl/interval_set.hpp>
using namespace boost::icl;
#endif

#include <set>

#include "sal/SALConfig.hpp"

namespace fail {

/**
 * \class MemoryMap
 * An efficient container for memory maps with holes.
 */
class MemoryMap {
private:
#ifdef BOOST_1_46_OR_NEWER
	typedef interval<address_t>::type address_interval;
	typedef interval_set<address_t>::type address_set;

	address_set as;
public:
	MemoryMap() { }
	void clear() { as.clear(); }
	void add(address_t addr, int size) { as.add(address_interval(addr, addr+size-1)); }
	void isMatching(address_t addr, int size) { return intersects(as, address_interval(addr, addr+size-1)); }
#endif
	std::set<address_t> as;
public:
	MemoryMap() { }
	/**
	 * Clears the map.
	 */
	void clear() { as.clear(); }
	/**
	 * Adds one or a sequence of addresses to the map.
	 */
	void add(address_t addr, int size = 1)
	{
		for (int i = 0; i < size; ++i) {
			as.insert(addr + i);
		}
	}
	/**
	 * Determines whether a given memory access at address \a addr with width
	 * \a size hits the map.
	 */
	bool isMatching(address_t addr, int size = 1)
	{
		for (int i = 0; i < size; ++i) {
			if (as.find(addr + i) != as.end()) {
				return true;
			}
		}
		return false;
	}
	/**
	 * The (STL-style) iterator of this class used to iterate over all
	 * addresses in this map.
	 */
	typedef std::set<address_t>::iterator iterator;
	/**
	 * Returns an (STL-style) iterator to the beginning of the internal data
	 * structure.
	 */
	iterator begin() { return as.begin(); }
	/**
	 * Returns an (STL-style) iterator to the end of the interal data
	 * structure.
	 */
	iterator end() { return as.end(); }
	/**
	 * Loads a memory map from a file and merges it with the current state.
	 *
	 * File format (addresses and sizes in decimal):
	 * \code
	 * address1<tab>size1
	 * address2<tab>size2
	 * ...
	 * \endcode
	 */
	bool readFromFile(char const * const filename);
	/**
	 * Saves the map to a file.
	 *
	 * Currently all access size information is lost; the map is flattened out
	 * to a long list of single-byte addresses.
	 */
	bool writeToFile(char const * const filename);
};

} // end-of-namespace: fail

#endif // __MEMORYMAP_HPP__
