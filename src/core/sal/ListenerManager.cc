#include <set>

#include "ListenerManager.hpp"
#include "SALInst.hpp"

#include "Listener.hpp"

namespace fail {

void ListenerManager::add(BaseListener* li, ExperimentFlow* pExp)
{
	assert(li != NULL && "FATAL ERROR: Listener (of base type BaseListener*) cannot be NULL!");
	// A zero counter does not make sense
	assert(li->getCounter() != 0 && "FATAL ERROR: Listener counter has already been zero!");
	li->setParent(pExp); // listener is linked to experiment flow
	// Ensure that the listener is not associated with any performance impl.:
	li->setPerformanceBuffer(NULL);
	m_BufferList.push_back(li);
	// Note: To keep the indices (within any perf-buffer-list) valid, we have to
	//       add new elements at the end of the vector.
	index_t idx = m_BufferList.size()-1;
	assert(m_BufferList[idx] == li && "FATAL ERROR: Invalid index after push_back() unexpected!");
	li->setLocation(idx);
}

void ListenerManager::remove(BaseListener* li)
{
	// Possible cases:
	// - li == 0 -> remove all listeners for the current flow
	//   * Inform the listeners (call onDeletion)
	//   * Clear m_BufferList
	//   * Remove indices in corresponding perf. buffer-lists (if existing)
	//   * Copy m_FireList to m_DeleteList
	if (li == 0) {
		// We have to remove *all* indices in *all* (possibly added) performance implementations
		// of matching listeners.
		for (index_t i = 0; i < m_BufferList.size(); ) {
			if (m_BufferList[i]->getParent() == simulator.m_Flows.getCurrent()) {
				m_BufferList[i]->onDeletion();
				if (m_BufferList[i]->getPerformanceBuffer() != NULL)
					m_BufferList[i]->getPerformanceBuffer()->remove(i);
				m_remove(i);
				// Inspect the element at m_BufferList[i] a 2nd time
				// (this element has been updated by m_remove()).
			} else {
				++i;
			}
		}
		// All remaining active listeners must not fire anymore (makeActive() already
		// called onDeletion for these listeners):
		m_DeleteList.insert(m_DeleteList.end(), m_FireList.begin(), m_FireList.end());

	// - li != 0 -> remove single listener
	//   * If added / not removed before,
	//     -> inform the listeners (call onDeletion)
	//     -> Remove the index in the perf. buffer-list (if existing)
	//     -> Find/remove 'li' in 'm_BufferList'
	//   * If 'li' in 'm_FireList', copy to 'm_DeleteList'
	} else {
		// has li been removed previously?
		if (li->getLocation() != INVALID_INDEX) {
			li->onDeletion();
			if (li->getPerformanceBuffer() != NULL) {
				li->getPerformanceBuffer()->remove(li->getLocation());
			}
			m_remove(li->getLocation());
		}
		// if li hasn't fired yet, make sure it doesn't
		firelist_t::const_iterator it =
			std::find(m_FireList.begin(), m_FireList.end(), li);
		if (it != m_FireList.end()) {
			m_DeleteList.push_back(li);
		}
	}
}

ExperimentFlow* ListenerManager::getExperimentOf(BaseListener* li)
{
	for (iterator it = begin(); it != end(); ++it) {
		if (*it == li)
			return (*it)->getParent();
	}
	return NULL;
}

void ListenerManager::m_remove(index_t idx)
{
	// Note: This operation has O(1) time complexity. It copies (aka "swaps") the
	//       trailing element "m_BufferList[m_BufferList.size()-1]" to the slot
	//       at "m_BufferList[idx]" and removes the last element (pop_back()).

	// Override the element to be deleted (= copy the last element to the slot
	// of the element to be deleted) and update their attributes:
	if (!m_BufferList.empty() && idx != INVALID_INDEX) {
		m_BufferList[idx]->setPerformanceBuffer(NULL);
		m_BufferList[idx]->setLocation(INVALID_INDEX);
		// Do we have at least 2 elements (so that there *is* a trailing element
		// to be used for swapping)? If idx == m_BufferList.size()-1, the last
		// element should be removed so there is no need to swap:
		if (m_BufferList.size() > 1 && idx != m_BufferList.size()-1) {
			// Update the index of the element object itself:
			m_BufferList[idx] = m_BufferList[m_BufferList.size()-1];
			index_t oldLoc = m_BufferList[idx]->getLocation();
			m_BufferList[idx]->setLocation(idx);
			// Update the (unique) index within it's performance buffer-list (if existing):
			PerfBufferBase* pBuf = m_BufferList[idx]->getPerformanceBuffer();
			if (pBuf != NULL) {
				pBuf->remove(oldLoc);
				pBuf->add(idx);
			}
		}
		// (pop_back()'s behaviour is undefined if the vector is empty)
		m_BufferList.pop_back(); // remove the last element
	}
}

void ListenerManager::remove(ExperimentFlow* flow)
{
	// All listeners (in all flows)?
	if (flow == 0) {
		// We have to remove *all* indices in *all* (possibly added) performance implementations.
		// Therefore we collect the ptr to these impl. in order to call clear() for each of them.
		std::set<PerfBufferBase*> perfBufLists;
		for (bufferlist_t::iterator it = m_BufferList.begin(); it != m_BufferList.end(); it++) {
			(*it)->onDeletion(); // invoke listener handler
			if ((*it)->getPerformanceBuffer() != NULL)
				perfBufLists.insert((*it)->getPerformanceBuffer());
			(*it)->setPerformanceBuffer(NULL);
			(*it)->setLocation(INVALID_INDEX);
		}
		m_BufferList.clear();
		// Remove the indices within each performance buffer-list (maybe empty):
		for (std::set<PerfBufferBase*>::iterator it = perfBufLists.begin();
			 it != perfBufLists.end(); ++it)
			(*it)->clear();
		// All remaining active listeners must not fire anymore
		m_DeleteList.insert(m_DeleteList.end(), m_FireList.begin(), m_FireList.end());
	} else { // remove all listeners corresponding to a specific experiment ("flow"):
		for (index_t i = 0; i < m_BufferList.size(); ) {
			if (m_BufferList[i]->getParent() == flow) {
				m_BufferList[i]->onDeletion();
				if (m_BufferList[i]->getPerformanceBuffer() != NULL) // perf. buffer-list existing?
					m_BufferList[i]->getPerformanceBuffer()->remove(i); // delete it!
				m_remove(i);
			} else {
				++i;
			}
		}
	}
	// listeners that just fired / are about to fire ...
	for (firelist_t::const_iterator it = m_FireList.begin();
		 it != m_FireList.end(); it++) {
		if (std::find(m_DeleteList.begin(), m_DeleteList.end(), *it) != m_DeleteList.end()) {
			continue;  // (already in the delete-list? -> skip!)
		}
		// ... need to be pushed into m_DeleteList, as we're currently
		// iterating over m_FireList in triggerActiveListeners() and cannot modify it
		if (flow == 0 || (*it)->getParent() == flow) {
			(*it)->onDeletion();
			m_DeleteList.push_back(*it);
		}
	}
}

ListenerManager::iterator ListenerManager::makeActive(iterator it)
{
	assert(it != m_BufferList.end() && "FATAL ERROR: Iterator has already reached the end!");
	BaseListener* li = *it;
	assert(li && "FATAL ERROR: Listener object pointer cannot be NULL!");
	li->decreaseCounter();
	if (li->getCounter() > 0) {
		return ++it;
	}
	li->resetCounter();

	//
	// Remove listener from buffer-list
	// Note: This is the one and only situation in which remove() should NOT
	//       store the removed item in the delete-list.
	(*it)->onDeletion();
	// This has O(1) time complexity due to a underlying std::vector (-> random access iterator)
	index_t dist = std::distance(begin(), it);
	assert((*it)->getPerformanceBuffer() == NULL &&
		"FATAL ERROR: makeActive(iterator) cannot be called on listeners stored in a perf. \
		 buff-list! Use makeActive(BaseListener*) instead!");
	// Remove the element from the buffer-list:
	m_remove((*it)->getLocation());
	// Create a new iterator, pointing to the element after *it
	// (the new iterator actually points to the same slot in the vector, but now
	//  this slot stores the element which has previously been stored in the last slot):
	// This is required because the provided iterator "it" isn't valid anymore (due
	// to the deletion of the last element within m_remove(index_t)).

	iterator it_next = begin() + dist; // O(1)
	// Note: "begin() + dist" yields end() if dist is "large enough" (as computed above)

	m_FireList.push_back(li);
	return it_next;
}

void ListenerManager::makeActive(BaseListener* pLi)
{
	assert(pLi && "FATAL ERROR: Listener object pointer cannot be NULL!");
	// If there is no performance buffer-list implementation, skip the operation:
	if (pLi->getPerformanceBuffer() == NULL)
		return;
	// Decrease and check the listener counter:
	pLi->decreaseCounter();
	if (pLi->getCounter() > 0)
		return;
	pLi->resetCounter();
	// Remove the index of the listener from the performance buffer-list:
	pLi->getPerformanceBuffer()->remove(pLi->getLocation());
	// Move the listener object from the buffer-list to the fire-list:
	m_remove(pLi->getLocation()); // (updates the internals of the listener, too)
	m_FireList.push_back(pLi);
}

void ListenerManager::triggerActiveListeners()
{
	for (firelist_t::iterator it = m_FireList.begin();
		 it != m_FireList.end(); it++) {
		if (std::find(m_DeleteList.begin(), m_DeleteList.end(), *it)
		    == m_DeleteList.end()) { // not found in delete-list?
			m_pFired = *it;
			// Note: onDeletion was previously called within makeActive()!

			// Inform (call) the simulator's (internal) listener handler that we are about
			// to trigger an listener (*before* we actually toggle the experiment flow):
			m_pFired->onTrigger(); // onTrigger will toggle the correct coroutine
		}
	}
	m_FireList.clear();
	m_DeleteList.clear();
	// Note: Do NOT call any listener handlers here!
}

size_t ListenerManager::getContextCount() const
{
	// Note: This works even if there are active performance buffer-list implementations.
	std::set<ExperimentFlow*> uniqueFlows; // count unique ExperimentFlow-ptr
	for (bufferlist_t::const_iterator it = m_BufferList.begin();
		 it != m_BufferList.end(); it++)
		uniqueFlows.insert((*it)->getParent());

	return uniqueFlows.size();
}

} // end-of-namespace: fail
