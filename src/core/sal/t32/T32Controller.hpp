#ifndef __T32_CONTROLLER_HPP__
	#define __T32_CONTROLLER_HPP__

#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "../SimulatorController.hpp"

namespace fail {

class ExperimentFlow;
class TimerListener;

/**
 * \class T32Controller
 * Very rudimentary, T32-specific implementation of a SimulatorController.
 */
class T32Controller : public SimulatorController {
public:
	// Initialize the controller.
	T32Controller();
	~T32Controller();


	/* ********************************************************************
	 * Simulator Controller & Access API:
	 * ********************************************************************/
	/**
	 * Save simulator state.  Quite hard on real hardware! Also safe all 
	 * HW registers!
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
};

} // end-of-namespace: fail

#endif // __T32_CONTROLLER_HPP__
