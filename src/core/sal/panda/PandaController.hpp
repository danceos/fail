#ifndef __PANDA_CONTROLLER_HPP__
#define __PANDA_CONTROLLER_HPP__

#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "../SimulatorController.hpp"

namespace fail {

class ExperimentFlow;


struct target;

/**
 * \class PandaController
 * Pandaboard-specific implementation of a SimulatorController.
 */
class PandaController : public SimulatorController {
private:
	ExperimentFlow* m_CurrFlow; //!< Stores the current flow for save/restore-operations
public:
	/**
	 * Initialize the controller, i.e., add the number of simulated CPUs.
	 */
	PandaController();
	~PandaController();
	/* ********************************************************************
	 * Standard Listener Handler API:
	 * ********************************************************************/
	/**
	 * Internal handler for TimerListeners. This method is called when a previously
	 * registered timer in openocd main loop triggers. It searches for the
	 * provided TimerListener object within the ListenerManager and fires
	 * such an event by calling * \c triggerActiveListeners().
	 * @param thisPtr a pointer to the TimerListener-object triggered.
	 */
	void onTimerTrigger(void *thisPtr);
	/* ********************************************************************
	 * Simulator Controller & Access API:
	 * ********************************************************************/
	/**
	 * Save simulator state.
	 * @param path Location to store state information
	 * @return \c true if the state has been successfully saved, \c false otherwise
	 */
	bool save(const std::string& path);
	/**
	 * Restore simulator state. Clears all Listeners.
	 * @param path Location to previously saved state information
	 */
	void restore(const std::string& path);
	/**
	 * Reboot simulator. Clears all Listeners.
	 */
	void reboot();
	/**
	 * Fire an interrupt.
	 * @param irq Interrupt to be fired
	 */
	//void fireInterrupt(unsigned irq);

	virtual simtime_t getTimerTicks();

	virtual simtime_t getTimerTicksPerSecond();

	// Overloading super method to terminate OpenOCD properly
	void terminate(int exCode = EXIT_SUCCESS);
};

} // end-of-namespace: fail

#endif // __PANDA_CONTROLLER_HPP__
