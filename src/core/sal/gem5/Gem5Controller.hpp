#ifndef __GEM5_CONTROLLER_HPP__
  #define __GEM5_CONTROLLER_HPP__

#include "../SimulatorController.hpp"

namespace fail {

extern int interrupt_to_fire;

class Gem5Controller : public SimulatorController {
public:
	virtual void save(const std::string &path);
	virtual void restore(const std::string &path);
	virtual void reboot();
};

} // end-of-namespace: fail

#endif // __GEM_CONTROLLER_HPP__
