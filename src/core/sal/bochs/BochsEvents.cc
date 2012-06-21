#include "BochsEvents.hpp"
#include "../SALInst.hpp"

namespace fail {

bool TimerEvent::onEventAddition()
{
	// Register the timer event in the Bochs simulator:
	setId(m_registerTimer(this));
	if(getId() == -1)
		return false; // unable to register the timer (error in Bochs' function call)
	return true;
}

void TimerEvent::onEventDeletion()
{
	// Unregister the time event:
	m_unregisterTimer(this);
}

timer_id_t TimerEvent::m_registerTimer(TimerEvent* pev)
{
	assert(pev != NULL && "FATAL ERROR: TimerEvent object ptr cannot be NULL!");
	return static_cast<timer_id_t>(
		bx_pc_system.register_timer(pev, BochsController::onTimerTrigger, pev->getTimeout(),
			false, 1/*start immediately*/, "Fail*: BochsController"/*name*/));
}

bool TimerEvent::m_unregisterTimer(TimerEvent* pev)
{
	assert(pev != NULL && "FATAL ERROR: TimerEvent object ptr cannot be NULL!");
	bx_pc_system.deactivate_timer(static_cast<unsigned>(pev->getId()));
	return bx_pc_system.unregisterTimer(static_cast<unsigned>(pev->getId()));
}

} // end-of-namespace: fail
