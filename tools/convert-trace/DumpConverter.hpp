#ifndef __DUMPCONVERTER_HPP__
#define __DUMPCONVERTER_HPP__

#include "FormatConverter.hpp"

class DumpConverter : public FormatConverter {
public:
	DumpConverter(std::istream& input, fail::ProtoOStream& ps) : FormatConverter(input, ps) {}
	bool convert();
};
#endif
