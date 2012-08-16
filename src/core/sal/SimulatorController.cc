#include "SimulatorController.hpp"
#include "SALInst.hpp"
#include "Event.hpp"

namespace fail {

// External reference declared in SALInst.hpp
ConcreteSimulatorController simulator;

bool SimulatorController::addListener(BaseListener* li)
{
	assert(li != NULL && "FATAL ERROR: Argument (ptr) cannot be NULL!");
	m_LstList.add(li, m_Flows.getCurrent());
	// Call the common postprocessing function:
	if (!li->onAddition()) { // If the return value signals "false"...,
		m_LstList.remove(li); // ...skip the addition
		return false;
	}
	return true;
}

BaseListener* SimulatorController::resume(void)
{
	if (!hasListeners())
		return NULL;
	m_Flows.resume();
	assert(m_LstList.getLastFired() != NULL &&
		   "FATAL ERROR: getLastFired() expected to be non-NULL!");
	return m_LstList.getLastFired();
}

void SimulatorController::startup()
{
	// Some greetings to the user:
	std::cout << "[SimulatorController] Initializing..." << std::endl;
	// TODO: Log-Level?
	
	// Activate previously added experiments to allow initialization:
	initExperiments();
}

void SimulatorController::initExperiments()
{
	/* empty. */
}

void SimulatorController::onBreakpoint(address_t instrPtr, address_t address_space)
{
	assert(false &&
	"FIXME: SimulatorController::onBreakpoint() has not been tested before");
	// FIXME: Improve performance!

	// Loop through all listeners of type BP*Listener:
	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) {
		BaseListener* pev = *it;
		BPSingleListener* pbp; BPRangeListener* pbpr;
		if ((pbp = dynamic_cast<BPSingleListener*>(pev)) && pbp->isMatching(instrPtr, address_space)) {
			 pbp->setTriggerInstructionPointer(instrPtr);
			it = m_LstList.makeActive(it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		} else if ((pbpr = dynamic_cast<BPRangeListener*>(pev)) &&
		           pbpr->isMatching(instrPtr, address_space)) {
			pbpr->setTriggerInstructionPointer(instrPtr);
			it = m_LstList.makeActive(it);
			continue; // dito
		}
		++it;
	}
	m_LstList.triggerActiveListeners();
}

void SimulatorController::onMemoryAccess(address_t addr, size_t len,
                                         bool is_write, address_t instrPtr)
{
	// FIXME: Improve performance!
	MemAccessEvent::access_type_t accesstype =
		is_write ? MemAccessEvent::MEM_WRITE
		         : MemAccessEvent::MEM_READ;

	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) { // check for active listeners
		BaseListener* pev = *it;
		MemAccessListener* ev = dynamic_cast<MemAccessListener*>(pev);
		// Is this a MemAccessListener? Correct access type?
		if (!ev || !ev->isMatching(addr, len, accesstype)) {
			++it;
			continue; // skip listener activation
		}
		ev->setTriggerAddress(addr);
		ev->setTriggerWidth(len);
		ev->setTriggerInstructionPointer(instrPtr);
		ev->setTriggerAccessType(accesstype);
		it = m_LstList.makeActive(it);
	}
	m_LstList.triggerActiveListeners();
}

void SimulatorController::onInterrupt(unsigned interruptNum, bool nmi)
{
	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) { // check for active listeners 
		BaseListener* pev = *it;
		InterruptListener* pie = dynamic_cast<InterruptListener*>(pev);
		if (!pie || !pie->isMatching(interruptNum)) {
			++it;
			continue; // skip listener activation
		}
		pie->setTriggerNumber(interruptNum);
		pie->setNMI(nmi);
		it = m_LstList.makeActive(it);
	}
	m_LstList.triggerActiveListeners();
}

bool SimulatorController::isSuppressedInterrupt(unsigned interruptNum)
{
	for (size_t i = 0; i < m_SuppressedInterrupts.size(); i++)
		if ((m_SuppressedInterrupts[i] == interruptNum ||
		    m_SuppressedInterrupts[i] == ANY_INTERRUPT) &&
		    interruptNum != (unsigned)interrupt_to_fire + 32) {
				if ((int)interruptNum == interrupt_to_fire + 32) {
					interrupt_to_fire = -1;
					return true;
				}
			return true;
		}
	return false;
}

bool SimulatorController::addSuppressedInterrupt(unsigned interruptNum)
{
	// Check if already existing:
	if (isSuppressedInterrupt(interruptNum+32))
		return false; // already added: nothing to do here
		
	if (interruptNum == ANY_INTERRUPT) {
		m_SuppressedInterrupts.push_back(interruptNum);
		return true;
	} else {
		m_SuppressedInterrupts.push_back(interruptNum+32);
		return true;
	}
}

bool SimulatorController::removeSuppressedInterrupt(unsigned interruptNum)
{
	for (size_t i = 0; i < m_SuppressedInterrupts.size(); i++) {	
		if (m_SuppressedInterrupts[i] == interruptNum+32 ||
		    m_SuppressedInterrupts[i] == ANY_INTERRUPT)
			m_SuppressedInterrupts.erase(m_SuppressedInterrupts.begin() + i);
			return true;
	}
	return false;
}

void SimulatorController::onTrap(unsigned trapNum)
{
	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) { // check for active listeners
		BaseListener* pev = *it;
		TrapListener* pte = dynamic_cast<TrapListener*>(pev);
		if (!pte || !pte->isMatching(trapNum)) {
			++it;
			continue; // skip listener activation
		}
		pte->setTriggerNumber(trapNum);
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

void SimulatorController::onJump(bool flagTriggered, unsigned opcode)
{
	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) { // check for active listeners
		JumpListener* pje = dynamic_cast<JumpListener*>(*it);
		if (pje != NULL) {
			pje->setOpcode(opcode);
			pje->setFlagTriggered(flagTriggered);
			it = m_LstList.makeActive(it);
			continue; // dito.
		}
		++it;
	}
	m_LstList.triggerActiveListeners();
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
	exit(exCode);
}

} // end-of-namespace: fail
