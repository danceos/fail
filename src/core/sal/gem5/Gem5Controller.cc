#include "Gem5Controller.hpp"
#include "Gem5Wrapper.hpp"
#include "../Listener.hpp"
#include "base/trace.hh"
#include "sim/root.hh"
#include "sim/core.hh"

#include <fstream>

namespace fail {

void Gem5Controller::startup()
{
	// Assuming there is only one defined system should be sufficient for most cases. More systems
	// are only used for switching cpu model or caches during a simulation run.
	m_System = GetSystemObject();
	m_Mem = new Gem5MemoryManager(m_System);

	restore_request = false;
	restore_path = "";

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
	Serializable::serializeAll(path);

	// Test if save was successful
	std::ifstream save_test;

	save_test.open((path+"/m5.cpt").c_str(), std::ios_base::in);

	if (save_test) {
		save_test.close();
		return true;
	} else {
		save_test.close();
		return false;
	}
}

void Gem5Controller::restore(const std::string& path)
{
	clearListeners();
	restore_request = true;
	restore_path = path;
	m_CurrFlow = m_Flows.getCurrent();
	m_Flows.resume();
}

void Gem5Controller::onRestore()
{
	/* Get the instance of the root-object from gem5
	 * The root object seems to be the root of a tree
	 * that contains all the simulation objects, e.g.
	 * cpu, memory etc.
	 * gem5/src/sim/root.cc,root.hh
	 * */
	Root* root = Root::root();
	/* Checkpoint is a class of gem5 that is used to
	 * manage the built-in save and restore function
	 * of gem5.
	 * gem5/src/sim/serialize.cc,serialize.hh
	 * */
	Checkpoint cp(restore_path);
	/* Set some global variables from checkpoint.
	 * gem5/src/sim/serialize.cc,serialize.hh
	 * */
	Serializable::unserializeGlobals(&cp);
	/* loadStateAll(Checkpoint *cp) is a self-implemented
	 * function that calls loadState() for all simulation
	 * objects. (without the root object).
	 * gem5/src/sim/serialize.cc,serialize.hh
	 * gem5/src/sim/sim_object.cc,sim_object.hh
	 * */
	Serializable::loadStateAll(&cp);
	/* Call loadState() on the root-object.
	 * gem5/src/sim/root.cc,root.hh
	 * */
	root->loadState(&cp);

	restore_request = false;
	m_Flows.toggle(m_CurrFlow);
}

bool Gem5Controller::isRestoreRequest()
{
	return restore_request;
}

// TODO: Implement reboot
void Gem5Controller::reboot()
{

}

simtime_t Gem5Controller::getTimerTicks()
{
	return curTick();
}

simtime_t Gem5Controller::getTimerTicksPerSecond()
{
	return SimClock::Frequency;
}

} // end-of-namespace: fail
