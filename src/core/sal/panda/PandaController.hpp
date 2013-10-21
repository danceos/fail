#ifndef __PANDA_CONTROLLER_HPP__
  #define __PANDA_CONTROLLER_HPP__

#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "../SimulatorController.hpp"

struct command_context;

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
	 * I/O port communication handler. This method is called (from
	 * the IOPortCom aspect) every time when pandaboard performs a port I/O operation.
	 * @param cpu the CPU that caused the IO port access
	 * @param data the data transmitted
	 * @param port the port it was transmitted on
	 * @param out \c true if the I/O traffic has been outbound, \c false otherwise
	 */
	void onIOPort(ConcreteCPU* cpu, unsigned char data, unsigned port, bool out);
	/**
	 * Internal handler for TimerListeners. This method is called when a previously
	 * registered (openocd-wrapper) timer triggers. It searches for the provided
	 * TimerListener object within the ListenerManager and fires such an event
	 * by calling \c triggerActiveListeners().
	 * @param thisPtr a pointer to the TimerListener-object triggered
	 */
	void onTimerTrigger(void *thisPtr);
	/* ********************************************************************
	 * Simulator Controller & Access API:
	 * ********************************************************************/
	/**
	 * Save device state.
	 * @param path Location to store state information
	 * @return \c true if the state has been successfully saved, \c false otherwise
	 */
	bool save(const std::string& path);
	/**
	 * Restore device state. Clears all Listeners.
	 * @param path Location to previously saved state information
	 */
	void restore(const std::string& path);
	/**
	 * Reboot pandaboard. Clears all Listeners.
	 */
	void reboot();
	/**
	 * Fire an interrupt.
	 * @param irq Interrupt to be fired
	 */
	//void fireInterrupt(unsigned irq);

	virtual simtime_t getTimerTicks() {
		// return bx_pc_system.time_ticks();
		// ToDo (PORT)
		return 0;
	}
	virtual simtime_t getTimerTicksPerSecond() {
		// return bx_pc_system.time_ticks() / bx_pc_system.time_usec() * 1000000; /* imprecise hack */
		// ToDo (PORT)
		return 0;
	}

	// Overloading super method to terminate OpenOCD properly
	void terminate(int exCode = EXIT_SUCCESS);
};

} // end-of-namespace: fail

#endif // __PANDA_CONTROLLER_HPP__
