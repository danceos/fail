#ifndef __BOCHS_MEMORY_HPP__
  #define __BOCHS_MEMORY_HPP__

#include "../Memory.hpp"

namespace sal
{

/**
 * \class BochsMemoryManager
 * Represents a concrete implemenation of the abstract
 * MemoryManager to provide access to Bochs' memory pool.
 */
class BochsMemoryManager : public MemoryManager
{
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
		size_t getPoolSize() const
		{
			return (static_cast<size_t>(BX_MEM(0)->get_memory_len()));
		}
		/**
		 * Retrieves the starting address of the host memory. This is the
		 * first valid address in memory.
		 * @return the starting address
		 */
		host_address_t getStartAddr() const
		{
			return (0);
		}
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
			return (static_cast<byte_t>(*reinterpret_cast<Bit8u*>(haddr)));
		}
		/**
		 * Retrieves \a cnt bytes at address \a addr in the memory.
		 * @param addr The guest address where the bytes are located.
		 *        The address is expected to be valid.
		 * @param cnt The number of bytes to be retrieved. \a addr + \a cnt
		 *        is expected to not exceed the memory limit.
		 * @param dest The destination buffer to write the bytes to
		 */
		void getBytes(guest_address_t addr, size_t cnt, std::vector<byte_t>& dest)
		{
			for(size_t i = 0; i < cnt; i++)
				dest.push_back(getByte(addr+i));
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
		 * Writes the bytes \a data to memory. Consequently \c data.size()
		 * bytes will be written.
		 * @param addr The guest address to write.
		 *        The address is expected to be valid.
		 * @param data The new bytes to write
		 */
		void setBytes(guest_address_t addr, const std::vector<byte_t>& data)
		{
			for(size_t i = 0; i < data.size(); i++)
				setByte(addr+i, data[i]);
		}
		/**
		 * Transforms the guest address \a addr to a host address.
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
			// Determine the *host* address of the physical address:
			Bit8u* hostAddr = BX_MEM(0)->getHostMemAddr(BX_CPU(0), physicalAddr, BX_READ);
			// Now, hostAddr contains the "final" address
	  		if(!fValid)
				return ((host_address_t)ADDR_INV); // error
			else
				return (reinterpret_cast<host_address_t>(hostAddr)); // okay
		}
};

}

#endif
