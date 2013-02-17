#ifndef __OVP_MEMORY_HPP__
  #define __OVP_MEMORY_HPP__

#include "../Memory.hpp"

namespace fail {

/**
 * \class OVPMemoryManager
 * Represents a concrete implemenation of the abstract
 * MemoryManager to provide access to OVP's memory pool.
 */
class OVPMemoryManager : public MemoryManager {
public:
	/**
	 * Constructs a new MemoryManager object and initializes
	 * it's attributes appropriately.
	 */
	OVPMemoryManager() : MemoryManager() { }
	/**
	 * Retrieves the size of the available simulated memory.
	 * @return the size of the memory pool in bytes
	 */
	size_t getPoolSize() const { return 0; }
	/**
	 * Retrieves the starting address of the host memory. This is the
	 * first valid address in memory.
	 * @return the starting address
	 */
	host_address_t getStartAddr() const { return 0; }
	/**
	 * Retrieves the byte at address \a addr in the memory.
	 * @param addr The guest address where the byte is located.
	 *        The address is expected to be valid.
	 * @return the byte at \a addr
	 */
	byte_t getByte(guest_address_t addr) { return 0; }
	/**
	 * Retrieves \a cnt bytes at address \a addr in the memory.
	 * @param addr The guest address where the bytes are located.
	 *        The address is expected to be valid.
	 * @param cnt the number of bytes to be retrieved. \a addr + \a cnt
	 *        is expected to not exceed the memory limit.
	 * @param dest Pointer to destination buffer to copy the data to.
	 */
	void getBytes(guest_address_t addr, size_t cnt, void *dest) { }
	/**
	 * Writes the byte \a data to memory.
	 * @param addr The guest address to write.
	 *        The address is expected to be valid.
	 * @param data The new byte to write
	 */
	void setByte(guest_address_t addr, byte_t data) { }
	/**
	 * Copies data to memory.
	 * @param addr The guest address to write.
	 *        The address is expected to be valid.
	 * @param cnt The number of bytes to be retrieved. \a addr + \a cnt
	 *        is expected to not exceed the memory limit.
	 * @param src Pointer to data to be copied.
	 */
	void setBytes(guest_address_t addr, size_t cnt, void const *src) { }
};

} // end-of-namespace: fail

#endif // __OVP_MEMORY_HPP__
