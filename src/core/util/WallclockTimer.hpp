/** 
 * \brief The WallclockTimer measures the elapsed time
 * 
 * The WallclockTimer measures the time which is elapsed between start and stop of the timer.
 */

#ifndef __WALLCLOCKTIMER_HPP__
  #define __WALLCLOCKTIMER_HPP__

#include <string>
#include <stdlib.h>
#include <sys/time.h>


namespace fail {

/**
 * \class WallclockTimer
 * 
 * The class WallclockTimer contains all functions for start, stop, reset and to get the elapsed 
 * time.
 */
class WallclockTimer {
private:

	bool isRunning;
	struct timeval start,end;
	
public:
	WallclockTimer();
	virtual ~WallclockTimer() { }
	/**
	 *	Starts the timer.
	 */
	void startTimer();
	/**
	 *	Returns the elapsed time as string. This works while the timer is running, and if it is stopped.
	 */
	std::string getRuntimeAsString() const;
	/**
	 *	Returns the elapsed time as double. This works while the timer is running, and if it is stopped.
	 */
	double getRuntimeAsDouble() const;
	/**
	 *	Stops the timer.
	 */
	void stopTimer();
	/**
	 *	Resets the timer. The timer is after a call of reset stopped.
	 */
	void reset();
	
	operator double() { return getRuntimeAsDouble(); }
	
	operator int() { return ((int) getRuntimeAsDouble()); }
};

std::ostream& operator<< (std::ostream& os, const WallclockTimer& w);

} // end-of-namespace: fail

#endif // __WALLCLOCKTIMER_HPP__
