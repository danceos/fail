#ifndef __L4SYS_CONVERSION_HPP__
  #define __L4SYS_CONVERSION_HPP__

#include "l4sys.pb.h"

class L4SysConversion {
public:
	L4SysConversion(char const **strings, size_t array_size)
	: m_Strings(strings)
	{
		m_ElementCount = array_size / sizeof(char*);
	}
	char const * output(unsigned int res) {
		if (res == 0 || res >= m_ElementCount) {
			return m_Strings[0];
		} else {
			return m_Strings[res];
		}
	}
private:
	char const **m_Strings;
	size_t m_ElementCount;
};

#endif // __L4SYS_CONVERSION_HPP__
