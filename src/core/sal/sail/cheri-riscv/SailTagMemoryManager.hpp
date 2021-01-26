#pragma once

#include "../../Memory.hpp"
#include <iostream>

namespace fail {
    class SailTagMemoryManager: public MemoryManager {
        private:
            static size_t tag_len;
            static size_t tag_size;
        public:
            /**
             * Constructs a new MemoryManager object and initializes
             * it's attributes appropriately.
             */
            SailTagMemoryManager() : MemoryManager() { }
            /**
             * Retrieves the size of the currently allocated memory blocks.
             * @return the size of the currently allocated memory blocks in bytes
             */
            size_t getPoolSize() const;
            /**
             * Retrieves the starting address of the host memory. This is the
             * first valid address in memory.
             * @return the starting address
             */
            host_address_t getStartAddr() const;
            /**
             * Retrieves the byte at address \a addr in the memory.
             * @param addr The guest address where the byte is located.
             * @return the byte at \a addr
             */
            byte_t getByte(guest_address_t addr);
            /**
             * Retrieves \a cnt bytes at address \a addr from the memory.
             * @param addr The guest address where the bytes are located.
             * @param cnt The number of bytes to be retrieved.
             * @param dest Pointer to destination buffer to copy the data to.
             */
            void getBytes(guest_address_t addr, size_t cnt, void *dest);
            /**
             * Writes the byte \a data to memory.
             * @param addr The guest address to write.
             * @param data The new byte to write
             */
            void setByte(guest_address_t addr, byte_t data);
            /**
             * Copies data to memory.
             * @param addr The guest address to write.
             * @param cnt The number of bytes to be retrieved.
             * @param src Pointer to data to be copied.
             */
            void setBytes(guest_address_t addr, size_t cnt, void const *src);
            /**
             * Checks whether memory is mapped and available.
             * @param addr The guest address to check.
             */
            bool isMapped(guest_address_t addr);
            /**
             * Serialize the memory state to output streams.
             * @param mem_os The output stream for memory blocks.
             * @param tag_os The output stream for memory tag blocks.
             */
            void serialize(std::ostream& os);

            /**
             * Unserialize the memory state from an input streams.
             * @param mem_is The input stream for memory blocks.
             * @param tag_is The input stream for memory tag blocks.
             */
            void unserialize(std::istream& is);

            /**
             * Reset the memory state
             */
            void reset();
    };
}
