#ifndef __PANDA_MEMORY_HPP__
  #define __PANDA_MEMORY_HPP__

#include "../Memory.hpp"

#include "openocd_wrapper.hpp"

namespace fail {

/**
 * \class PandaMemoryManager
 * Represents a concrete implemenation of the abstract
 * MemoryManager to provide access to the memory pool
 * of the pandaboard.
 */
class PandaMemoryManager : public MemoryManager {
public:
	/**
	 * Constructs a new MemoryManager object and initializes
	 * it's attributes appropriately.
	 */
	PandaMemoryManager() : MemoryManager() {  }
	/**
	 * Retrieves the size of the available simulated memory.
	 * @return the size of the memory pool in bytes
	 */
	size_t getPoolSize() const
	{
		//ToDo (PORT): Get pool size from ELF file?
		return 0xFFFFFFFF;
	}
	/**
	 * Retrieves the starting address of the host memory. This is the
	 * first valid address in memory.
	 * @return the starting address
	 */
	host_address_t getStartAddr() const
	{
		//ToDo Get Start address from ELF file!?
		return 0;
	}

	/**
	 * Retrieves the byte at address \a addr in the memory.
	 * @param addr The guest address where the byte is located.
	 *        The address is expected to be valid.
	 * @return the byte at \a addr
	 */
	byte_t getByte(guest_address_t addr)
	{
			// ToDo: Address translation
			/* host_address_t haddr = guestToHost(addr);
			assert(haddr != (host_address_t)ADDR_INV && "FATAL ERROR: Invalid guest address provided!");
			return static_cast<byte_t>(*reinterpret_cast<Bit8u*>(haddr));*/

			uint8_t buf [1];

			oocdw_read_from_memory(addr, 1, 1, buf);

			return buf[0];
		}

	/**
	 * Retrieves \a cnt bytes at address \a addr from the memory.
	 * @param addr The guest address where the bytes are located.
	 *        The address is expected to be valid.
	 * @param cnt The number of bytes to be retrieved. \a addr + \a cnt
	 *        is expected to not exceed the memory limit.
	 * @param dest Pointer to destination buffer to copy the data to.
	 */
	void getBytes(guest_address_t addr, size_t cnt, void *dest)
	{
		// ToDo: Address translation
		/* host_address_t haddr = guestToHost(addr);
		assert(haddr != (host_address_t)ADDR_INV && "FATAL ERROR: Invalid guest address provided!");
		return static_cast<byte_t>(*reinterpret_cast<Bit8u*>(haddr));*/

		uint8_t *d = static_cast<uint8_t *>(dest);

		oocdw_read_from_memory(addr, 1, cnt, d);
	}

	/**
	 * Writes the byte \a data to memory.
	 * @param addr The guest address to write.
	 *        The address is expected to be valid.
	 * @param data The new byte to write
	 */
	void setByte(guest_address_t addr, byte_t data)
	{
		oocdw_write_to_memory(addr, 1, 1, &data, false);
	}

	/**
	 * Copies data to memory.
	 * @param addr The guest address to write.
	 *        The address is expected to be valid.
	 * @param cnt The number of bytes to be retrieved. \a addr + \a cnt
	 *        is expected to not exceed the memory limit.
	 * @param src Pointer to data to be copied.
	 */
	void setBytes(guest_address_t addr, size_t cnt, void const *src)
	{
		// ToDo (PORT): This works, but may be slower than writing a whole block
		uint8_t const *s = static_cast<uint8_t const *>(src);

		// ToDo: Check Endianess?
		oocdw_write_to_memory(addr, 1, cnt, s, false);
	}
};

} // end-of-namespace: fail

#endif // __PANDA_MEMORY_HPP__
