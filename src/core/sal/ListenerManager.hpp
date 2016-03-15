#ifndef __LISTENER_MANAGER_HPP__
#define __LISTENER_MANAGER_HPP__

#include <cassert>
#include <list>
#include <vector>
#include <algorithm>

#include "perf/BufferInterface.hpp"

namespace fail {

class ExperimentFlow;
class BaseListener;

/**
 * Buffer-list for a specific experiment; acts as a simple storage container
 * for listeners to watch for.
 */
typedef std::vector<BaseListener*>  bufferlist_t;
/**
 * List of listeners that match the current simulator listener; these listeners will
 * be triggered next (the list is used temporarily).
 */
typedef std::vector<BaseListener*>  firelist_t;
/**
 * List of listeners that have been deleted during a toggled experiment flow while
 * triggering the listeners in the fire-list. This list is used to skip already
 * deleted listeners (therefore it is used temporarily either).
 */
typedef std::vector<BaseListener*>  deletelist_t;

/**
 * \class ListenerManager
 *
 * \brief This class manages the listeners of the FAIL* implementation.
 *
 * If a listener is triggered, the internal data structure will be updated (i.e.,
 * the listener will be removed from the so called buffer-list and added to the
 * fire-list). Additionally, if an experiment-flow deletes an "active" listener
 * which is currently stored in the fire-list, the listener (to be removed) will
 * be added to a -so called- delete-list. This ensures to prevent triggering
 * "active" listeners which have already been deleted by a previous experiment
 * flow. (See makeActive() and fireActiveListener() for implementation specific
 * details.) ListenerManager is part of the SimulatorController and "outsources"
 * it's listener management.
 */
class ListenerManager {
private:
	bufferlist_t m_BufferList; //!< the storage for listeners added by exp.
	firelist_t m_FireList; //!< the active listeners (used temporarily)
	deletelist_t m_DeleteList; //!< the deleted listeners (used temporarily)
	BaseListener* m_pFired; //!< the recently fired Listener-object
public:
	/**
	 * Determines the pointer to the listener base type, stored at index \c idx.
	 * @param idx the index within the buffer-list of the listener to retrieve
	 * @return the pointer to the (up-casted) base type (if \c idx is invalid and debug
	 *         mode is enabled, an assertion is thrown)
	 * @note This operation has O(1) time complexity (due to the underlying \c std::vector).
	 */
	inline BaseListener* dereference(index_t idx) { assert(idx < m_BufferList.size() &&
	"FATAL ERROR: Invalid index! Listener already deleted?"); return m_BufferList[idx]; }
	/**
	 * The iterator of this class used to loop through the list of added
	 * listeners. To retrieve an iterator to the first element, call \c begin().
	 * \c end() returns the iterator, pointing after the last element.
	 * (This behaviour equals the STL iterator in C++.)
	 */
	typedef bufferlist_t::iterator iterator;

