// Thread safe counter

#ifndef __SYNCHRONIZED_COUNTER_HPP__
#define __SYNCHRONIZED_COUNTER_HPP__

#ifndef __puma
#include <boost/thread.hpp>
#include <sys/types.h>
#endif

class SynchronizedCounter
{
private:
	int m_counter;
#ifndef __puma
	boost::mutex m_mutex;				// The mutex to synchronise on
#endif
	public:
		SynchronizedCounter() : m_counter(0) {};

		int increment();
		int decrement();
		int getValue();
};

#endif