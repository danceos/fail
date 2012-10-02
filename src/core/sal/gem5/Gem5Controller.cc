#include "Gem5Controller.hpp"

#include <fstream>

#include "sim/core.hh"
#include "sim/sim_exit.hh"
//#include "sim/root.hh"

namespace fail {

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

void Gem5Controller::onBreakpoint(address_t instrPtr, address_t address_space)
{
	bool do_fire = false;
	// Check for active breakpoint-events:
	ListenerManager::iterator it = m_LstList.begin();
	BPEvent tmp(instrPtr, address_space);
	while (it != m_LstList.end()) {
		BaseListener* pLi = *it;
		BPListener* pBreakpt = dynamic_cast<BPListener*>(pLi);
		if (pBreakpt != NULL && pBreakpt->isMatching(&tmp)) {
			pBreakpt->setTriggerInstructionPointer(instrPtr);
			it = m_LstList.makeActive(it);
			do_fire = true;
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		it++;
	}
	if (do_fire)
		m_LstList.triggerActiveListeners();
}

} // end-of-namespace: fail
