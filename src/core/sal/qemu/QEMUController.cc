#include <sstream>

#include "QEMUController.hpp"
#include "QEMUMemory.hpp"
#include "QEMURegister.hpp"
#include "../Register.hpp"
#include "../SALInst.hpp"

namespace fail {

QEMUController::QEMUController()
	: SimulatorController(new QEMURegisterManager(), new QEMUMemoryManager()),
	  m_cpuenv(0)
{
	// TODO: probably do additional RegisterManager initializations
}

QEMUController::~QEMUController()
{
	delete m_Regs;
	delete m_Mem;
}

} // end-of-namespace: fail
