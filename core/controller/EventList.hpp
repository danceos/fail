#ifndef __EVENT_LIST_HPP__
  #define __EVENT_LIST_HPP__

#include <cassert>
#include <list>
#include <vector>
#include <algorithm>

#include "Event.hpp"
#include "BufferCache.hpp"

namespace fail {

class ExperimentFlow;

/**
 * Buffer-list for a specific experiment; acts as a simple storage container
 * for events to watch for:
 */
typedef std::list<BaseEvent*>    bufferlist_t;
/**
 * List of events that match the current simulator event; these events will
 * be triggered next (the list is used temporarily).
 */
typedef std::vector<BaseEvent*>  firelist_t;
/**
 * List of events that have been deleted during a toggled experiment flow while
 * triggering the events in the fire-list. This list is used to skip already
 * deleted events (therefore it is used temporarily either).
 */
typedef std::vector<BaseEvent*>  deletelist_t;

/**
 * \class EventList
 *
 * \brief This class manages the events of the Fail* implementation.
 *
 * If a event is triggered, the internal data structure will be updated (id est,
 * the event will be removed from the so called buffer-list and added to the
 * fire-list). Additionaly, if an experiment-flow deletes an "active" event
 * which is currently stored in the fire-list, the event (to be removed) will
 * be added to a -so called- delete-list. This ensures to prevent triggering
 * "active" events which have already been deleted by a previous experiment
 * flow. (See makeActive() and fireActiveEvent() for implementation specific
 * details.) EventList is part of the SimulatorController and "outsources"
 * it's event management.
 */
class EventList
{
	private:
		// TODO: List separation of "critical types"? Hashing/sorted lists? (-> performance!)
		bufferlist_t m_BufferList; //!< the storage for events added by exp.
		firelist_t m_FireList; //!< the active events (used temporarily)
		deletelist_t m_DeleteList; //!< the deleted events (used temporarily)
		BaseEvent* m_pFired; //!< the recently fired Event-object
		BufferCache<BPEvent*> m_Bp_cache;
	public:
		/**
		 * The iterator of this class used to loop through the list of
		 * added events. To retrieve an iterator to the first element, call
		 * begin(). end() returns the iterator, pointing after the last element.
		 * (This behaviour equals the STL iterator in C++.)
		 */
		typedef bufferlist_t::iterator iterator;

		EventList() : m_pFired(NULL) { }
		~EventList();
		/**
		 * Adds the specified event object for the given ExperimentFlow to the
		 * list of events to be watched for.
		 * @param ev pointer to the event object to be added (cannot be \c NULL)
		 * @param pExp the event context (a pointer to the experiment object
		 *        which is interested in such events, cannot be \c NULL)
		 * @return the id of the added event object, that is ev->getId()
		 */
		EventId add(BaseEvent* ev, ExperimentFlow* pExp);
		/**
		 * Removes the event based upon the specified \a ev pointer (requires
		 * to loop through the whole buffer-list).
		 * @param ev the pointer of the event to be removed; if ev is set to
		 *        \c NULL, all events (for \a all experiments) will be
		 *        removed
		 */
		void remove(BaseEvent* ev);
		/**
		 * Behaves like remove(BaseEvent) and additionally updates the provided
		 * iteration.
		 * @return the updated iterator which will point to the next element
		 */
		iterator remove(iterator it);
	private:
		/**
		 * Internal implementation of remove(iterator it) that allows
		 * to skip the delete-list.
		 * @return the updated iterator which will point to the next element
		 */
		iterator m_remove(iterator it, bool skip_deletelist);
	public:
		/**
		 * Returns an iterator to the beginning of the internal data structure.
		 * Don't forget to update the returned iterator when calling one of the
		 * modifying methods like makeActive() or remove(). Therefore you need
		 * to call the iterator-based variants of makeActive() and remove().
		 * \code
		 * [X|1|2| ... |n]
		 *  ^
		 * \endcode
		 */
		iterator begin() { return (m_BufferList.begin()); }
		/**
		 * Returns an iterator to the end of the interal data structure.
		 * Don't forget to update the returned iterator when calling one of the
		 * modifying methods like makeActive() or remove(). Therefore you need
		 * to call the iterator-based variants of makeActive() and remove().
		 * \code
		 * [1|2| ... |n]X
		 *              ^
		 * \endcode
		 */
		iterator end() { return (m_BufferList.end()); }
		/**
		 * Retrieves the event object for the given \a id. The internal data
		 * remains unchanged.
		 * @param id of event to be retrieved.
		 * @return pointer to event or \c NULL of \a id could not be found
		 */
		BaseEvent* getEventFromId(EventId id);
		/**
		 * Removes all events for the specified experiment.
		 * @param flow pointer to experiment context (0 = all experiments)
		 */
		void remove(ExperimentFlow* flow);
		/**
		 * Retrieves the number of experiments which currently have active
		 * events. This number is trivially equal to the (current) total
		 * number of ExperimentFlow-objects.
		 * @return number of experiments having active events
		 */
		size_t getContextCount() const;
		/**
		 * Retrieves the total number of buffered events. This doesn't include
		 * the events in the fire- or delete-list.
		 * @return the total event count (for all flows)
		 */
		size_t getEventCount() const { return m_BufferList.size(); }
		/**
		 * Retrieves the recently triggered event object. To map this object to
		 * it's context (id est, the related ExerimentFlow), use
		 * \c getLastFiredDest().
		 * @return a pointer to the recent event or \c NULL if nothing has been
		 *         triggered so far
		 */
		BaseEvent* getLastFired() { return (m_pFired); }
		/**
		 * Retrieves the ExperimentFlow-object for the given BaseEvent (it's
		 * \a context).
		 * @param pEv the event object to be looked up
		 * @return a pointer to the context of \a pEv or \c NULL if the
		 *         corresponding context could not be found
		 */
		ExperimentFlow* getExperimentOf(BaseEvent* pEv);
		/**
		 * Moves the events from the (internal) buffer-list to the fire-list.
		 * To actually fire the events, call fireActiveEvents().
		 * Returns an updated iterator which points to the next element.
		 * @param ev the event to trigger
		 * @return returns the updated iteration, pointing to the next element
		 *         after makeActive returns, "it" is invalid, so the returned
		 *         iterator should be used to continue the iteration
		 *
		 * TODO: Improve naming (instead of "makeActive")?
		 */
		iterator makeActive(iterator it);
		/**
		 * Triggers the active events. Each event is triggered if it has not
		 * recently been removed (id est: is not found in the delete-list). See
		 * makeActive() for more details. The recently triggered event can be
		 * retrieved by calling \a getLastFired(). After all events have been
		 * triggered, the (internal) fire- and delete-list will be cleared.
		 *
		 * TODO: Improve naming (instead of "fireActiveEvents")?
		 */
		void fireActiveEvents();
		/**
		 * Retrieves the BPEvent buffer cache.
		 * @returns the buffer cache
		 */
		inline BufferCache<BPEvent*> *getBPBuffer() { return &m_Bp_cache; }
};

} // end-of-namespace: fail

#endif // __EVENT_LIST_HPP__
