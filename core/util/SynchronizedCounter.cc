#include "SynchronizedCounter.hpp"

namespace fail {

int SynchronizedCounter::increment()
{
	// Acquire lock on the queue
#ifndef __puma
	boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
	return ++m_counter;
} // Lock is automatically released here

int SynchronizedCounter::decrement()
{
// Acquire lock on the queue
#ifndef __puma
	boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
	return --m_counter;
} // Lock is automatically released here

int SynchronizedCounter::getValue()
{
	// Acquire lock on the queue
#ifndef __puma
	boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
	return m_counter;
} // Lock is automatically released here

} // end-of-namespace: fail
