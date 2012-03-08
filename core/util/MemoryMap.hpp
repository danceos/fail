#ifndef __MEMORYMAP_HPP__
  #define __MEMORYMAP_HPP__

#ifdef BOOST_1_46_OR_NEWER
#include <boost/icl/interval_set.hpp>
using namespace boost::icl;
#endif

#include <set>
#include "SAL/SALConfig.hpp"

/**
 * \class MemoryMap
 * An efficient container for memory maps with holes.
 */
class MemoryMap
{
#ifdef BOOST_1_46_OR_NEWER
	typedef interval<sal::address_t>::type address_interval;
	typedef interval_set<sal::address_t>::type address_set;

	address_set as;
public:
	MemoryMap() {}
	void clear() { as.clear(); }
	void add(sal::address_t addr, int size) { as.add(address_interval(addr, addr+size-1)); }
	void isMatching(sal::address_t addr, int size) { return intersects(as, address_interval(addr, addr+size-1)); }
#endif
	std::set<sal::address_t> as;
public:
	MemoryMap() {}
	void clear() { as.clear(); }
	void add(sal::address_t addr, int size = 1)
	{
		for (int i = 0; i < size; ++i) {
			as.insert(addr + i);
		}
	}
	bool isMatching(sal::address_t addr, int size = 1)
	{
		for (int i = 0; i < size; ++i) {
			if (as.find(addr + i) != as.end()) {
				return true;
			}
		}
		return false;
	}
};

#endif
