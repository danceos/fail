//! The sail internal memory structure
struct block {
    uint64_t block_id;
    uint8_t *mem;
    struct block *next;
};
//! sail-internal tag memory structure
struct tag_block {
    uint64_t block_id;
    bool *mem;
    struct tag_block *next;
};

//! weak-reference to start of sail memory
extern "C" __attribute__((weak)) struct block *sail_memory;
//! weak-symbol to beginning of tag memory linked-list.
extern "C" __attribute__((weak)) struct tag_block *sail_tags;
//! weak-reference to block mask.
extern "C" __attribute__((weak)) uint64_t MASK;

/**
 * Retrieves the data at \a address from the sail memory representation.
 * @param address The address where the data is located.
 * @return The data at \a address.
 */
extern "C" __attribute__((weak)) uint64_t read_mem(uint64_t address);
/**
 * Writes the data to \a address in the memory.
 * @param address The address to write.
 * @param byte The new data to write.
 */
extern "C" __attribute__((weak)) void write_mem(uint64_t address, uint64_t byte);

/**
 * Sail-internal function to read a tag bit at the specified \a address.
 * \param address The address of which the tag bit should be returned.
 * \return The tag bit at the specified address.
 */
extern "C" __attribute__((weak)) bool read_tag_bool(const uint64_t address);
/**
 * Sail-internal function to write a tag bit for the given \a address.
 * \param address The address at which the tag bit will be modified.
 * \param tag Which tag to write at the address.
 */
extern "C" __attribute__((weak)) int write_tag_bool(const uint64_t address, const bool tag);
