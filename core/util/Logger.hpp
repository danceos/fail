#ifndef __LOGGER_HPP__
  #define __LOGGER_HPP__

// Author: Adrian Böckenkamp
// Date:   21.11.2011

#include <iostream>
#include <sstream>

/**
 * \class Logger
 * Provides logging mechanisms.
 */
class Logger
{
	private:
		std::ostream* m_pDest;
		std::string m_description;
		bool m_showTime;
	public:
		/**
		 * Constructor.
		 * @param description Description shown alongside each log entry in
		 *        square brackets [ ].  Can be overridden in single add() calls.
		 * @param show_time Show a timestamp with each log entry.
		 * @param dest Stream to log into.
		 */
		Logger(const std::string& description = "Fail*", bool show_time = true,
			   std::ostream& dest = std::cout) 
		 : m_pDest(&dest), m_description(description), m_showTime(show_time) { }
		/**
		 * Change the default description which is shown alongside each log
		 * entry in square brackets [ ].
		 * @param descr The description text.
		 */
		void setDescription(const std::string& descr)
		{
			m_description = descr;
		}
		/**
		 * Add a new log entry.
		 * @param what Message for log entry.
		 * @param descr Description shown alongside this log entry in square
		 *        brackets [ ].
		 */
		void add(const std::string& what, const std::string& descr);
		/**
		 * Add a new log entry.
		 * @param what Message for log entry.  The default description is
		 *        being used in square brackets [ ].
		 */
		void add(const std::string& what) { add(what, m_description); }
		/**
		 * Simplifies the logging.
		 * @param v data to log
		 */
		template<class T>
		inline std::ostream& operator <<(const T& v)
		{
			std::stringstream ss;
			ss << v;
			add(ss.str());
			return (*m_pDest);
		}
};

#endif /* __LOGGER_HPP__ */
