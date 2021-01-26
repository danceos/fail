#include <climits>
#include <stdexcept>
#include "SailTagMemoryManager.hpp"
#include "../SailMemoryExtern.hpp"


namespace fail {

size_t SailTagMemoryManager::getPoolSize() const {
	return ULONG_MAX; //Return unsinged long max since pool size is dynamic
}

host_address_t SailTagMemoryManager::getStartAddr() const {
	return 0;
}

byte_t SailTagMemoryManager::getByte(guest_address_t addr) {
	if(read_tag_bool)
		return static_cast<byte_t>(read_tag_bool(addr));
	else
		throw std::runtime_error("can't read memory, simulator not linked!");
}

void SailTagMemoryManager::getBytes(guest_address_t addr, size_t cnt,
									void *dest) {
	char *d = static_cast<char *>(dest);
	for (size_t i = 0; i < cnt; ++i) d[i] = getByte(addr + i);
}

void SailTagMemoryManager::setByte(guest_address_t addr, byte_t data) {
	if(write_tag_bool) {
		write_tag_bool(addr, static_cast<bool>(data));
	}
	else
		throw std::runtime_error("can't write memory, simulator not linked");
}

void SailTagMemoryManager::setBytes(guest_address_t addr, size_t cnt, void const *src) {
	char const *s = static_cast<char const *>(src);
	for (size_t i = 0; i < cnt; ++i) setByte(addr + i, s[i]);
}

bool SailTagMemoryManager::isMapped(guest_address_t addr) {
	if(!sail_tags) throw std::runtime_error("can't access memory, simulator not linked");

	if(sail_tags) {
		uint64_t address = static_cast<uint64_t>(addr);
		uint64_t mask = address & ~MASK;
		tag_block *current = sail_tags;

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

void SailTagMemoryManager::serialize(std::ostream& os) {
	// at least this must be != NULL, otherwise
	// we can safely assume that the simulator has not been linked
	// in theory both, sail_memory and sail_tags could be NULL if the simulator hasn't actually accessed any memory
	// but since we are executing a program, at the very least that
	// has been loaded to memory
	if(!sail_memory) throw std::runtime_error("can't access memory, simulator not linked");
	if(!os) throw std::runtime_error("can't open ostream.");

	const size_t tag_len = (MASK + 1);
	const size_t tag_size = tag_len*sizeof(bool);

	unsigned cnt = 0;
	tag_block* start = sail_tags;
	while(sail_tags != NULL) {
		std::cout << "serializing tag block (id=" << sail_tags->block_id << ",size=" << tag_size << std::endl;
		os.write(reinterpret_cast<char*>(&sail_tags->block_id), sizeof(sail_tags->block_id));
		os.write(reinterpret_cast<char*>(sail_tags->mem), tag_size);
		sail_tags = sail_tags->next;
		cnt++;
	}
	std::cout << "serialized " << cnt << " tag blocks" << std::endl;
	sail_tags = start;
}

void SailTagMemoryManager::unserialize(std::istream& is) {
	if(!is) throw std::runtime_error("can't open istream.");

	reset();
	const size_t tag_len = (MASK + 1);
	const size_t tag_size = tag_len*sizeof(bool);

	unsigned long cnt = 0;
	tag_block* last;
	while(is) {
		tag_block* b = new tag_block;
		b->mem = new bool[tag_len];
		b->next = NULL;

		is.read(reinterpret_cast<char*>(&b->block_id), sizeof(b->block_id));
		is.read(reinterpret_cast<char*>(b->mem), tag_size);
		if(is.fail()) {
			// XXX: we must check here, since is.eof() will return false, even if the end-of-file has been reached.
			//      it will only return true if the is has been used to read unsuccessfully
			delete b->mem;
			delete b;
			break;
		}

		if(cnt == 0) {
			sail_tags = b;
		} else if(last != NULL) {
			last->next = b;
		} else { throw std::runtime_error("sail_tags is not empty, can't unserialize!"); }

		last = b;
		cnt++;
	}
	std::cerr << "unserialized " << cnt << " tag blocks with size " << tag_size << std::endl;
}

void SailTagMemoryManager::reset() {
	while(sail_tags != NULL) {
		tag_block *next = sail_tags->next;

		delete sail_tags->mem;
		delete sail_tags;

		sail_tags = next;
	}
}

} // namespace fail
