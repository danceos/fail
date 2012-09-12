#ifndef __QEMU_CONTROLLER_HPP__
  #define __QEMU_CONTROLLER_HPP__

#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "../SimulatorController.hpp"
#include "../Listener.hpp"

struct CPUX86State;

namespace fail {

class ExperimentFlow;

/**
 * \class QEMUController
 * Very rudimentary, QEMU-specific implementation of a SimulatorController.
 */
class QEMUController : public SimulatorController {
public:
	CPUX86State *m_cpuenv;

	// Initialize the controller.
	QEMUController();
	~QEMUController();
	/**
	 * I/O port communication handler. This method is called from QEMU.  TODO.
	 * @param data the data transmitted
	 * @param port the port it was transmitted on
	 * @param out true if the I/O traffic has been outbound, false otherwise
	 */
	void onIOPort(unsigned char data, unsigned port, bool out) {}
	/**
	 * Static internal handler for TimerListeners.  TODO.
	 */
	static void onTimerTrigger(void *thisPtr) {}
	/* ********************************************************************
	 * Simulator Controller & Access API:
	 * ********************************************************************/
	/**
	 * Save simulator state.  TODO.
	 * @param path Location to store state information
	 */
	void save(const std::string& path) {}
	/**
	 * Restore simulator state. Clears all Listeners.  TODO.
	 * @param path Location to previously saved state information
	 */
	void restore(const std::string& path) {}
	/**
	 * Reboot simulator. Clears all Listeners.  TODO.
	 */
	void reboot() {}
	/* internal, QEMU-specific stuff */
	void setCPUEnv(struct CPUX86State *env) { m_cpuenv = env; }
};

} // end-of-namespace: fail

#endif // __QEMU_CONTROLLER_HPP__
