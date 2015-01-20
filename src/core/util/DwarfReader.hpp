#ifndef __DWARFREADER_HPP__
#define __DWARFREADER_HPP__

#include <string>
#include <ostream>
#include <list>

namespace fail {

// temporary wrapper object for (file, linenumber)
class SourceLine {
public:
	std::string source_file;
	unsigned line_number;

	SourceLine(std::string source, unsigned line) : source_file(source), line_number(line) {}
};

// wrapper object for insertion into DB
class DwarfLineMapping {
public:
	unsigned absolute_addr;
	unsigned line_range_size;
	unsigned line_number;
	std::string line_source;

	DwarfLineMapping(unsigned addr, unsigned size, unsigned number, std::string src)
		: absolute_addr(addr), line_range_size(size), line_number(number), line_source(src){}
};

/**
	* This source code is based on bcov 0.2.
	* Sourcefile: src/coverage.cpp
	* http://bcov.sourceforge.net/
	* GNU GENERAL PUBLIC LICENSE
*/

/**
* \class DwarfReader
* ToDO
*/
class DwarfReader {

		public:

		bool read_source_files(const std::string& fileName, std::list<std::string>& lines);
		bool read_mapping(std::string fileName, std::list<DwarfLineMapping>& lineMapping);
};

} // end-of-namespace fail

#endif //__DwarfREADER_HPP__