	ListenerManager() : m_pFired(NULL) { }
	~ListenerManager() { }
	/**
	 * Adds the specified listener object for the given ExperimentFlow to the
	 * list of listeners to be watched for.
	 * @param li pointer to the listener object to be added (cannot be \c NULL)
	 * @param flow the listener context (a pointer to the experiment object
	 *        which is interested in such listeners (cannot be \c NULL)
	 * @note You need to overload this method if you are adding a (type-specific)
	 *       performance buffer-list implementation.
	 */
	void add(BaseListener* li, ExperimentFlow* flow);
	/**
	 * Removes the listener based upon the specified \c li pointer (requires
	 * to loop through the whole buffer-list).
	 * @param li the pointer of the listener to be removed; if \c li is set to
	 *        \c NULL, all listeners (for the \a current experiment) will be
	 *        removed
	 * @note There is no need to re-implement this method in a performance-
	 *       buffer-list implementation.
	 */
	void remove(BaseListener* li);
private:
	/**
	 * Updates the buffer-list by "removing" the element located at index \c idx.
	 * This is done by replacing the element with the last element of the buffer-list.
	 * @param idx the index of the element to be removed
	 * @warning The internals of the listener, stored at index \c idx will be
	 *          updated, too.
	 * @note This method should typically be used in a performance buffer-list
	 *       implementation.
	 */
	void m_remove(index_t idx);
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
	 * @return iterator to the beginning
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
	 * @return iterator to the end
	 */
	iterator end() { return (m_BufferList.end()); }
	/**
	 * Removes all listeners for the specified experiment.
	 * @param flow pointer to experiment context (0 = all experiments)
	 */
	void remove(ExperimentFlow* flow);
	/**
	 * Retrieves the number of experiments which currently have active
	 * listeners. This number is trivially equal to the (current) total
	 * number of ExperimentFlow-objects.
	 * @return number of experiments having active listeners
	 */
	size_t getContextCount() const;
	/**
	 * Retrieves the total number of buffered listeners. This doesn't include
	 * the listeners in the fire- or delete-list.
	 * @return the total listener count (for all flows)
	 */
	size_t getListenerCount() const { return m_BufferList.size(); }
	/**
	 * Retrieves the recently triggered listener object. To map this object to it's
	 * context (i.e., the related \c ExerimentFlow), use \c getLastFiredDest().
	 * @return a pointer to the recent listener or \c NULL if nothing has been
	 *         triggered so far
	 */
	BaseListener* getLastFired() { return (m_pFired); }
	/**
	 * Retrieves the ExperimentFlow-object for the given BaseListener (it's
	 * \a context).
	 * @param li the listener object to be looked up
	 * @return a pointer to the context of \a pEv or \c NULL if the
	 *         corresponding context could not be found
	 */
	ExperimentFlow* getExperimentOf(BaseListener* li);
	/**
	 * Moves the listeners from the (internal) buffer-list to the fire-list.
	 * To actually fire the listeners, call triggerActiveListeners().
	 * Returns an updated iterator which points to the next element. Additionally,
	 * \c makeActive() decreases the listener counter and performs it's operation if
	 * and only if the *decreased* listener counter is zero.
	 * @param ev the listener to trigger
	 * @return returns the updated iterator, pointing to the next element
	 *         after makeActive returns, "it" is *invalid*, so the *returned*
	 *         iterator should be used to continue the iteration
	 *
	 * TODO: Improve naming (instead of "makeActive")?
	 */
	iterator makeActive(iterator it);
	/**
	 * Moves the listener \c pLi from the (internal "performance") buffer-list \c pSrc
	 * to the fire-list. This method should be called from a performance implemenation.
	 * It expects the member \c pLi->getLocation() to be valid, i.e. the element needs
	 * to be stored in the buffer-list previously. Additionally, \c makeActive()
	 * decreases the listener counter and performs it's operation if and only if the
	 * *decreased* listener counter is zero. Activated listener objects will be updated
	 * regarding their location and performance-buffer reference, i.e. their index and
	 * performance-buffer pointer will be invalidated (by setting \c INVALID_INDEX and
	 * \c NULL, respectively).
	 * To actually fire the listeners, call triggerActiveListeners().
	 * @param pLi the listener object pointer to trigger; \c pLi will be removed in
	 *        \c pLi->getPerformanceBuffer(). If the performance buffer-list ptr is
	 *        \c NULL, nothing will be done.
	 */
	void makeActive(BaseListener* pLi);
	/**
	 * Triggers the active listeners. Each listener is triggered if it has not
	 * recently been removed (i.e.: is not found in the delete-list). See
	 * \c makeActive() for more details. The recently triggered listener can be
	 * retrieved by calling \c getLastFired(). After all listeners have been
	 * triggered, the (internal) fire- and delete-list will be cleared.
	 */
	void triggerActiveListeners();
};

} // end-of-namespace: fail

#endif // __LISTENER_MANAGER_HPP__
