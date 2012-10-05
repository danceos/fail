#ifndef __GEM5_CONTROLLER_HPP__
  #define __GEM5_CONTROLLER_HPP__

#include <string>

#include "../SimulatorController.hpp"

namespace fail {

class Gem5Controller : public SimulatorController {
public:
	void onBreakpoint(address_t instrPtr, address_t address_space);

	bool save(const std::string &path);
	void restore(const std::string &path);
	void reboot();
};

} // end-of-namespace: fail

#endif // __GEM_CONTROLLER_HPP__
