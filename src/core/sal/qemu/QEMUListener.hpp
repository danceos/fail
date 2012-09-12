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

class QEMUMemWriteListener : public GenericMemWriteListener {
public:
	QEMUMemWriteListener()
		: GenericMemWriteListener() { }
	QEMUMemWriteListener(address_t addr)
		: GenericMemWriteListener(addr) { }

	virtual bool onAddition();
	virtual void onDeletion();
};

typedef QEMUMemWriteListener MemWriteListener;

} // end-of-namespace: fail

#endif // __QEMU_LISTENER_HPP__
