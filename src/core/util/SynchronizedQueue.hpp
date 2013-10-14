/**
 * \brief Queue class that has thread synchronisation.
 */

#ifndef __SYNCHRONIZED_QUEUE_HPP__
#define __SYNCHRONIZED_QUEUE_HPP__

#include <queue>

#ifndef __puma
#include <boost/thread.hpp>
#endif

namespace fail {

template <typename T>
class SynchronizedQueue { // Adapted from: http://www.quantnet.com/cplusplus-multithreading-boost/
private:
	std::queue<T> m_queue; //!< Use STL queue to store data
	unsigned capacity;
	bool finished;
#ifndef __puma
	boost::mutex m_mutex; //!< The mutex to synchronise on
	boost::condition_variable m_cond; //!< The condition to wait for
	boost::condition_variable m_cond_capacity; //!< Another condition to wait for
#endif
public:
	SynchronizedQueue() : capacity(0), finished(false) {}
	SynchronizedQueue(unsigned capacity) : capacity(capacity), finished(false) {}
	int Size()
	{
#ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
		return m_queue.size();
	}
	// Add data to the queue and notify others
	void Enqueue(const T& data)
	{
		// Acquire lock on the queue
#ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
#endif

		while (capacity != 0 && m_queue.size() >= capacity) {
#ifndef __puma
			m_cond_capacity.wait(lock);
#endif
		}

		// Add the data to the queue
		m_queue.push(data);
		// Notify others that data is ready
#ifndef __puma
		m_cond.notify_one();
#endif
	} // Lock is automatically released here

	/**
	 * Get data from the queue. Wait for data if not available
	 */
	T Dequeue()
	{
		// Acquire lock on the queue
#ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
		// When there is no data, wait till someone fills it.
		// Lock is automatically released in the wait and obtained
		// again after the wait
#ifndef __puma
		while (m_queue.size() == 0) {
			if (finished) {
				// default-constructed T, 0 for integral types
				return T();
			}
			m_cond.wait(lock);
		}
#endif
		// Retrieve the data from the queue
		T result = m_queue.front();
		m_queue.pop();

		// Notify others that we have free slots
#ifndef __puma
		if (m_queue.size() < capacity) {
			m_cond_capacity.notify_one();
		}
#endif
		return result;
	} // Lock is automatically released here


	/**
	 * Get data from the queue. Non blocking variant.
	 * @param d Pointer to copy queue element to
	 * @return false if no element in queue
	 *
	 */
	bool Dequeue_nb(T& d)
	{
		// Acquire lock on the queue
#ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
		// When there is no data, return false.
		// Lock is automatically released in the wait and obtained
		// again after the wait
		if (m_queue.size() > 0) {
			// Retrieve the data from the queue
			d = m_queue.front();
			m_queue.pop();
			// Notify others that we have free slots
#ifndef __puma
			if (m_queue.size() < capacity) {
				m_cond_capacity.notify_one();
			}
#endif
			return true;
		} else {
			return false;
		}
	} // Lock is automatically released here

	void setIsFinished(bool value = true)
	{
#ifndef __puma
		boost::unique_lock<boost::mutex> lock(m_mutex);
#endif
		finished = value;
#ifndef __puma
		m_cond.notify_all();
#endif
	}
};

} // end-of-namespace: fail

#endif // __SYNCHRONIZED_QUEUE_HPP__
