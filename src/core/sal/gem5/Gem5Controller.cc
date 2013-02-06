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

	SimulatorController::startup();
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
