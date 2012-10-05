#ifndef __SIMULATOR_CONTROLLER_HPP__
  #define __SIMULATOR_CONTROLLER_HPP__

#include <iostream>
#include <string>
#include <cassert>
#include <vector>

#include "efw/CoroutineManager.hpp"
#include "ListenerManager.hpp"
#include "SALConfig.hpp"

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
 *        Abstraction Layer", SAL for short).
 *
 * This class manages (1..N) experiments and provides access to the underlying
 * simulator/debugger system. Experiments can enlist arbritrary listeners
 * (Breakpoint, Memory access, Traps, etc.). The \c SimulatorController then
 * activates the specific experiment. There are further methods to read/write
 * registers and memory, and control the SUT (save/restore/reset).
 */
class SimulatorController {
protected:
	ListenerManager m_LstList; //!< storage where listeners are being buffered
	CoroutineManager m_Flows; //!< managed experiment flows
	RegisterManager *m_Regs; //!< access to cpu register
	MemoryManager *m_Mem; //!< access to memory pool
	std::vector<unsigned> m_SuppressedInterrupts; //!< list of suppressed interrupts
	friend class ListenerManager; //!< "outsources" the listener management
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
	 * Breakpoint handler. This routine needs to be called in the simulator
	 * specific backend each time a breakpoint occurs.
	 * @param instrPtr the instruction pointer of the breakpoint
	 * @param address_space the address space it should occur in
	 */
	void onBreakpoint(address_t instrPtr, address_t address_space);
	/**
	 * Memory access handler (read/write).
	 * @param addr the accessed memory address
	 * @param len the length of the accessed memory
	 * @param is_write \c true if memory is written, \c false if read
	 * @param instrPtr the address of the instruction causing the memory
	 *        access
	 * 
	 * FIXME: should instrPtr be part of this interface?
	 */
	void onMemoryAccess(address_t addr, size_t len, bool is_write, address_t instrPtr);
	/**
	 * Interrupt handler.
	 * @param interruptNum the interrupt-type id
	 * @param nmi nmi-value from guest-system
	 */
	void onInterrupt(unsigned interruptNum, bool nmi);
	/**
	 * Trap handler.
	 * @param trapNum the trap-type id
	 */
	void onTrap(unsigned trapNum);
	/**
	 * Guest system communication handler.
	 * @param data the "message" from the guest system
	 * @param port the port used for communications
	 */
	void onGuestSystem(char data, unsigned port);
	/**
	 * (Conditional) Jump-instruction handler.
	 * @param flagTriggered \c true if the jump was triggered due to a
	 *        specific FLAG (zero/carry/sign/overflow/parity flag)
	 * @param opcode the opcode of the conrecete jump instruction
	 */
	void onJump(bool flagTriggered, unsigned opcode);
	/* ********************************************************************
	 * Simulator Controller & Access API:
	 * ********************************************************************/
	/**
	 * Save simulator state.
	 * @param path Location to store state information
	 * @return \c true if the state has been successfully saved, \c false otherwise
	 */
	virtual bool save(const std::string& path) = 0;
	/**
	 * Restore simulator state.  Implicitly discards all previously
	 * registered listeners.
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
	 * @return \c true if sucessfully removed, \c false otherwise (not found)
	 */
	bool removeSuppressedInterrupt(unsigned interruptNum);
	/**
	 * Returns the (constant) initialized register manager.
	 * @return a reference to the register manager
	 */
	RegisterManager& getRegisterManager() { return *m_Regs; }
	const RegisterManager& getRegisterManager() const { return *m_Regs; }
	/**
	 * Sets the register manager.
	 * @param pReg the new register manager (or a concrete derived class of \c RegisterManager)
	 */
	void setRegisterManager(RegisterManager* pReg) { m_Regs = pReg; }
	/**
	 * Returns the (constant) initialized memory manager.
	 * @return a reference to the memory manager
	 */
	MemoryManager& getMemoryManager() { return *m_Mem; }
	const MemoryManager& getMemoryManager() const { return *m_Mem; }
	/**
	 * Sets the memory manager.
	 * @param pMem a new concrete memory manager
	 */
	void setMemoryManager(MemoryManager* pMem) { m_Mem = pMem; }
	/* ********************************************************************
	 * Experiment-Flow & Listener Management API:
	 * ********************************************************************/
	/**
	 * Adds the specified experiment or plugin and creates a coroutine to run it in.
	 * @param flow the experiment flow object to be added
	 */
	void addFlow(ExperimentFlow* flow);
	/**
	 * Removes the specified experiment or plugin and destroys its coroutine
	 * and all associated listeners.
	 * @param flow the experiment flow object to be removed
	 */
	void removeFlow(ExperimentFlow* flow);
	/**
	 * Adds the listener \c li to the listener management. This causes the listener to be active.
	 * @param li the listener pointer to be added for the current flow
	 * @return \c true if the listener has been added successfully, \c false otherwise
	 */
	bool addListener(BaseListener* li);
	/**
	 * Removes the listener with the specified pointer \c li.
	 * @param li the pointer of the listener object to be removed; if \c li is
	 *        equal to \c NULL, all listeners (for the \a current experiments) will be removed
	 */
	void removeListener(BaseListener* li) { m_LstList.remove(li); }
	/**
	 * Removes all previously added listeners for all experiments.  To
	 * restrict this to a specific experiment flow, pass a pointer to it.
	 * @param flow a ptr to a specific experiment flow whose listeners should
	 *        be removed; if \c flow is \c NULL, all listeners will be removed
	 */
	void clearListeners(ExperimentFlow *flow = 0) { m_LstList.remove(flow); }
	/**
	 * Switches the control flow to the simulator and waits on any listeners
	 * which have been added to the listener management.  If one of those listeners
	 * occurs, resume() will return the pointer of that listener.  If there are
	 * no more active listeners for this experiment, resume() never returns.
	 * @return the previously occurred listener
	 */
	BaseListener* resume();
	/**
	 * Add listener \c li to the global buffer and continues the simulation
	 * (combines \c addListener() and \c resume()).
	 * @param li the listener pointer to be added
	 * @return the pointer of the occurred listener (it is not guaranteed that
	 *         this pointer will be equal to \c li)
	 */
	BaseListener* addListenerAndResume(BaseListener* li);
	/**
	 * Checks whether any experiment flow has listeners in the listener (buffer-)list.
	 * @return \c true if there are still listeners, or \c false otherwise
	 */
	bool hasListeners() const { return getListenerCount() > 0; }
	/**
	 * Determines the number of (stored) listeners in the listener-list which have
	 * not been triggered so far.
	 * @return the actual number of listeners
	 */
	unsigned getListenerCount() const { return m_LstList.getListenerCount(); }
	/**
	 * Determines the pointer to the listener base type, stored at index \c idx.
	 * @param idx the index within the buffer-list of the listener to retrieve
	 * @return the pointer to the (up-casted) base type (if \c idx is invalid and debug
	 *         mode is enabled, an assertion is thrown)
	 * @note This operation has O(1) time complexity (due to the underlying \c std::vector).
	 * @see ListenerManager::dereference()
	 */
	inline BaseListener* dereference(index_t idx) { return m_LstList.dereference(idx); }
	/**
	 * Toggles the provided experiment flow by activating its coroutine.
	 * @param pfl the experiment flow to be activated
	 */
	void toggle(ExperimentFlow* pfl) { m_Flows.toggle(pfl); }
};

// FIXME (see SimulatorController.cc): Weird, homeless global variable
extern int interrupt_to_fire;

} // end-of-namespace: fail

#endif // __SIMULATOR_CONTROLLER_HPP__
