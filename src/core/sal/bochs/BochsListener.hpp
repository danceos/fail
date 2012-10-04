#ifndef __BOCHS_LISTENER_HPP__
  #define __BOCHS_LISTENER_HPP__

namespace fail {

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
 *        This leads to the consequence that timers cannot be added/enabled at
 *        boot time.
 */
void onTimerTrigger(void *thisPtr);

} // end-of-namespace: fail

#endif // __BOCHS_LISTENER_HPP__
