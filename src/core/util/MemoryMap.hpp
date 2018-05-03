#ifndef __MEMORYMAP_HPP__
#define __MEMORYMAP_HPP__

#ifndef __puma
#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/closed_interval.hpp>
#include <boost/icl/interval_set.hpp>
#endif

#include <ostream>

#include "sal/SALConfig.hpp"

namespace fail {

/**
 * \class MemoryMap
 * An efficient container for memory maps with holes.
 */
class MemoryMap {
#ifndef __puma
private:
	typedef boost::icl::discrete_interval<address_t>::type address_interval;
	typedef boost::icl::interval_set<address_t>::type address_set;

	address_set as;

public:
	/**
	 * The (STL-style) iterator of this class used to iterate over all
	 * addresses in this map.
	 */
	typedef address_set::element_iterator iterator;
#else
public:
	typedef int const* iterator;
#endif

public:
	/**
	 * Clears the map.
	 */
	void clear()
	{
#ifndef __puma
		as.clear();
#endif
	}

	/**
	 * Adds one or a sequence of addresses to the map.
	 */
	void add(address_t addr, int size = 1)
	{
#ifndef __puma
		as.add(boost::icl::construct<address_interval>(addr, addr + size - 1, boost::icl::interval_bounds::closed()));
#endif
	}

	/**
	 * Determines whether a given memory access at address \a addr with width
	 * \a size hits the map.
	 */
	bool isMatching(address_t addr, int size = 1)
	{
#ifndef __puma
		return boost::icl::intersects(as, boost::icl::construct<address_interval>(addr, addr + size - 1, boost::icl::interval_bounds::closed()));
#endif
	}

	/**
	 * Returns an (STL-style) iterator to the beginning of the internal data
	 * structure.
	 */
	iterator begin()
	{
#ifndef __puma
		return boost::icl::elements_begin(as);
#endif
	}

	/**
	 * Returns an (STL-style) iterator to the end of the interal data
	 * structure.
	 */
	iterator end()
	{
#ifndef __puma
		return boost::icl::elements_end(as);
#endif
	}

	/**
	 * Loads a memory map from a file and merges it with the current state.
	 *
	 * File format (addresses and sizes in decimal):
	 * \code
	 * address1<tab>size1
	 * address2<tab>size2
	 * ...
	 * \endcode
	 *
	 * @return false if file could not be read
	 */
	bool readFromFile(char const * const filename);
	/**
	 * Saves the map to a file.
	 *
	 * Currently all access size information is lost; the map is flattened out
	 * to a long list of single-byte addresses.
	 *
	 * @return false if file could not be written
	 */
	bool writeToFile(char const * const filename);

	// debugging
	void dump(std::ostream& os)
	{
#ifndef __puma
		os << as << std::endl;
#endif
	}
};

} // end-of-namespace: fail

#endif // __MEMORYMAP_HPP__
