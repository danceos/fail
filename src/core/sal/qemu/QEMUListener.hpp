#ifndef __QEMU_LISTENER_HPP__
  #define __QEMU_LISTENER_HPP__

#include "../Listener.hpp"

namespace fail {

typedef GenericBPSingleListener BPSingleListener;

/**
 * \class QEMUTimerListener
 * Concrete TimerListener implementation of GenericTimerListener for QEMU.
 */
class QEMUTimerListener : public GenericTimerListener {
private:
public:
	QEMUTimerListener(unsigned timeout)
		: GenericTimerListener(timeout) { }
	~QEMUTimerListener() { onDeletion(); } // FIXME ~BaseListener should automatically dequeue a Listener, and then indirectly calls onDeletion.  In the current implementation, no dequeueing happens at all.
	bool onAddition();
	void onDeletion();
};
typedef QEMUTimerListener TimerListener;

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
