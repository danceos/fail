#ifndef __GEM5CONVERTER_HPP__
#define __GEM5CONVERTER_HPP__

#include "FormatConverter.hpp"

class Gem5Converter : public FormatConverter {
public:
	Gem5Converter(std::istream& input, fail::ProtoOStream& ps) : FormatConverter(input, ps) {}
	bool convert();
};
#endif
