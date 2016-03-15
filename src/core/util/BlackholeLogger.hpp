#ifndef __BLACKHOLE_LOGGER_HPP__
#define __BLACKHOLE_LOGGER_HPP__

#include <iostream>

namespace fail {

/**
 * \class BlackholeLogger
 * A /dev/null sink as a drop-in replacement for Logger.  Should be completely
 * optimized away on non-trivial optimization levels.
 */
class BlackholeLogger {
public:
	Logger(const std::string& description = "FAIL*", bool show_time = true,
		   std::ostream& dest = std::cout) { }
	void setDescription(const std::string& descr) { }
	void showTime(bool choice) { }
	template<class T>
	inline std::ostream& operator <<(const T& v) { }
};

} // end-of-namespace: fail

#endif // __BLACKHOLE_LOGGER_HPP__
