#ifndef __SIMULATOR_CONTROLLER_HPP__
  #define __SIMULATOR_CONTROLLER_HPP__

#include <iostream>
#include <string>
#include <cassert>
#include <vector>

#include "efw/CoroutineManager.hpp"
#include "EventList.hpp"
#include "SALConfig.hpp"
#include "Event.hpp"

namespace fail {

// Incomplete types suffice here:
class ExperimentFlow;
class RegisterManager;
class MemoryManager;

/**
 * \class SimulatorController
 *
 * \brief The abstract interface for controlling simulators and
 *        accessing experiment data/flows (as part of the "Simulator
 *        Abstraction Layer".
 *
 * This class manages (1..N) experiments and provides access to the underlying
 * simulator/debugger system. Experiments can enlist arbritrary events
 * (Breakpoint, Memory access, Traps, etc.). The SimulatorController then
 * activates the specific experiment There are further methods to read/write
 * registers and memory, and control the SUT (save/restore/reset).
 */
class SimulatorController {
protected:
	EventList m_EvList; //!< storage where events are being buffered
	CoroutineManager m_Flows; //!< managed experiment flows
	RegisterManager *m_Regs; //!< access to cpu register
	MemoryManager *m_Mem; //!< access to memory pool
	//! list of suppressed interrupts
	std::vector<unsigned> m_SuppressedInterrupts;
	friend class EventList; //!< "outsources" the event management
public:
	SimulatorController()
		: m_Regs(NULL), m_Mem(NULL) { }
	SimulatorController(RegisterManager* regs, MemoryManager* mem)
		: m_Regs(regs), m_Mem(mem) { }
	virtual ~SimulatorController() { }
	/**
	 * @brief Initialization function each implementation needs to call on
	 *        startup
	 *
	 * This function needs to be invoked once the simulator starts, and
	 * allows the SimulatorController to instantiate all needed experiment
	 * components.
	 */
	void startup();
	/**
	 * Experiments need to hook here.
	 */
	void initExperiments();
	/* ********************************************************************
	 * Standard Event Handler API
	 * ********************************************************************/
	/**
	 * Breakpoint event handler. This routine needs to be called in the
	 * simulator specific backend each time a breakpoint event occurs.
	 * @param instrPtr the instruction pointer of the breakpoint event
	 * @param address_space the address space it should occur in
	 */
	void onBreakpointEvent(address_t instrPtr, address_t address_space);
	/**
	 * Memory access event handler (read/write).
	 * @param addr the accessed memory address
	 * @param len the length of the accessed memory
	 * @param is_write \c true if memory is written, \c false if read
	 * @param instrPtr the address of the instruction causing the memory
	 *        access
	 * 
	 * FIXME: should instrPtr be part of this interface?
	 */
	void onMemoryAccessEvent(address_t addr, size_t len,
							 bool is_write, address_t instrPtr);
	/**
	 * Interrupt event handler.
	 * @param interruptNum the interrupt-type id
	 * @param nmi nmi-value from guest-system
	 */
	void onInterruptEvent(unsigned interruptNum, bool nmi);
	/**
	 * Trap event handler.
	 * @param trapNum the trap-type id
	 */
	void onTrapEvent(unsigned trapNum);
	/**
	 * Guest system communication handler.
	 * @param data the "message" from the guest system
	 * @param port the port of the event
	 */
	void onGuestSystemEvent(char data, unsigned port);
	/**
	 * (Conditional) Jump-instruction handler.
	 * @param flagTriggered \c true if the jump was triggered due to a
	 *        specific FLAG (zero/carry/sign/overflow/parity flag)
	 * @param opcode the opcode of the conrecete jump instruction
	 */
	void onJumpEvent(bool flagTriggered, unsigned opcode);
	/**
	 * This method is called when an experiment flow adds a new event by
	 * calling \c simulator.addEvent(pev) or \c simulator.addEventAndWait(pev).
	 * More specifically, the event will be added to the event-list first
	 * (buffer-list, to be precise) and then this event handler is called.
	 * @param pev the event which has been added
	 * @return You should return \c true to continue and \c false to prevent
	 *         the addition of the event \a pev, yielding an error in the
	 *         experiment flow (i.e. -1 is returned).
	 */
	virtual bool onEventAddition(BaseEvent* pev) { return true; }
	/**
	 * This method is called when an experiment flow removes an event from
	 * the event-management by calling \c removeEvent(prev), \c clearEvents()
	 * or by deleting a complete flow (\c removeFlow). More specifically, this
	 * event handler will be called *before* the event is actually deleted.
	 * @param pev the event to be deleted when returning from the event handler
	 */
	virtual void onEventDeletion(BaseEvent* pev) { }
	/**
	 * This method is called when an previously added event is about to be
	 * triggered by the simulator-backend. More specifically, this event handler
	 * will be called *before* the event is actually triggered, i.e. before the
	 * corresponding coroutine is toggled.
	 * @param pev the event to be triggered when returning from the event handler
	 */
	virtual void onEventTrigger(BaseEvent* pev) { }
	/* ********************************************************************
	 * Simulator Controller & Access API:
	 * ********************************************************************/
	/**
	 * Save simulator state.
	 * @param path Location to store state information
	 */
	virtual void save(const std::string& path) = 0;
	/**
	 * Restore simulator state.  Implicitly discards all previously
	 * registered events.
	 * @param path Location to previously saved state information
	 */
	virtual void restore(const std::string& path) = 0;
	/**
	 * Reboot simulator.
	 */
	virtual void reboot() = 0;
	/**
	 * Terminate simulator
	 * @param exCode Individual exit code
	 */
	void terminate(int exCode = EXIT_SUCCESS) __attribute__ ((noreturn));
	/**
	 * Check whether the interrupt should be suppressed.
	 * @param interruptNum the interrupt-type id
	 * @return \c true if the interrupt is suppressed, \c false oterwise
	 */
	bool isSuppressedInterrupt(unsigned interruptNum);
	/**
	 * Add a Interrupt to the list of suppressed.
	 * @param interruptNum the interrupt-type id
	 * @return \c true if sucessfully added, \c false otherwise (already
	 *         existing)
	 */
	bool addSuppressedInterrupt(unsigned interruptNum);
	/**
	 * Remove a Interrupt from the list of suppressed.
	 * @param interruptNum the interrupt-type id
	 * @return \c true if sucessfully removed, \c false otherwise (not
	 *         found)
	 */
	bool removeSuppressedInterrupt(unsigned interruptNum);
	/**
	 * Returns the (constant) initialized register manager.
	 * @return a reference to the register manager
	 */
	RegisterManager& getRegisterManager() { return (*m_Regs); }
	const RegisterManager& getRegisterManager() const { return (*m_Regs); }
	/**
	 * Sets the register manager.
	 * @param pReg the new register manager (or a concrete derived class of
	 *        RegisterManager)
	 */
	void setRegisterManager(RegisterManager* pReg) { m_Regs = pReg; }
	/**
	 * Returns the (constant) initialized memory manager.
	 * @return a reference to the memory manager
	 */
	MemoryManager& getMemoryManager() { return (*m_Mem); }
	const MemoryManager& getMemoryManager() const { return (*m_Mem); }
	/**
	 * Sets the memory manager.
	 * @param pMem a new concrete memory manager
	 */
	void setMemoryManager(MemoryManager* pMem) { m_Mem = pMem; }
	/* ********************************************************************
	 * Experiment-Flow & Event Management API:
	 * ********************************************************************/
	/**
	 * Adds the specified experiment or plugin and creates a coroutine to
	 * run it in.
	 * @param flow the experiment flow object to be added
	 */
	void addFlow(ExperimentFlow* flow);
	/**
	 * Removes the specified experiment or plugin and destroys its coroutine
	 * and all associated events.
	 * @param flow the experiment flow object to be removed
	 */
	void removeFlow(ExperimentFlow* flow);
	/**
	 * Add event ev to the event management. This causes the event to be
	 * active.
	 * @param ev the event pointer to be added for the current flow
	 * @return the id of the event used to identify the object on occurrence;
	 *         -1 is returned on errors
	 */
	EventId addEvent(BaseEvent* ev);
	/**
	 * Removes the event with the specified id.
	 * @param ev the pointer of the event-object to be removed; if \a ev is
	 *        equal to \c NULL all events (for all experiments) will be
	 *        removed
	 */
	void removeEvent(BaseEvent* ev) { m_EvList.remove(ev); }
	/**
	 * Removes all previously added events for all experiments.  To
	 * restrict this to a specific experiment flow, pass a pointer to it.
	 */
	void clearEvents(ExperimentFlow *flow = 0) { m_EvList.remove(flow); }
	/**
	 * Waits on any events which have been added to the event management. If
	 * one of those events occour, waitAny() will return the id of that event.
	 * @return the previously occurred event
	 * 
	 * FIXME: Maybe this should return immediately if there are not events?
	 *        (additional parameter flag?)
	 */
	BaseEvent* waitAny();
	/**
	 * Add event \a ev to the global buffer and wait for it (combines
	 * \c addEvent() and \c waitAny()).
	 * @param ev the event pointer to be added
	 * @return the pointer of the occurred event (it is not guaranteed that
	 *         this pointer will be equal to ev)
	 *
	 * FIXME: Rename to make clear this returns when *any* event occurs
	 */
	BaseEvent* addEventAndWait(BaseEvent* ev);
	/**
	 * Checks whether any experiment flow has events in the event-list.
	 * @return \c true if there are still events, or \c false otherwise
	 */
	bool hasEvents() const { return getEventCount() > 0; }
	/**
	 * Determines the number of (stored) events in the event-list which have
	 * not been triggered so far.
	 * @return the actual number of events
	 */
	unsigned getEventCount() const { return m_EvList.getEventCount(); }
};

} // end-of-namespace: fail

#endif // __SIMULATOR_CONTROLLER_HPP__
