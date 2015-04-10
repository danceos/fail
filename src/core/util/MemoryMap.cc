#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include <stdlib.h>

#include "MemoryMap.hpp"

namespace fail {

bool MemoryMap::readFromFile(char const * const filename)
{
	std::ifstream file(filename);
	if (!file.is_open()) {
		return false;
	}

	std::string buf;
	unsigned guest_addr, guest_len;
	unsigned count = 0;

	while (getline(file, buf)) {
		std::string addr, len;
		std::stringstream ss(buf, std::ios::in);
		ss >> addr >> len;
		guest_addr = strtoul(addr.c_str(), NULL, 0);
		guest_len = strtoul(len.c_str(), NULL, 0);
		add(guest_addr, guest_len);
		count++;
	}

	// assertion kept from original code; usually something's fishy if the file
	// contains no entries
	assert(count > 0);
	return true;
}

bool MemoryMap::writeToFile(char const * const filename)
{
	std::ofstream file(filename);
	if (!file.is_open()) {
		return false;
	}

#ifndef __puma
	for (address_set::iterator it = as.begin();
		it != as.end(); ++it) {
		file << it->lower() << " " << (it->upper() - it->lower() + 1) << "\n";
	}
#endif

	return true;
}

}
