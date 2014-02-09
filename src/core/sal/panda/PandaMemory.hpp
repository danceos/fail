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
	PandaMemoryManager() : MemoryManager() { }
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

			uint8_t buf;

			oocdw_read_from_memory(addr, 1, 1, &buf);

			return buf;
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

		uint8_t *d = static_cast<uint8_t *>(dest);

		// Write in 4-Byte chunks

		// ToDo: Correct byte ordering?

		if (cnt >= 4) {
			oocdw_read_from_memory(addr, 4, cnt / 4, d);
		}

		if ((cnt % 4) != 0) {
			oocdw_read_from_memory(addr + (cnt/4) * 4, 1, cnt % 4, d);
		}
	}

	/**
	 * Writes the byte \a data to memory.
	 * @param addr The guest address to write.
	 *        The address is expected to be valid.
	 * @param data The new byte to write
	 */
	void setByte(guest_address_t addr, byte_t data)
	{
		// ToDo: Address translation

		oocdw_write_to_memory(addr, 1, 1, &data, true);
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
		// ToDo: Address translation

		uint8_t const *s = static_cast<uint8_t const *>(src);

		// Write in 4-Byte chunks

		// ToDo: Correct byte ordering?
		if (cnt >= 4) {
			oocdw_write_to_memory(addr, 4, cnt / 4, s, true);
		}

		if ((cnt % 4) != 0) {
			oocdw_write_to_memory(addr + (cnt/4) * 4, 1, cnt % 4, s, true);
		}
	}
};

} // end-of-namespace: fail

#endif // __PANDA_MEMORY_HPP__
