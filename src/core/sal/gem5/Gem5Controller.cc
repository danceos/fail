#include "Gem5Controller.hpp"

#include <fstream>

#include "sim/core.hh"
#include "sim/sim_exit.hh"
//#include "sim/root.hh"

namespace fail {

int     interrupt_to_fire           = -1;

void Gem5Controller::save(const std::string &path)
{
	// Takes a snapshot in the m5out dir
	Tick when = curTick() + 1;
	exitSimLoop("checkpoint", 0, when, 0);
	
	// This could be a version to take snapshots with a specified name
	/*Root* root = Root::root();

	std::ofstream file(path.c_str()); 
	root->serialize(file);
	file.close();*/
}

void Gem5Controller::restore(const std::string &path)
{

}

void Gem5Controller::reboot()
{

}

} // end-of-namespace: fail
