#ifndef __SIMULATOR_CONTROLLER_HPP__
#define __SIMULATOR_CONTROLLER_HPP__

#include <iostream>
#include <string>
#include <cassert>
#include <vector>

#include "efw/CoroutineManager.hpp"
#include "ListenerManager.hpp"
#include "SALConfig.hpp"
#include "ConcreteCPU.hpp"


/// All classes, functions, constants, etc. are encapsulated in the namespace "fail".
namespace fail {

// Incomplete types suffice here:
class ExperimentFlow;
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
	bool m_isInitialized;
	ListenerManager m_LstList; //!< storage where listeners are being buffered
	CoroutineManager m_Flows; //!< managed experiment flows
	MemoryManager *m_Mem; //!< access to memory pool
	std::vector<ConcreteCPU*> m_CPUs; //!< list of CPUs in the target system
	friend class ListenerManager; //!< "outsources" the listener management
public:
	SimulatorController() : m_isInitialized(false), m_Mem(NULL) { }
	SimulatorController(MemoryManager* mem) : m_isInitialized(false), m_Mem(mem) { }
	virtual ~SimulatorController() { }
	/**
	 * @brief Initialization function each implementation needs to call on
	 *        startup
	 *
	 * This function needs to be invoked once the simulator starts, and
	 * allows the SimulatorController to instantiate all needed experiment
	 * components.
	 *
	 * @param argc main()'s argument counter
	 * @param argv main()'s argument value vector
	 */
	void startup(int& argc, char **& argv);
	/**
	 * @brief Parameter-less version of startup() for backends that do not (yet) handle parameters
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
	 * @param cpu the CPU that reached the breakpoint
	 * @param instrPtr the instruction pointer of the breakpoint
	 * @param address_space the address space it should occur in
	 */
	void onBreakpoint(ConcreteCPU* cpu, address_t instrPtr, address_t address_space);
	/**
	 * Memory access handler (read/write).
	 * @param cpu the CPU that accessed the memory
	 * @param addr the accessed memory address
	 * @param len the length of the accessed memory
	 * @param is_write \c true if memory is written, \c false if read
	 * @param instrPtr the address of the instruction causing the memory
	 *        access
	 *
	 * FIXME: should instrPtr be part of this interface?
	 */
	void onMemoryAccess(ConcreteCPU* cpu, address_t addr, size_t len, bool is_write, address_t instrPtr);
	/**
	 * Interrupt handler.
	 * @param cpu the CPU that caused the interrupt
	 * @param interruptNum the interrupt-type id
	 * @param nmi nmi-value from guest-system
	 */
	void onInterrupt(ConcreteCPU* cpu, unsigned interruptNum, bool nmi);
	/**
	 * Trap handler.
	 * @param cpu the CPU that caused the trap
	 * @param trapNum the trap-type id
	 */
	void onTrap(ConcreteCPU* cpu, unsigned trapNum);
	/**
	 * Guest system communication handler.
	 * @param data the "message" from the guest system
	 * @param port the port used for communications
	 */
	void onGuestSystem(char data, unsigned port);
	// FIXME: ConcreteCPU* cpu is missing here...
	/**
	 * (Conditional) Jump-instruction handler.
	 * @param cpu the CPU that did the jump
	 * @param flagTriggered \c true if the jump was triggered due to a
	 *        specific FLAG (zero/carry/sign/overflow/parity flag)
	 * @param opcode the opcode of the conrecete jump instruction
	 */
	void onJump(ConcreteCPU* cpu, bool flagTriggered, unsigned opcode);
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
	 * Adds a new CPU to the CPU list. Listener/Events and experiment code can require that all
	 * CPUs of the simulated system are added. So it's recommend to add them early in the backend
	 * implementation (especially before the experiment code runs).
	 * @param cpu the cpu that should be added to the list
	 */
	void addCPU(ConcreteCPU* cpu);
	/**
	 * Gets the CPU with the provided \c id.
	 * @param id the id of the CPU to get
	 * @return a reference to the requested CPU object
	 */
	ConcreteCPU& getCPU(size_t id) const;
	/**
	 * Get the total number of CPUs.
	 * @return the CPU count
	 */
	size_t getCPUCount() const { return m_CPUs.size(); }
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
	 * Checks whether any experiment flow has listeners in the listener
	 * (buffer-)list.  For internal use.
	 * @return \c true if there are still listeners, or \c false otherwise
	 */
	bool hasListeners() const { return getListenerCount() > 0; }
	/**
	 * Determines the number of (stored) listeners in the listener-list which have
	 * not been triggered so far.  For internal use.
	 * @return the actual number of listeners
	 */
	unsigned getListenerCount() const { return m_LstList.getListenerCount(); }
	/**
	 * Determines the pointer to the listener base type, stored at index \c
	 * idx.  For internal use.
	 * @param idx the index within the buffer-list of the listener to retrieve
	 * @return the pointer to the (up-casted) base type (if \c idx is invalid and debug
	 *         mode is enabled, an assertion is thrown)
	 * @note This operation has O(1) time complexity (due to the underlying \c std::vector).
	 * @see ListenerManager::dereference()
	 */
	inline BaseListener* dereference(index_t idx) { return m_LstList.dereference(idx); }
	/**
	 * Toggles the provided experiment flow by activating its coroutine.  For
	 * internal use.
	 * @param pfl the experiment flow to be activated
	 */
	void toggle(ExperimentFlow* pfl) { m_Flows.toggle(pfl); }
	/**
	 * Retrieves the current backend time, in a backend-specific format.
	 * @note FIXME Consider making this pure virtual.
	 * @see SimulatorController::getTimerTicksPerSecond()
	 */
	virtual simtime_t getTimerTicks() { return 0; }
	/**
	 * Retrieves the backend-specific number of timer ticks per second.
	 * @note FIXME Consider making this pure virtual.
	 * @see SimulatorController::getTimerTicks()
	 */
	virtual simtime_t getTimerTicksPerSecond() { return 0; }
	/**
	 * Returns whether FAIL* has already been initialized.
	 * @return A Boolean, indicating whether FAIL* has been initialized.
	*/
	bool isInitialized() { return m_isInitialized; }
};

} // end-of-namespace: fail

#endif // __SIMULATOR_CONTROLLER_HPP__
