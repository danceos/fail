#include <fstream>
#include <string>
#include <sstream>
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
		std::stringstream ss(buf, std::ios::in);
		ss >> guest_addr >> guest_len;
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

	for (iterator it = begin(); it != end(); ++it) {
		file << *it << "\t1\n";
	}

	return true;
}

}
