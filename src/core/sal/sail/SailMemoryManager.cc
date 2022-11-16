#include <climits>
#include <stdexcept>
#include "SailMemoryManager.hpp"
#include "SailMemoryExtern.hpp"

using namespace fail;


size_t SailMemoryManager::getPoolSize() const {
	return ULONG_MAX; //Return unsinged long max since pool size is dynamic
}

host_address_t SailMemoryManager::getStartAddr() const {
	return 0;
}

byte_t SailMemoryManager::getByte(guest_address_t addr) {
	if(read_mem)
		return static_cast<byte_t>(read_mem(addr));
	else
		throw std::runtime_error("can't read memory, simulator not linked!");
}

void SailMemoryManager::getBytes(guest_address_t addr, size_t cnt,
								 void *dest) {
	char *d = static_cast<char *>(dest);
	for (size_t i = 0; i < cnt; ++i) d[i] = getByte(addr + i);
}

void SailMemoryManager::setByte(guest_address_t addr, byte_t data) {
	if(write_mem) {
		write_mem(addr, data);
	}
	else
		throw std::runtime_error("can't write memory, simulator not linked");
}

void SailMemoryManager::setBytes(guest_address_t addr, size_t cnt, void const *src) {
	char const *s = static_cast<char const *>(src);
	for (size_t i = 0; i < cnt; ++i) setByte(addr + i, s[i]);
}

bool SailMemoryManager::isMapped(guest_address_t addr) {
	if(!sail_memory) throw std::runtime_error("can't access memory, simulator not linked");

	if(sail_memory) {
		uint64_t address = static_cast<uint64_t>(addr);
		uint64_t mask = address & ~MASK;
		struct block *current = sail_memory;

		while (current != NULL) {
			if (current->block_id == mask) {
				return true;
			} else {
				current = current->next;
			}
		}
	} else throw std::runtime_error("can't access memory, simulator not linked");

	return false;
}

void SailMemoryManager::serialize(std::ostream& os) {
	// at least this must be != NULL, otherwise
	// we can safely assume that the simulator has not been linked
	// in theory both, sail_memory and sail_tags could be NULL if the simulator hasn't actually accessed any memory
	// but since we are executing a program, at the very least that
	// has been loaded to memory
	if(!sail_memory) throw std::runtime_error("can't access memory, simulator not linked");
	if(!os) throw std::runtime_error("can't open ostream.");

	const size_t blk_len = (MASK + 1);
	const size_t blk_size = blk_len*sizeof(uint8_t);

	unsigned long cnt = 0;
	block* start = sail_memory;
	while(sail_memory != NULL) {
		std::cout << "serializing memory block (id=" << sail_memory->block_id << ",size=" << blk_size << ")"
#if 0

				  << std::endl
				  << std::hex
				  << "\tm[0] = 0x"  << (int)sail_memory->mem[0] << std::endl
				  << "\tm[1] = 0x" << (int)sail_memory->mem[1] << std::endl
				  << "\tm[2] = 0x" << (int)sail_memory->mem[2] << std::endl
				  << "\tm[3] = 0x" << (int)sail_memory->mem[3] << std::endl
				  << std::endl;
#else
		<< std::endl;
#endif
		os.write(reinterpret_cast<char*>(&sail_memory->block_id), sizeof(sail_memory->block_id));
		os.write(reinterpret_cast<char*>(sail_memory->mem), blk_size);
		sail_memory = sail_memory->next;
		cnt++;
	}
	std::cout << "serialized " << cnt << " memory blocks" << std::endl;
	sail_memory = start;
}

void SailMemoryManager::unserialize(std::istream& is) {
	if(!is) throw std::runtime_error("can't open istream.");

	reset();
	const size_t blk_len = (MASK + 1);
	const size_t blk_size = blk_len*sizeof(uint8_t);

	unsigned cnt = 0;
	block* last = nullptr;
	while(is) {
		block *b = new block;
		b->mem = new uint8_t[blk_len];
		b->next = NULL;

		is.read(reinterpret_cast<char*>(&b->block_id), sizeof(b->block_id));
		is.read(reinterpret_cast<char*>(b->mem), blk_size);
		if(is.fail()) {
			// XXX: we must check here, since is.eof() will return false, even if the end-of-file has been reached.
			//      it will only return true if the is has been used to read unsuccessfully
			delete b->mem;
			delete b;
			break;
		}
		std::cout << "unserializing memory block (id=" << b->block_id << ",size=" << blk_size << ")"
#if 0
				  << std::endl
				  << std::hex
				  << "\tm[0] = 0x"  << (int)b->mem[0] << std::endl
				  << "\tm[1] = 0x" << (int)b->mem[1] << std::endl
				  << "\tm[2] = 0x" << (int)b->mem[2] << std::endl
				  << "\tm[3] = 0x" << (int)b->mem[3] << std::endl
				  << std::endl;
#else
		<< std::endl;
#endif

		if(sail_memory == nullptr){
			sail_memory = b;
		} else if (last != nullptr) {
			last->next = b;
		} else { throw std::runtime_error("sail_memory is not empty, can't unserialize!"); }

		last = b;
		cnt++;
	}
	std::cerr << "unserialized " <<  cnt << " mem blocks with size: " << blk_size << std::endl;
}

void SailMemoryManager::reset() {
	while(sail_memory != NULL) {
		block *next = sail_memory->next;

		delete sail_memory->mem;
		delete sail_memory;

		sail_memory = next;
	}
}
