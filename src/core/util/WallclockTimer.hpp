/**
 * \brief The WallclockTimer measures the elapsed time
 *
 * The WallclockTimer measures the time which is elapsed between start
 * and stop of the timer.
 */

#ifndef __WALLCLOCKTIMER_HPP__
#define __WALLCLOCKTIMER_HPP__

#include <string>
#include <ostream>
#include <stdlib.h>
#include <sys/time.h>

namespace fail {

/**
 * \class WallclockTimer
 *
 * The class WallclockTimer contains all functions for start,
 * stop, reset and to get the elapsed time.
 */
class WallclockTimer {
private:
	bool m_IsRunning;
	struct timeval m_Start, m_End;
public:
	WallclockTimer() : m_IsRunning(false) { }
	virtual ~WallclockTimer() { }
	/**
	 * Starts the timer.
	 */
	void startTimer();
	/**
	 * Returns the elapsed time as \c std::string. This works while the timer
	 * is running, and if it is stopped.
	 */
	std::string getRuntimeAsString() const;
	/**
	 * Returns the elapsed time as \c double. This works while the timer
	 * is running, and if it is stopped.
	 */
	double getRuntimeAsDouble() const;
	/**
	 * Stops the timer.
	 */
	void stopTimer();
	/**
	 * Resets the timer. The timer is stopped after calling reset().
	 */
	void reset();
	/**
	 * Returns the elapsed time as \c double. This works while the
	 * timer is running, and if it is stopped.
	 */
	operator double() { return getRuntimeAsDouble(); }
	/**
	 * Returns the elapsed time as \c float. This works while the
	 * timer is running, and if it is stopped.
	 */
	operator float() { return (float)getRuntimeAsDouble(); }
	/**
	 * Returns the elapsed time as \c int. This works while the timer
	 * is running, and if it is stopped.
	 */
	operator int() { return (int)getRuntimeAsDouble(); }
};

inline std::ostream& operator<< (std::ostream& os, const WallclockTimer& w)
{ return os << w.getRuntimeAsString().c_str(); }

} // end-of-namespace: fail

#endif // __WALLCLOCKTIMER_HPP__
