#include "Gem5Controller.hpp"
//#include "Gem5Connector.hpp"

#include "../Listener.hpp"

#include "Gem5Wrapper.hpp"

namespace fail {

void Gem5Controller::startup()
{
	// Assuming there is only one defined system should be sufficient for most cases. More systems
	// are only used for switching cpu model or caches during a simulation run.
	m_System = GetSystemObject();
	m_Mem = new Gem5MemoryManager(m_System);

	int numCtxs = GetNumberOfContexts(m_System);
	for (int i = 0; i < numCtxs; i++) {
		ConcreteCPU* cpu = new ConcreteCPU(GetCPUId(m_System, i), m_System);
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
	delete m_Mem;
}

bool Gem5Controller::save(const std::string &path)
{
//	connector.save(path); // FIXME: not working?!

	return true;
}

void Gem5Controller::restore(const std::string &path)
{
//	connector.restore(path); // FIXME: not working?!
}

// TODO: Implement reboot
void Gem5Controller::reboot()
{

}

} // end-of-namespace: fail
