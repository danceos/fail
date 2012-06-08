#include "SynchronizedCounter.hpp"

namespace fail {

int SynchronizedCounter::increment()
{
#ifndef __puma
	boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
	return ++m_counter;
}

int SynchronizedCounter::decrement()
{
#ifndef __puma
	boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
	return --m_counter;
}

int SynchronizedCounter::getValue()
{
#ifndef __puma
	boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
	return m_counter;
}

} // end-of-namespace: fail
