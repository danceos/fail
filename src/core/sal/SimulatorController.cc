#include "SimulatorController.hpp"
#include "SALInst.hpp"

namespace fail {

// External reference declared in SALInst.hpp
ConcreteSimulatorController simulator;

event_id_t SimulatorController::addEvent(BaseEvent* ev)
{
	assert(ev != NULL && "FATAL ERROR: Argument (ptr) cannot be NULL!");
	event_id_t ret = m_EvList.add(ev, m_Flows.getCurrent());
	// Call the common postprocessing function:
	if (!ev->onEventAddition()) { // If the return value signals "false"...,
		m_EvList.remove(ev); // ...skip the addition
		ret = INVALID_EVENT;
	}
	return ret;
}

BaseEvent* SimulatorController::waitAny(void)
{
	if (!hasEvents())
		return NULL;
	m_Flows.resume();
	assert(m_EvList.getLastFired() != NULL &&
		   "FATAL ERROR: getLastFired() expected to be non-NULL!");
	return m_EvList.getLastFired();
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

void SimulatorController::onBreakpointEvent(address_t instrPtr, address_t address_space)
{
	assert(false &&
	"FIXME: SimulatorController::onBreakpointEvent() has not been tested before");
	// FIXME: Improve performance!

	// Loop through all events of type BP*Event:
	EventManager::iterator it = m_EvList.begin();
	while (it != m_EvList.end()) {
		BaseEvent* pev = *it;
		BPSingleEvent* pbp; BPRangeEvent* pbpr;
		if ((pbp = dynamic_cast<BPSingleEvent*>(pev)) && pbp->isMatching(instrPtr, address_space)) {
			 pbp->setTriggerInstructionPointer(instrPtr);
			it = m_EvList.makeActive(it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		} else if ((pbpr = dynamic_cast<BPRangeEvent*>(pev)) &&
		           pbpr->isMatching(instrPtr, address_space)) {
			pbpr->setTriggerInstructionPointer(instrPtr);
			it = m_EvList.makeActive(it);
			continue; // dito
		}
		++it;
	}
	m_EvList.fireActiveEvents();
}

void SimulatorController::onMemoryAccessEvent(address_t addr, size_t len,
                                              bool is_write, address_t instrPtr)
{
	// FIXME: Improve performance!
	MemAccessEvent::accessType_t accesstype =
		is_write ? MemAccessEvent::MEM_WRITE
		         : MemAccessEvent::MEM_READ;

	EventManager::iterator it = m_EvList.begin();
	while (it != m_EvList.end()) { // check for active events
		BaseEvent* pev = *it;
		MemAccessEvent* ev = dynamic_cast<MemAccessEvent*>(pev);
		// Is this a MemAccessEvent? Correct access type?
		if (!ev || !ev->isMatching(addr, accesstype)) {
			++it;
			continue; // skip event activation
		}
		ev->setTriggerAddress(addr);
		ev->setTriggerWidth(len);
		ev->setTriggerInstructionPointer(instrPtr);
		ev->setTriggerAccessType(accesstype);
		it = m_EvList.makeActive(it);
	}
	m_EvList.fireActiveEvents();
}

void SimulatorController::onInterruptEvent(unsigned interruptNum, bool nmi)
{
	EventManager::iterator it = m_EvList.begin();
	while (it != m_EvList.end()) { // check for active events 
		BaseEvent* pev = *it;
		InterruptEvent* pie = dynamic_cast<InterruptEvent*>(pev);
		if (!pie || !pie->isMatching(interruptNum)) {
			++it;
			continue; // skip event activation
		}
		pie->setTriggerNumber(interruptNum);
		pie->setNMI(nmi);
		it = m_EvList.makeActive(it);
	}
	m_EvList.fireActiveEvents();
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

void SimulatorController::onTrapEvent(unsigned trapNum)
{
	EventManager::iterator it = m_EvList.begin();
	while(it != m_EvList.end()) { // check for active events
		BaseEvent* pev = *it;
		TrapEvent* pte = dynamic_cast<TrapEvent*>(pev);
		if (!pte || !pte->isMatching(trapNum)) {
			++it;
			continue; // skip event activation
		}
		pte->setTriggerNumber(trapNum);
		it = m_EvList.makeActive(it);
	}
	m_EvList.fireActiveEvents();
}

void SimulatorController::onGuestSystemEvent(char data, unsigned port)
{
	EventManager::iterator it = m_EvList.begin();
	while (it != m_EvList.end()) { // check for active events
		BaseEvent* pev = *it;
		GuestEvent* pge = dynamic_cast<GuestEvent*>(pev);
		if (pge != NULL) {
			pge->setData(data);
			pge->setPort(port);
			it = m_EvList.makeActive(it);
			continue; // dito.
		}
		++it;
	}
	m_EvList.fireActiveEvents();
}

void SimulatorController::onJumpEvent(bool flagTriggered, unsigned opcode)
{
	EventManager::iterator it = m_EvList.begin();
	while (it != m_EvList.end()) { // check for active events
		JumpEvent* pje = dynamic_cast<JumpEvent*>(*it);
		if (pje != NULL) {
			pje->setOpcode(opcode);
			pje->setFlagTriggered(flagTriggered);
			it = m_EvList.makeActive(it);
			continue; // dito.
		}
		++it;
	}
	m_EvList.fireActiveEvents();
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
	// remove all remaining events of this flow
	clearEvents(flow);
	// remove coroutine
	m_Flows.remove(flow);
}

BaseEvent* SimulatorController::addEventAndWait(BaseEvent* ev)
{
	addEvent(ev);
	return waitAny();
}

void SimulatorController::terminate(int exCode)
{
	// Attention: This could cause problems, e.g., because of non-closed sockets
	std::cout << "[FAIL] Exit called by experiment with exit code: " << exCode << std::endl;
	// TODO: (Non-)Verbose-Mode? Log-Level?
	exit(exCode);
}

} // end-of-namespace: fail
