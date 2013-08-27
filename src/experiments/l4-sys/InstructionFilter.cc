#include "InstructionFilter.hpp"
#include "util/Logger.hpp"
#include <fstream>

// for more complex filters yet to come

/**
 * Determine how many bytes are left in this file.
 */
static inline unsigned
getFileBytesLeft(std::ifstream& f)
{
	// get pos, seek to end, return difference
	std::streampos pos = f.tellg();
	f.seekg(0, std::ios::end);
	std::streampos end = f.tellg();
	
	// and don't forget to reset the seek pos
	f.seekg(pos, std::ios::beg);

	return end - pos;
}


RangeSetInstructionFilter::RangeSetInstructionFilter(char const *filename)
	: _filters()
{
	fail::Logger log("RangeSet", false);

	std::ifstream filterFile(filename, std::ios::in);
	if (!filterFile.is_open()) {
		log << filename << " not found" << std::endl;
		return;
	}
	
	while (getFileBytesLeft(filterFile) > 1) {
		address_t start, end;
		filterFile >> std::hex >> start;
		filterFile >> std::hex >> end;
		log << "new filter range: " << std::hex
			<< start << " - " << end << std::endl;
		addFilter(start, end);
	}
}
