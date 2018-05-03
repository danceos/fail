#ifndef __QEMU_CONTROLLER_HPP__
#define __QEMU_CONTROLLER_HPP__

#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "../SimulatorController.hpp"

struct CPUX86State;

namespace fail {

class ExperimentFlow;
class TimerListener;

/**
 * \class QEMUController
 * Very rudimentary, QEMU-specific implementation of a SimulatorController.
 */
class QEMUController : public SimulatorController {
public:
	CPUX86State *m_cpu0env;

	// Initialize the controller.
	QEMUController();
	~QEMUController();
	/**
	 * I/O port communication handler. This method is called from QEMU.
	 * @param data the data transmitted
	 * @param port the port it was transmitted on
	 * @param out true if the I/O traffic has been outbound, false otherwise
	 * FIXME Should this be part of the generic interface?  Inherited from some generic x86 arch class?
	 * FIXME Access width should be part of the interface.
	 * FIXME Read/Write should be separate listeners.
	 */
	void onIOPort(unsigned char data, unsigned port, bool out);
	/**
	 * Internal handler for TimerListeners.
	 */
	void onTimerTrigger(TimerListener *pli);
	/* ********************************************************************
	 * Simulator Controller & Access API:
	 * ********************************************************************/
	/**
	 * Save simulator state.  TODO.
	 * @param path Location to store state information
	 * @return \c true if the state has been successfully saved, \c false otherwise
	 */
	bool save(const std::string& path) { return false; }
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
	void setCPUEnv(struct CPUX86State *env) { m_cpu0env = env; }
};

} // end-of-namespace: fail

#endif // __QEMU_CONTROLLER_HPP__
