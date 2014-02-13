#ifndef __BOCHS_MEMORY_HPP__
#define __BOCHS_MEMORY_HPP__

#include "../Memory.hpp"

namespace fail {

/**
 * \class BochsMemoryManager
 * Represents a concrete implemenation of the abstract
 * MemoryManager to provide access to Bochs' memory pool.
 */
class BochsMemoryManager : public MemoryManager {
public:
	/**
	 * Constructs a new MemoryManager object and initializes
	 * it's attributes appropriately.
	 */
	BochsMemoryManager() : MemoryManager() {  }
	/**
	 * Retrieves the size of the available simulated memory.
	 * @return the size of the memory pool in bytes
	 */
	size_t getPoolSize() const { return static_cast<size_t>(BX_MEM(0)->get_memory_len()); }
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
	byte_t getByte(guest_address_t addr)
	{
		host_address_t haddr = guestToHost(addr);
		assert(haddr != (host_address_t)ADDR_INV && "FATAL ERROR: Invalid guest address provided!");
		return static_cast<byte_t>(*reinterpret_cast<Bit8u*>(haddr));
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
		char *d = static_cast<char *>(dest);
		for (size_t i = 0; i < cnt; ++i)
			d[i] = getByte(addr + i);
	}
	/**
	 * Writes the byte \a data to memory.
	 * @param addr The guest address to write.
	 *        The address is expected to be valid.
	 * @param data The new byte to write
	 */
	void setByte(guest_address_t addr, byte_t data)
	{
		host_address_t haddr = guestToHost(addr);
		assert(haddr != (host_address_t)ADDR_INV &&
			   "FATAL ERROR: Invalid guest address provided!");
		*reinterpret_cast<Bit8u*>(haddr) = data;
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
		char const *s = static_cast<char const *>(src);
		for (size_t i = 0; i < cnt; ++i)
			setByte(addr + i, s[i]);
	}
	/**
	 * Transforms the guest address \a addr to a host address.  Bochs specific.
	 * @param addr The (logical) guest address to be transformed
	 * @return the transformed (host) address or \c ADDR_INV on errors
	 */
	host_address_t guestToHost(guest_address_t addr)
	{
		const unsigned SEGMENT_SELECTOR_IDX = 2; // always the code segment
		const bx_address logicalAddr = static_cast<bx_address>(addr); // offset within the segment
		// Get the linear address:
		Bit32u linearAddr = BX_CPU(0)->get_laddr32(SEGMENT_SELECTOR_IDX/*seg*/, logicalAddr/*offset*/);
		// Map the linear address to the physical address:
		bx_phy_address physicalAddr;
		bx_bool fValid = BX_CPU(0)->dbg_xlate_linear2phy(linearAddr, (bx_phy_address*)&physicalAddr);
		if (!fValid) {
			return (host_address_t) ADDR_INV; // error
		}
		// Determine the *host* address of the physical address:
		Bit8u* hostAddr = BX_MEM(0)->getHostMemAddr(BX_CPU(0), physicalAddr, BX_RW);
		if (!hostAddr) {
			return (host_address_t) ADDR_INV; // error
		}

		return reinterpret_cast<host_address_t>(hostAddr);
	}

	/**
	 * Checks whether memory is mapped and available.
	 * @param addr The guest address to check.
	 */
	virtual bool isMapped(guest_address_t addr) {
		host_address_t hostaddr = guestToHost(addr);
		if (hostaddr == (host_address_t)ADDR_INV)
			return false;
		return true;
	}
};

} // end-of-namespace: fail

#endif // __BOCHS_MEMORY_HPP__
