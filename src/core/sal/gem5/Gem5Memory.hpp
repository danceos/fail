#ifndef __GEM5_MEMORY_HPP__
#define __GEM5_MEMORY_HPP__

#include "../Memory.hpp"
#include "Gem5Wrapper.hpp"

// gem5 forward declarations:
class System;

namespace fail {

/**
 * \class Gem5MemoryManager
 * Represents a concrete implemenation of the abstract
 * MemoryManager to provide access to gem5's memory pool.
 */
class Gem5MemoryManager : public MemoryManager {
private:
	System* m_System;
public:
	Gem5MemoryManager(System* sys) : m_System(sys) { }
	size_t getPoolSize() const { return GetPoolSize(m_System); }
	host_address_t getStartAddr() const { return 0; }
	byte_t getByte(guest_address_t addr)
	{
		byte_t data;
		ReadMemory(m_System, addr, 1, &data);
		return data;
	}
	void getBytes(guest_address_t addr, size_t cnt, void *dest)
	{
		ReadMemory(m_System, addr, cnt, dest);
	}
	void setByte(guest_address_t addr, byte_t data)
	{
		WriteMemory(m_System, addr, 1, &data);
	}
	void setBytes(guest_address_t addr, size_t cnt, void const *src)
	{
		WriteMemory(m_System, addr, cnt, src);
	}
};

} // end-of-namespace: fail

#endif // __GEM5_MEMORY_HPP__
