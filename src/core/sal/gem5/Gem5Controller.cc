#include "Gem5Controller.hpp"
#include "Gem5Connector.hpp"

#include "../Listener.hpp"

namespace fail {

bool Gem5Controller::save(const std::string &path)
{
	connector.save(path);

	return true;
}

void Gem5Controller::restore(const std::string &path)
{
	connector.restore(path);
}

// TODO: Implement reboot
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
