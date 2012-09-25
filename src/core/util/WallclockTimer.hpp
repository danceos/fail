/** 
 * \brief The WallclockTimer measures the elapsed time
 * 
 * The WallclockTimer measures the time which is elapsed between start and stop of the timer.
 */

#ifndef __WALLCLOCKTIMER_HPP__
  #define __WALLCLOCKTIMER_HPP__

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
	struct timeval start,end,current;
	
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
	std::string getRuntimeAsString();
	/**
	 *	Returns the elapsed time as double. This works while the timer is running, and if it is stopped.
	 */
	double getRuntimeAsDouble();
	/**
	 *	Stops the timer.
	 */
	void stopTimer();
	/**
	 *	Resets the timer. The timer is after a call of reset stopped.
	 */
	void reset();
	
	
};

} // end-of-namespace: fail

#endif // __WALLCLOCKTIMER_HPP__
