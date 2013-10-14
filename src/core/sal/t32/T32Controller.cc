#include "T32Controller.hpp"
#include "../Listener.hpp"

namespace fail {

void T32Controller::startup(){
	// Do some T32-specific startup
	addCPU(new ConcreteCPU(0));
	// Startup generic SimulatorController
	// TODO pass on command-line parameters
	SimulatorController::startup();
}


T32Controller::~T32Controller()
{
	std::vector<ConcreteCPU*>::iterator it = m_CPUs.begin();
	while (it != m_CPUs.end()) {
		delete *it;
		it = m_CPUs.erase(it);
	}
}


} // end-of-namespace: fail
