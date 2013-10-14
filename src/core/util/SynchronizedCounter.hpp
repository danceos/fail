/**
 * \brief Thread safe (synchronized) counter.
 */

#ifndef __SYNCHRONIZED_COUNTER_HPP__
#define __SYNCHRONIZED_COUNTER_HPP__

#ifndef __puma
#include <boost/thread.hpp>
#include <sys/types.h>
#endif

namespace fail {

/**
 * \class ssd
 *
 * Provides a thread safe (synchronized) counter. When a method is called,
 * the internal mutex is automatically locked. On return, the lock is
 * automatically released ("scoped lock").
 */
class SynchronizedCounter {
private:
	int m_counter;
#ifndef __puma
	boost::mutex m_mutex; //! The mutex to synchronise on
#endif
public:
	SynchronizedCounter() : m_counter(0) { }

	int increment();
	int decrement();
	int getValue();
};

} // end-of-namespace: fail

#endif // __SYNCHRONIZED_COUNTER_HPP__
