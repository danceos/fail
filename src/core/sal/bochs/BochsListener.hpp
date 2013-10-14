#ifndef __BOCHS_LISTENER_HPP__
#define __BOCHS_LISTENER_HPP__

namespace fail {

/**
 * Global internal handler for (Bochs) TimerListeners. The function just calls
 * BochsController::onTimerTrigger().
 */
void onTimerTrigger(void *thisPtr);

} // end-of-namespace: fail

#endif // __BOCHS_LISTENER_HPP__
