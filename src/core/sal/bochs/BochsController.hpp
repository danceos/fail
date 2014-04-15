#ifndef __BOCHS_CONTROLLER_HPP__
#define __BOCHS_CONTROLLER_HPP__

#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "FailBochsGlobals.hpp"

#include "../SimulatorController.hpp"

#include "bochs.h"
#include "cpu/cpu.h"
#include "config.h"
#include "iodev/iodev.h"
#include "pc_system.h"
#include "gui/siminterface.h" // for BX_SMP_PROCESSORS

namespace fail {

class ExperimentFlow;

/**
 * \class BochsController
 * Bochs-specific implementation of a SimulatorController.
 *
 * @note The instruction (IP) pointer modification handler (onBreakpoint())
 * is called (from the Breakpoints aspect) *every* time the Bochs-internal IP
 * changes. The handler itself evaluates if a breakpoint event needs to be
 * triggered. This handler needs to implement the breakpoint-mechanism in an
 * indirect fashion because the Bochs simulator doesn't support native
 * breakpoints. To be compatible with the interface specified by the simulator
 * class, we need to provide the two members \c m_CPUContext and \c m_CacheEntry.
 * The elements are being set before the handler is called (see
 * \c updateBPEventInfo())
 */
class BochsController : public SimulatorController {
private:
	ExperimentFlow* m_CurrFlow; //!< Stores the current flow for save/restore-operations
	BX_CPU_C *m_CPUContext; //!< Additional information that is passed on occurence of a BPEvent
	bxInstruction_c *m_CurrentInstruction; //!< dito.
public:
	/**
	 * Initialize the controller, i.e., add the number of simulated CPUs.
	 */
	BochsController();
	~BochsController();
	/* ********************************************************************
	 * Standard Listener Handler API:
	 * ********************************************************************/
	/**
	 * I/O port communication handler. This method is called (from
	 * the IOPortCom aspect) every time when Bochs performs a port I/O operation.
	 * @param cpu the CPU that caused the IO port access
	 * @param data the data transmitted
	 * @param port the port it was transmitted on
	 * @param out \c true if the I/O traffic has been outbound, \c false otherwise
	 */
	void onIOPort(ConcreteCPU* cpu, unsigned char data, unsigned port, bool out);
	/**
	 * Internal handler for TimerListeners. This method is called when a previously
	 * registered (Bochs) timer triggers. It searches for the provided TimerListener
	 * object within the ListenerManager and fires such an event by calling
	 * \c triggerActiveListeners().
	 * @param thisPtr a pointer to the TimerListener-object triggered
	 *
	 * FIXME: Due to Bochs internal timer and ips-configuration related stuff,
	 *        the simulator sometimes panics with "keyboard error:21" (see line
	 *        1777 in bios/rombios.c, function keyboard_init()) if a TimerListener
	 *        is added *before* the bios has been loaded and initialized. To
	 *        reproduce this error, try adding a \c TimerListener as the initial step
	 *        in your experiment code and wait for it (\c addListenerAndResume()).
	 *        This leads to the consequence that timers cannot be added/enabled at
	 *        boot time.
	 */
	void onTimerTrigger(void *thisPtr);
	/* ********************************************************************
	 * Simulator Controller & Access API:
	 * ********************************************************************/
	/**
	 * Save simulator state.
	 * @param path Location to store state information
	 * @return \c true if the state has been successfully saved, \c false otherwise
	 */
	bool save(const std::string& path);
	/**
	 * Save finished: Callback from Simulator
	 */
	void saveDone();
	/**
	 * Restore simulator state. Clears all Listeners.
	 * @param path Location to previously saved state information
	 */
	void restore(const std::string& path);
	/**
	 * Restore finished: Callback from Simulator
	 */
	void restoreDone();
	/**
	 * Reboot simulator. Clears all Listeners.
	 */
	void reboot();
	/**
	 * Reboot finished: Callback from Simulator
	 */
	void rebootDone();
	/**
	 * Fire an interrupt.
	 * @param irq Interrupt to be fired
	 */
	void fireInterrupt(unsigned irq);
	/**
	 * Fire done: Callback from Simulator
	 */
	void fireInterruptDone();
	virtual simtime_t getTimerTicks() { return bx_pc_system.time_ticks(); }
	/**
	 * Only works reliably when the simulation has already begun, e.g., after
	 * calling simulator.restore().
	 */
	virtual simtime_t getTimerTicksPerSecond() { return bx_pc_system.time_ticks() / bx_pc_system.time_usec() * 1000000; /* imprecise hack */ }
	/* ********************************************************************
	 * BochsController-specific (not implemented in SimulatorController!):
	 * ********************************************************************/
	/**
	 * Retrieves the textual description (mnemonic) for the current
	 * instruction. The format of the returned string is Bochs-specific.
	 * @return the mnemonic of the current instruction whose address
	 *         is given by \c Register::getInstructionPointer(). On errors,
	 *         the returned string is empty
	 */
	const std::string& getMnemonic() const;
	/**
	 * Retrieves the current Bochs instruction cache entry
	 * @return a pointer to a \c bxICacheEntry_c object
	 */
	inline bxInstruction_c *getCurrentInstruction() const { return m_CurrentInstruction; }
	/**
	 * Retrieves the current CPU context
	 * @return a pointer to a \c BX_CPU_C object
	 */
	inline BX_CPU_C *getCPUContext() const { return m_CPUContext; }
	/**
	 * Updates the internal BPEvent data. See \c BochsController::onBreakpoint
	 * for further information. This method should only be called from the breakpoint aspect.
	 * @param context the CPU context object ptr (Bochs internal=
	 * @param cacheEntry the Bochs internal CPU cache entry ptr
	 */
	void updateBPEventInfo(BX_CPU_C *context, bxInstruction_c *instr);
	/**
	 * Retrieves the concrete CPU object, based on the given Bochs' internal pointer \a pCPU.
	 * @param pCPU the Bochs' internal CPU object
	 * @return the FailBochs CPU representation that corresponds to Bochs' internal CPU \a pCPU
	 * @see SimulatorController::getCPU().
	 */
	ConcreteCPU& detectCPU(BX_CPU_C* pCPU) const;
};

} // end-of-namespace: fail

#endif // __BOCHS_CONTROLLER_HPP__
