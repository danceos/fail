#ifndef __QEMU_LISTENER_HPP__
  #define __QEMU_LISTENER_HPP__

#include "../Listener.hpp"

namespace fail {

typedef GenericBPSingleListener BPSingleListener;

/**
 * \class TimerListener
 * Concrete TimerListener implementation of GenericTimerListener for QEMU.
 */
class TimerListener : public GenericTimerListener {
private:
public:
	// TODO
};

// TODO: MemWriteListener

} // end-of-namespace: fail

#endif // __QEMU_LISTENER_HPP__
