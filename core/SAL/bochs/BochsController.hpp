#ifndef __BOCHS_CONTROLLER_HPP__
  #define __BOCHS_CONTROLLER_HPP__

#define DEBUG

#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "failbochs.hpp"

#include "../SimulatorController.hpp"
#include "../../controller/Event.hpp"

#include "../../../bochs/bochs.h"
#include "../../../bochs/cpu/cpu.h"
#include "../../../bochs/config.h"
#include "../../../bochs/iodev/iodev.h"
#include "../../../bochs/pc_system.h"

using namespace std;

namespace fi { class ExperimentFlow; }

/// Simulator Abstraction Layer namespace
namespace sal
{
	
/**
 * \class BochsController
 * Bochs-specific implementation of a SimulatorController.
 */
class BochsController : public SimulatorController
{
	private:
		fi::ExperimentFlow* m_CurrFlow; //!< Stores the current flow for save/restore-operations
	  #ifdef DEBUG
		unsigned m_Regularity;
		unsigned m_Counter;
		std::ostream* m_pDest;
	  #endif
		/**
		 * Static internal event handler for TimerEvents. This static function is
		 * called when a previously registered (Bochs) timer triggers. This function
		 * searches for the provided TimerEvent object within the EventList and
		 * fires such an event by calling \c fireActiveEvents().
		 * @param thisPtr a pointer to the TimerEvent-object triggered
		 * 
		 * FIXME: Due to Bochs internal timer and ips-configuration related stuff,
		 *        the simulator sometimes panics with "keyboard error:21" (see line
		 *        1777 in bios/rombios.c, function keyboard_init()) if a TimerEvent
		 *        is added *before* the bios has been loaded and initialized. To
		 *        reproduce this error, try adding a TimerEvent as the initial step
		 *        in your experiment code and wait for it (addEventAndWait()).
		 */
		static void m_onTimerTrigger(void *thisPtr);
		/**
		 * Registers a timer in the Bochs simulator. This timer fires \a TimerEvents
		 * to inform the corresponding experiment-flow. Note that the number of timers
		 * (in Bochs) is limited to \c BX_MAX_TIMERS (defaults to 64 in v2.4.6).
		 * @param pev a pointer to the (experiment flow-) allocated TimerEvent object,
		 *        providing all required information to start the time, e.g. the
		 *        timeout value.
		 * @return \c The unique id of the timer recently created. This id is carried
		 *         along with the TimerEvent, @see getId(). On error, -1 is returned
		 *         (e.g. because a timer with the same id is already existing)
		 */
		timer_id_t m_registerTimer(fi::TimerEvent* pev);
		/**
		 * Deletes a timer. No further events will be triggered by the timer.
		 * @param pev a pointer to the TimerEvent-object to be removed
		 * @return \c true if the timer with \a pev->getId() has been removed
		 *         successfully, \c false otherwise
		 */
		bool m_unregisterTimer(fi::TimerEvent* pev);
	public:
		// Initialize the controller.
		BochsController();
		~BochsController();
		/* ********************************************************************
		 * Standard Event Handler API:
		 * ********************************************************************/
		/**
		 * Instruction pointer modification handler. This method is called (from
		 * the Breakpoints aspect) every time when the Bochs-internal IP changes.
		 * @param instrPtr the new instruction pointer
		 * @param address_space
		 */
		void onInstrPtrChanged(address_t instrPtr, address_t address_space);
		/**
		 * This method is called when an experiment flow adds a new event by
		 * calling \c simulator.addEvent(pev) or \c simulator.addEventAndWait(pev).
		 * More specifically, the event will be added to the event-list first
		 * (buffer-list, to be precise) and then this event handler is called.
		 * @param pev the event which has been added
		 * @return You should return \c true to continue and \c false to prevent
		 *         the addition of the event \a pev.
		 */
		bool onEventAddition(fi::BaseEvent* pev);
		/**
		 * This method is called when an experiment flow removes an event from
		 * the event-management by calling \c removeEvent(prev), \c clearEvents()
		 * or by deleting a complete flow (\c removeFlow). More specifically,
		 * this event handler will be called *before* the event is actually deleted.
		 * @param pev the event to be deleted when returning from the event handler
		 */
		void onEventDeletion(fi::BaseEvent* pev);
		/**
		 * This method is called when an previously added event is about to be
		 * triggered by the simulator-backend. More specifically, this event handler
		 * will be called *before* the event is actually triggered, i.e. before the
		 * corresponding coroutine is toggled.
		 * @param pev the event to be triggered when returning from the event handler
		 */
		void onEventTrigger(fi::BaseEvent* pev);
		/* ********************************************************************
		 * Simulator Controller & Access API:
		 * ********************************************************************/
		/**
		 * Save simulator state.
		 * @param path Location to store state information
		 */
		void save(const string& path);
		/**
		 * Save finished: Callback from Simulator
		 */
		void saveDone();
		/**
		 * Restore simulator state. Clears all Events.
		 * @param path Location to previously saved state information
		 */
		void restore(const string& path);
		/**
		 * Restore finished: Callback from Simulator
		 */
		void restoreDone();
		/**
		 * Reboot simulator. Clears all Events.
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
	  #ifdef DEBUG
		/**
		 * Enables instruction pointer debugging output.
		 * @param regularity the output regularity; 1 to display every
		 *        instruction pointer, 0 to disable
		 * @param dest specifies the output destition; defaults to \c std::cout
		 */
		void dbgEnableInstrPtrOutput(unsigned regularity, std::ostream* dest = &cout);
	  #endif
};

} // end-of-namespace: sal

#endif /* __BOCHS_CONTROLLER_HPP__ */
