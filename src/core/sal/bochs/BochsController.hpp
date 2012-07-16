#ifndef __BOCHS_CONTROLLER_HPP__
  #define __BOCHS_CONTROLLER_HPP__

#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "FailBochsGlobals.hpp"
#include "BochsListener.hpp"

#include "../SimulatorController.hpp"
#include "../Listener.hpp"

#include "bochs.h"
#include "cpu/cpu.h"
#include "config.h"
#include "iodev/iodev.h"
#include "pc_system.h"

namespace fail {

class ExperimentFlow;

/**
 * \class BochsController
 * Bochs-specific implementation of a SimulatorController.
 */
class BochsController : public SimulatorController {
private:
	ExperimentFlow* m_CurrFlow; //!< Stores the current flow for save/restore-operations
  #ifdef DEBUG
	unsigned m_Regularity; //! regularity of instruction ptr output
	unsigned m_Counter; //! current instr-ptr counter
	std::ostream* m_pDest; //! debug output object (defaults to \c std::cout)
  #endif
public:
	// Initialize the controller.
	BochsController();
	~BochsController();
	/* ********************************************************************
	 * Standard Listener Handler API:
	 * ********************************************************************/
	/**
	 * Instruction pointer modification handler. This method is called (from
	 * the Breakpoints aspect) every time when the Bochs-internal IP changes.
	 * @param instrPtr the new instruction pointer
	 * @param address_space the address space the CPU is currently in
	 */
	void onInstrPtrChanged(address_t instrPtr, address_t address_space, BX_CPU_C *context,
	                       bxICacheEntry_c *cache_entry);
	/**
	 * I/O port communication handler. This method is called (from
	 * the IOPortCom aspect) every time when Bochs performs a port I/O operation.
	 * @param data the data transmitted
	 * @param port the port it was transmitted on
	 * @param out true if the I/O traffic has been outbound, false otherwise
	 */
	void onIOPort(unsigned char data, unsigned port, bool out);
	/**
	 * Static internal handler for TimerListeners. This static function is
	 * called when a previously registered (Bochs) timer triggers. This function
	 * searches for the provided TimerListener object within the ListenerManager and
	 * fires such an event by calling \c triggerActiveListeners().
	 * @param thisPtr a pointer to the TimerListener-object triggered
	 * 
	 * FIXME: Due to Bochs internal timer and ips-configuration related stuff,
	 *        the simulator sometimes panics with "keyboard error:21" (see line
	 *        1777 in bios/rombios.c, function keyboard_init()) if a TimerListener
	 *        is added *before* the bios has been loaded and initialized. To
	 *        reproduce this error, try adding a \c TimerListener as the initial step
	 *        in your experiment code and wait for it (\c addListenerAndResume()).
	 */
	static void onTimerTrigger(void *thisPtr);
	/* ********************************************************************
	 * Simulator Controller & Access API:
	 * ********************************************************************/
	/**
	 * Save simulator state.
	 * @param path Location to store state information
	 */
	void save(const std::string& path);
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
	/* ********************************************************************
	 * BochsController-specific (not implemented in SimulatorController!):
	 * ********************************************************************/
  #ifdef DEBUG
	/**
	 * Enables instruction pointer debugging output.
	 * @param regularity the output regularity; 1 to display every
	 *        instruction pointer, 0 to disable
	 * @param dest specifies the output destition; defaults to \c std::cout
	 */
	void dbgEnableInstrPtrOutput(unsigned regularity, std::ostream* dest = &std::cout);
  #endif
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
	 * @returns a pointer to a bxICacheEntry_c object
	 */
	inline bxICacheEntry_c *getICacheEntry() const { return m_CacheEntry; }
	/**
	 * Retrieves the current CPU context
	 * @return a pointer to a BX_CPU_C object
	 */
	inline BX_CPU_C *getCPUContext() const { return m_CPUContext; }
private:
	BX_CPU_C *m_CPUContext;
	bxICacheEntry_c *m_CacheEntry;
};

} // end-of-namespace: fail

#endif // __BOCHS_CONTROLLER_HPP__
