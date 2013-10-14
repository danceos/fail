#ifndef __QEMU_MEMORY_HPP__
#define __QEMU_MEMORY_HPP__

#include "../Memory.hpp"

extern "C" {
#include "qemu/failqemu.h"
}

namespace fail {

/**
 * \class QEMUMemoryManager
 * Represents a concrete implemenation of the abstract
 * MemoryManager to provide access to QEMU's memory pool.
 */
class QEMUMemoryManager : public MemoryManager {
public:
	size_t getPoolSize() const { return 0; /* TODO */ }
	host_address_t getStartAddr() const { return 0; }
	byte_t getByte(guest_address_t addr)
	{
		return failqemu_mem_read_byte(addr);
	}
	void getBytes(guest_address_t addr, size_t cnt, void *dest)
	{
		char *d = static_cast<char *>(dest);
		for (size_t i = 0; i < cnt; ++i)
			d[i] = getByte(addr + i);
	}
	void setByte(guest_address_t addr, byte_t data)
	{
		failqemu_mem_write_byte(addr, data);
	}
	void setBytes(guest_address_t addr, size_t cnt, void const *src)
	{
		char const *s = static_cast<char const *>(src);
		for (size_t i = 0; i < cnt; ++i)
			setByte(addr + i, s[i]);
	}
};

} // end-of-namespace: fail

#endif // __QEMU_MEMORY_HPP__
