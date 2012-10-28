#ifndef __GEM5_MEMORY_HPP__
  #define __GEM5_MEMORY_HPP__

#include "../Memory.hpp"

#include "sim/system.hh"
#include "mem/packet.hh"
#include "mem/physical.hh"

//class System;

namespace fail {

/**
 * \class Gem5MemoryManager
 * Represents a concrete implemenation of the abstract
 * MemoryManager to provide access to gem5's memory pool.
 */
class Gem5MemoryManager : public MemoryManager {
public:
	Gem5MemoryManager(System* system) : m_System(system), m_Mem(&system->getPhysMem()) {}
	
	size_t getPoolSize() const { return m_Mem->totalSize(); }
	host_address_t getStartAddr() const { return 0; }

	byte_t getByte(guest_address_t addr)
	{
		Request req(addr, 1, Request::PHYSICAL, 0);

		Packet pkt(&req, MemCmd::ReadReq);
		byte_t data;
		pkt.dataStatic(&data);

		m_Mem->functionalAccess(&pkt);
		return data;
	}

	void getBytes(guest_address_t addr, size_t cnt, void *dest)
	{
		Request req(addr, cnt, Request::PHYSICAL, 0);

		Packet pkt(&req, MemCmd::ReadReq);
		pkt.dataStatic(dest);

		m_Mem->functionalAccess(&pkt);
	}

	void setByte(guest_address_t addr, byte_t data)
	{
		Request req(addr, 1, Request::PHYSICAL, 0);

		Packet pkt(&req, MemCmd::WriteReq);
		pkt.dataStatic(&data);

		m_Mem->functionalAccess(&pkt);
	}

	void setBytes(guest_address_t addr, size_t cnt, void const *src)
	{
		Request req(addr, cnt, Request::PHYSICAL, 0);

		Packet pkt(&req, MemCmd::WriteReq);
		pkt.dataStatic(src);

		m_Mem->functionalAccess(&pkt);
	}

private:
	System* m_System;
	PhysicalMemory* m_Mem;
};

} // end-of-namespace: fail

#endif // __GEM5_MEMORY_HPP__
