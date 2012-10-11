#include <sstream>

#include "T32Controller.hpp"
#include "T32Memory.hpp"
#include "T32Register.hpp"
#include "../Register.hpp"
#include "../SALInst.hpp"

namespace fail {

T32Controller::T32Controller()
	: SimulatorController(new T32RegisterManager(), new T32MemoryManager())
{
	// TODO: probably do additional RegisterManager initializations
}

T32Controller::~T32Controller()
{
	delete m_Regs;
	delete m_Mem;
}


} // end-of-namespace: fail
