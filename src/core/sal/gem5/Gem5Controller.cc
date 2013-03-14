#include "Gem5Controller.hpp"
#include "Gem5Connector.hpp"

#include "../Listener.hpp"

#include "sim/system.hh"

namespace fail {

void Gem5Controller::startup()
{
	// Assuming there is only one defined system should be sufficient for most cases. More systems
	// are only used for switching cpu model or caches during a simulation run.
	System* sys = System::systemList.front();
	m_Mem = new Gem5MemoryManager(sys);

	for (int i = 0; i < sys->numContexts(); i++) {
		ConcreteCPU* cpu = new ConcreteCPU(sys->getThreadContext(i)->cpuId(), System::systemList.front());
		addCPU(cpu);
	}

	// TODO pass on command-line parameters
	SimulatorController::startup();
}

Gem5Controller::~Gem5Controller()
{
	std::vector<ConcreteCPU*>::iterator it = m_CPUs.begin();
	while (it != m_CPUs.end()) {
		delete *it;
		it = m_CPUs.erase(it);
	}
}

bool Gem5Controller::save(const std::string &path)
{
	connector.save(path); // FIXME: not working?!

	return true;
}

void Gem5Controller::restore(const std::string &path)
{
	connector.restore(path); // FIXME: not working?!
}

// TODO: Implement reboot
void Gem5Controller::reboot()
{

}

} // end-of-namespace: fail
