#ifndef __DWARFREADER_HPP__
#define __DWARFREADER_HPP__

#include <string>
#include <ostream>
#include <list>

namespace fail {


/**
	* This source code is based on bcov 0.2.
	* Sourcefile: src/coverage.cpp
	* http://bcov.sourceforge.net/
	* GNU GENERAL PUBLIC LICENSE
*/

	struct addrToLine {
		unsigned absoluteAddr;
		unsigned lineNumber;
		std::string lineSource;
	};

	/**
	* \class DwarfReader
	* ToDO
	*/

	class DwarfReader {

		public:

		bool read_source_files(const std::string& fileName, std::list<std::string>& lines);
		bool read_mapping(std::string fileName, std::list<addrToLine>& addrToLineList);
};

} // end-of-namespace fail

#endif //__DwarfREADER_HPP__

