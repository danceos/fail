#ifndef __GEM5_CONTROLLER_HPP__
  #define __GEM5_CONTROLLER_HPP__

#include "../SimulatorController.hpp"
#include "Gem5Memory.hpp"

#include "sim/system.hh"

namespace fail {

/**
 * \class Gem5Controller
 * 
 * Gem5-specific implementation of a SimulatorController.
 */
class Gem5Controller : public SimulatorController {
public:
	void startup();
	~Gem5Controller();

	bool save(const std::string &path);
	void restore(const std::string &path);
	void reboot();
};

} // end-of-namespace: fail

#endif // __GEM5_CONTROLLER_HPP__
