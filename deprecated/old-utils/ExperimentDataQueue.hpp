/**
 * \brief A queue for experiment data.
 * 
 *
 * \author Martin Hoffmann, Richard Hellwig
 *
 */

// FIXME: This is deprecated stuff. Remove it.

#ifndef __EXPERIMENT_DATA_QUEUE_H__
#define __EXPERIMENT_DATA_QUEUE_H__


#include <deque>
#include "ExperimentData.hpp"


namespace fi{

/**
 * \class ExperimentDataQueue
 * Class which manage ExperimentData in a queue.
 */
	class ExperimentDataQueue	
	{
		protected:
			std::deque<ExperimentData*> m_queue;

		public:
			ExperimentDataQueue() {}
			~ExperimentDataQueue() {}

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
		 * @return the size of teh queue
		 */
			size_t size() const { return m_queue.size(); };	
	};

};

#endif //__EXPERIMENT_DATA_QUEUE_H__


