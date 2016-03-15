#include "SimulatorController.hpp"
#include "SALInst.hpp"
#include "Event.hpp"
#include "Listener.hpp"
#include "util/CommandLine.hpp"

namespace fail {

// External reference declared in SALInst.hpp
ConcreteSimulatorController simulator;

bool SimulatorController::addListener(BaseListener* li)
{
	assert(li != NULL && "FATAL ERROR: Argument (ptr) cannot be NULL!");
	// If addListener() was called from onTrigger(), there is no parent
	// to retrieve/assign. In this case, we simple expect the parent-member
	// to be valid ('as is'). (Otherwise, the current flow is retrieved by
	// calling CoroutineManager::getCurrent().)
	ExperimentFlow* pFlow = m_Flows.getCurrent();
	if (pFlow == CoroutineManager::SIM_FLOW)
		pFlow = li->getParent();
	m_LstList.add(li, pFlow);
	// Call the common postprocessing function:
	if (!li->onAddition()) { // If the return value signals "false"...,
		m_LstList.remove(li); // ...skip the addition
		return false;
	}
	return true;
}

BaseListener* SimulatorController::resume(void)
{
	m_Flows.resume();
	assert(m_LstList.getLastFired() != NULL &&
		   "FATAL ERROR: getLastFired() expected to be non-NULL!");
	return m_LstList.getLastFired();
}

void SimulatorController::startup(int& argc, char **& argv)
{
	if (argv) {
		CommandLine::Inst().collect_args(argc, argv);
	}
	startup();
}

void SimulatorController::startup()
{
	// Some greetings to the user:
	std::cout << "[SimulatorController] Initializing..." << std::endl;
	// TODO: Log-Level?

	// Set FAIL* as initialized
	m_isInitialized = true;

	// Activate previously added experiments to allow initialization:
	initExperiments();
}

void SimulatorController::initExperiments()
{
	/* empty. */
}

void SimulatorController::onBreakpoint(ConcreteCPU* cpu, address_t instrPtr, address_t address_space)
{
	// Check for active breakpoint-events:
	ListenerManager::iterator it = m_LstList.begin();
	BPEvent tmp(instrPtr, address_space, cpu);
	while (it != m_LstList.end()) {
		BaseListener* pLi = *it;
		BPListener* pBreakpt = dynamic_cast<BPListener*>(pLi);
		if (pBreakpt != NULL && pBreakpt->isMatching(&tmp)) {
			pBreakpt->setTriggerCPU(cpu);
			pBreakpt->setTriggerInstructionPointer(instrPtr);
			it = m_LstList.makeActive(it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		it++;
	}
	m_LstList.triggerActiveListeners();
}

void SimulatorController::onMemoryAccess(ConcreteCPU* cpu, address_t addr, size_t len,
	bool is_write, address_t instrPtr)
{
	MemAccessEvent::access_type_t accesstype =
		is_write ? MemAccessEvent::MEM_WRITE
		         : MemAccessEvent::MEM_READ;

	MemAccessEvent tmp(addr, len, instrPtr, accesstype, cpu);
	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) { // check for active listeners
		BaseListener* pev = *it;
		MemAccessListener* ev = dynamic_cast<MemAccessListener*>(pev);
		// Is this a MemAccessListener? Correct access type?
		if (!ev || !ev->isMatching(&tmp)) {
			++it;
			continue; // skip listener activation
		}
		ev->setTriggerAddress(addr);
		ev->setTriggerWidth(len);
		ev->setTriggerInstructionPointer(instrPtr);
		ev->setTriggerAccessType(accesstype);
		ev->setTriggerCPU(cpu);
		it = m_LstList.makeActive(it);
	}
	m_LstList.triggerActiveListeners();
}

void SimulatorController::onInterrupt(ConcreteCPU* cpu, unsigned interruptNum, bool nmi)
{
	ListenerManager::iterator it = m_LstList.begin();
	InterruptEvent tmp(nmi, interruptNum, cpu);
	while (it != m_LstList.end()) { // check for active listeners
		BaseListener* pev = *it;
		InterruptListener* pie = dynamic_cast<InterruptListener*>(pev);
		if (!pie || !pie->isMatching(&tmp)) {
			++it;
			continue; // skip listener activation
		}
		pie->setTriggerNumber(interruptNum);
		pie->setNMI(nmi);
		pie->setTriggerCPU(cpu);
		it = m_LstList.makeActive(it);
	}
	m_LstList.triggerActiveListeners();
}

void SimulatorController::onTrap(ConcreteCPU* cpu, unsigned trapNum)
{
	TroubleEvent tmp(trapNum, cpu);
	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) { // check for active listeners
		BaseListener* pev = *it;
		TrapListener* pte = dynamic_cast<TrapListener*>(pev);
		if (!pte || !pte->isMatching(&tmp)) {
			++it;
			continue; // skip listener activation
		}
		pte->setTriggerNumber(trapNum);
		pte->setTriggerCPU(cpu);
		it = m_LstList.makeActive(it);
	}
	m_LstList.triggerActiveListeners();
}

void SimulatorController::onGuestSystem(char data, unsigned port)
{
	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) { // check for active listeners
		BaseListener* pev = *it;
		GuestListener* pge = dynamic_cast<GuestListener*>(pev);
		if (pge != NULL) {
			pge->setData(data);
			pge->setPort(port);
			it = m_LstList.makeActive(it);
			continue; // dito.
		}
		++it;
	}
	m_LstList.triggerActiveListeners();
}

void SimulatorController::onJump(ConcreteCPU* cpu, bool flagTriggered, unsigned opcode)
{
	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) { // check for active listeners
		JumpListener* pje = dynamic_cast<JumpListener*>(*it);
		if (pje != NULL) {
			pje->setOpcode(opcode);
			pje->setFlagTriggered(flagTriggered);
			pje->setTriggerCPU(cpu);
			it = m_LstList.makeActive(it);
			continue; // dito.
		}
		++it;
	}
	m_LstList.triggerActiveListeners();
}

void SimulatorController::addCPU(ConcreteCPU* cpu)
{
	assert(cpu != NULL && "FATAL ERROR: Argument (cpu) cannot be NULL!");
	m_CPUs.push_back(cpu);
}

ConcreteCPU& SimulatorController::getCPU(size_t i) const
{
	assert(i < m_CPUs.size() && "FATAL ERROR: Invalid index provided!");
	return *m_CPUs[i];
}

void SimulatorController::addFlow(ExperimentFlow* flow)
{
	// Store the (flow,corohandle)-tuple internally and create its coroutine:
	m_Flows.create(flow);
	// let it run for the first time
	m_Flows.toggle(flow);
}

void SimulatorController::removeFlow(ExperimentFlow* flow)
{
	// remove all remaining listeners of this flow
	clearListeners(flow);
	// remove coroutine
	m_Flows.remove(flow);
}

BaseListener* SimulatorController::addListenerAndResume(BaseListener* li)
{
	addListener(li);
	return resume();
}

void SimulatorController::terminate(int exCode)
{
	// Attention: This could cause problems, e.g., because of non-closed sockets
	std::cout << "[FAIL] Exit called by experiment with exit code: " << exCode << std::endl;
	// TODO: (Non-)Verbose-Mode? Log-Level?

	m_Flows.setTerminated(); // we are about to terminate
	exit(exCode);
}

} // end-of-namespace: fail
