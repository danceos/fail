#ifndef __BOCHS_EVENTS_HPP__
  #define __BOCHS_EVENTS_HPP__

#include "../Event.hpp"

#include "BochsController.hpp"

namespace fail {

/**
 * \class TimerEvent
 * Concrete TimerEvent implementation of GenericTimerEvent for the Bochs
 * simulator backend.
 */
class TimerEvent : public GenericTimerEvent {
private:
	/**
	 * Registers a timer in the Bochs simulator. This timer fires \a TimerEvents
	 * to inform the corresponding experiment-flow. Note that the number of timers
	 * (in Bochs) is limited to \c BX_MAX_TIMERS (defaults to 64 in v2.4.6).
	 * @param pev a pointer to the (experiment flow-) allocated TimerEvent object,
	 *        providing all required information to start the time, e.g. the
	 *        timeout value.
	 * @return \c The unique id of the timer recently created. This id is carried
	 *         along with the TimerEvent, @see getId(). On errors, -1 is returned
	 *         (e.g. because a timer with the same id is already existing)
	 */
	static timer_id_t m_registerTimer(TimerEvent* pev);
	/**
	 * Deletes a timer. No further events will be triggered by the timer.
	 * @param pev a pointer to the TimerEvent-object to be removed
	 * @return \c true if the timer with \a pev->getId() has been removed
	 *         successfully, \c false otherwise
	 */
	static bool m_unregisterTimer(TimerEvent* pev);
public:
	/**
	 * Creates a new timer event. This can be used to implement a timeout-
	 * mechanism in the experiment-flow. The timer starts automatically when
	 * added to FailBochs.
	 * @param timeout the time intervall in milliseconds (ms)
	 * @see SimulatorController::addEvent
	 */
	TimerEvent(unsigned timeout)
		: GenericTimerEvent(timeout) { }
	~TimerEvent() { onEventDeletion(); }
	
	/**
	 * This method is called when an experiment flow adds a new event by
	 * calling \c simulator.addEvent(pev) or \c simulator.addEventAndWait(pev).
	 * More specifically, the event will be added to the event-list first
	 * (buffer-list, to be precise) and then this event handler is called.
	 * @return You should return \c true to continue and \c false to prevent
	 *         the addition of the event \a pev, yielding an error in the
	 *         experiment flow (i.e. -1 is returned).
	 */
	bool onEventAddition();
	/**
	 * This method is called when an experiment flow removes an event from
	 * the event-management by calling \c removeEvent(prev), \c clearEvents()
	 * or by deleting a complete flow (\c removeFlow). More specifically, this
	 * event handler will be called *before* the event is actually deleted.
	 */
	void onEventDeletion();
	/**
	 * This method is called when an previously added event is about to be
	 * triggered by the simulator-backend. More specifically, this event handler
	 * will be called *before* the event is actually triggered, i.e. before the
	 * corresponding coroutine is toggled.
	 */
	void onEventTrigger() { onEventDeletion(); }
};

} // end-of-namespace: fail

#endif // __BOCHS_EVENTS_HPP__
