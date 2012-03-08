#ifndef __MEMORY_HPP__
  #define __MEMORY_HPP__

// Author: Adrian Böckenkamp
// Date:   07.09.2011

#include <vector>
#include <stdint.h>
#include <cstring> // Added for size_t support

#include "SALConfig.hpp"

namespace sal
{

/**
 * \class MemoryManager
 * Defines an abstract interface to access the simulated memory pool
 * and query meta information, e.g. the memory size.
 */
class MemoryManager
{
	public:
		virtual ~MemoryManager() { }
		/**
		 * Retrieves the size of the available simulated memory.
		 * @return the size of the memory pool in bytes
		 */
		virtual size_t getPoolSize() const = 0;
		/**
		 * Retrieves the starting address of the host memory. This is the
		 * first valid address in memory.
		 * @return the starting address
		 */
		virtual host_address_t getStartAddr() const = 0;
		/**
		 * Retrieves the byte at address \a addr in the memory.
		 * @param addr The guest address where the byte is located.
		 *        The address is expected to be valid.
		 * @return the byte at \a addr
		 */
		virtual byte_t getByte(guest_address_t addr) = 0;
		/**
		 * Retrieves \a cnt bytes at address \a addr in the memory.
		 * @param addr The guest address where the bytes are located.
		 *        The address is expected to be valid.
		 * @param cnt the number of bytes to be retrieved. \a addr + \a cnt
		 *        is expected to not exceed the memory limit.
		 * @param dest the destination buffer to write the bytes to
		 */
		virtual void getBytes(guest_address_t addr, size_t cnt, std::vector<byte_t>& dest) = 0;
		/**
		 * Writes the byte \a data to memory.
		 * @param addr The guest address to write.
		 *        The address is expected to be valid.
		 * @param data The new byte to write
		 */
		virtual void setByte(guest_address_t addr, byte_t data) = 0;
		/**
		 * Writes the bytes \a data to memory. Consequently data.size() bytes
		 * will be written.
		 * @param addr The guest address to write.
		 *        The address is expected to be valid.
		 * @param data The new bytes to write
		 */
		virtual void setBytes(guest_address_t addr, const std::vector<byte_t>& data) = 0;
		/**
		 * Transforms the guest address \a addr to a host address.
		 * @param addr The guest address to be transformed
		 * @return the transformed (host) address or \c ADDR_INV on errors
		 */
		virtual host_address_t guestToHost(guest_address_t addr) = 0;
};

} // end-of-namespace: sal

#endif /* __MEMORY_HPP__ */
