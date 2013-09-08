#ifndef __FORMATCONVERTER_HPP__
#define __FORMATCONVERTER_HPP__

#include <istream>
#include "util/ProtoStream.hpp"

class FormatConverter {
public:
	FormatConverter(std::istream& input, fail::ProtoOStream& ps) : m_input(input), m_ps(ps) {}
	virtual bool convert() = 0;

protected:
	std::istream& m_input;
	fail::ProtoOStream& m_ps;
};
#endif
