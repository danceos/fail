/**
 * \brief A queue for experiment data.
 * 
 *
 * \author Martin Hoffmann, Richard Hellwig
 *
 */

// FIXME: This file is not used. Delete it.

#ifndef __SYNC_EXPERIMENT_DATA_QUEUE_H__
#define __SYNC_EXPERIMENT_DATA_QUEUE_H__

#include "ExperimentDataQueue.hpp"
#include "Signal.hpp"

namespace fi{

/**
 * \class SynchronizedExperimentDataQueue
 * Class which manage ExperimentData in a queue.
 * Thread safe using semphores.
 */
	class SynchronizedExperimentDataQueue	: public ExperimentDataQueue
	{
		private:
		    /// There are maxSize elements in at a time
		    /// Or do we allow a really possibly huge queue?
		      Semaphore m_sema_full;
		      Semaphore m_sema_empty;
	  
		public:
			SynchronizedExperimentDataQueue(int maxSize = 1024) : m_sema_full(maxSize), m_sema_empty(0) {}
			~SynchronizedExperimentDataQueue() {}

		/**
		 * Adds ExperimentData to the queue.
		 * @param exp ExperimentData that is to be added to the queue.
		 */
			void addData(ExperimentData* exp);
		/**
		 * Returns an item from the queue
		 * @return the next element of the queue
		 */
			ExperimentData* getData();
		/**
		 * Returns the number of elements in the queue
		 * @return the size of the queue
		 */
			size_t size() const { return m_queue.size(); };	
	};

};

#endif //__EXPERIMENT_DATA_QUEUE_H__


