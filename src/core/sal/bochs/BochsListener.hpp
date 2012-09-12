#ifndef __BOCHS_LISTENER_HPP__
  #define __BOCHS_LISTENER_HPP__

#include "../Listener.hpp"

namespace fail {

typedef GenericBPSingleListener BPSingleListener;
typedef GenericMemWriteListener MemWriteListener;

/**
 * \class TimerListener
 * Concrete TimerListener implementation of GenericTimerListener for the Bochs
 * simulator backend.
 */
class TimerListener : public GenericTimerListener {
private:
	/**
	 * Registers a timer in the Bochs simulator. This timer triggers \a TimerListeners
	 * to inform the corresponding experiment-flow. Note that the number of timers
	 * (in Bochs) is limited to \c BX_MAX_TIMERS (defaults to 64 in v2.4.6).
	 * @param pli a pointer to the (experiment flow-) allocated TimerListener object,
	 *        providing all required information to start the timer (e.g. the
	 *        timeout value).
	 * @return The unique id of the timer recently created. This id is carried
	 *         along with the TimerListener. On errors, -1 is returned (e.g. because
	 *         a timer with the same id is already existing)
	 * @see TimerListener::getId()
	 */
	static timer_id_t m_registerTimer(TimerListener* pli);
	/**
	 * Deletes a timer. No further events will be triggered by the timer.
	 * @param pli a pointer to the TimerListener-object to be removed
	 * @return \c true if the timer with \a pli->getId() has been removed
	 *         successfully, \c false otherwise
	 */
	static bool m_unregisterTimer(TimerListener* pli);
public:
	/**
	 * Creates a new timer event. This can be used to implement a timeout-
	 * mechanism in the experiment-flow. The timer starts automatically when
	 * added to FailBochs.
	 * @param timeout the time intervall in milliseconds (ms)
	 * @see SimulatorController::addListener
	 */
	TimerListener(unsigned timeout)
		: GenericTimerListener(timeout) { }
	~TimerListener() { onDeletion(); }
	
	/**
	 * This method is called when an experiment flow adds a new event by
	 * calling \c simulator.addListener() or \c simulator.addListenerAndResume().
	 * More specifically, the event will be added to the event-list first
	 * (buffer-list, to be precise) and then this event handler is called.
	 * @return You should return \c true to continue and \c false to prevent
	 *         the addition of the event \a pev, yielding an error in the
	 *         experiment flow (i.e. -1 is returned).
	 */
	bool onAddition();
	/**
	 * This method is called when an experiment flow removes an event from
	 * the event-management by calling \c removeListener(prev), \c clearListeners()
	 * or by deleting a complete flow (\c removeFlow). More specifically, this
	 * event handler will be called *before* the event is actually deleted.
	 */
	void onDeletion();
	/**
	 * This method is called when an previously added event is about to be
	 * triggered by the simulator-backend. More specifically, this event handler
	 * will be called *before* the event is actually triggered, i.e. before the
	 * corresponding coroutine is toggled.
	 */
	void onTrigger() { onDeletion(); }
	// TODO/FIXME: bei neuer impl. anpassen
};

} // end-of-namespace: fail

#endif // __BOCHS_LISTENER_HPP__
