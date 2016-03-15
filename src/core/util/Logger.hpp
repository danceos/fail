#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <iostream>
#include <sstream>

namespace fail {

/**
 * \class Logger
 * Provides logging mechanisms.
 */
class Logger {
private:
	std::ostream* m_pDest;
	std::string m_description;
	bool m_showTime;
	void timestamp();
public:
	/**
	 * Constructor.
	 * @param description Description shown alongside each log entry in
	 *        square brackets [ ].  Can be overridden in single add() calls.
	 * @param show_time Show a timestamp with each log entry.
	 * @param dest Stream to log into.
	 */
	Logger(const std::string& description = "FAIL*", bool show_time = true,
		   std::ostream& dest = std::cout)
	 : m_pDest(&dest), m_description(description), m_showTime(show_time) { }
	/**
	 * Change the default description which is shown alongside each log
	 * entry in square brackets [ ].
	 * @param descr The description text.
	 */
	void setDescription(const std::string& descr) { m_description = descr; }
	/**
	 * Change the default option of show_time which shows a timestamp
	 * each log entry.
	 * @param choice The choice for show_time
	 */
	void showTime(bool choice) { m_showTime = choice; }
	/**
	 * Add a new log entry.
	 * @param v data to log
	 * @return a \c std::ostream reference to continue streaming a longer log entry
	 */
	template<class T>
	inline std::ostream& operator <<(const T& v) { timestamp(); return (*m_pDest) << v; }
};

} // end-of-namespace: fail

#endif // __LOGGER_HPP__
