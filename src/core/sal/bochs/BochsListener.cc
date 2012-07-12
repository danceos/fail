#include "BochsListener.hpp"
#include "../SALInst.hpp"

namespace fail {

bool TimerListener::onAddition()
{
	// Register the timer listener in the Bochs simulator:
	setId(m_registerTimer(this));
	if(getId() == -1)
		return false; // unable to register the timer (error in Bochs' function call)
	return true;
}

void TimerListener::onDeletion()
{
	// Unregister the time listener:
	m_unregisterTimer(this);
}

timer_id_t TimerListener::m_registerTimer(TimerListener* pev)
{
	assert(pev != NULL && "FATAL ERROR: TimerListener object ptr cannot be NULL!");
	return static_cast<timer_id_t>(
		bx_pc_system.register_timer(pev, BochsController::onTimerTrigger, pev->getTimeout(),
			false, 1/*start immediately*/, "Fail*: BochsController"/*name*/));
}

bool TimerListener::m_unregisterTimer(TimerListener* pev)
{
	assert(pev != NULL && "FATAL ERROR: TimerListener object ptr cannot be NULL!");
	bx_pc_system.deactivate_timer(static_cast<unsigned>(pev->getId()));
	return bx_pc_system.unregisterTimer(static_cast<unsigned>(pev->getId()));
}

} // end-of-namespace: fail
