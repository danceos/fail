#include <set>

#include "EventManager.hpp"
#include "SALInst.hpp"

namespace fail {

void EventManager::addToCaches(BaseEvent *ev)
{
	BPEvent *bps_ev;
	if ((bps_ev = dynamic_cast<BPEvent*>(ev)) != NULL)
		m_Bp_cache.add(bps_ev);

	IOPortEvent *io_ev;
	if ((io_ev = dynamic_cast<IOPortEvent*>(ev)) != NULL)
		m_Io_cache.add(io_ev);
}

void EventManager::removeFromCaches(BaseEvent *ev)
{
	BPEvent *bpr_ev;
	if ((bpr_ev = dynamic_cast<BPEvent*>(ev)) != NULL)
		m_Bp_cache.remove(bpr_ev);

	IOPortEvent *io_ev;
	if ((io_ev = dynamic_cast<IOPortEvent*>(ev)) != NULL)
		m_Io_cache.remove(io_ev);
}

void EventManager::clearCaches()
{
	m_Bp_cache.clear();
	m_Io_cache.clear();
}

void EventManager::add(BaseEvent* ev, ExperimentFlow* pExp)
{
	assert(ev != NULL && "FATAL ERROR: Event (of base type BaseEvent*) cannot be NULL!");
	// a zero counter does not make sense
	assert(ev->getCounter() != 0);
	ev->setParent(pExp); // event is linked to experiment flow

	addToCaches(ev);
	m_BufferList.push_back(ev);
}

void EventManager::remove(BaseEvent* ev)
{
	// possible cases:
	// - ev == 0 -> remove all events
	//   * clear m_BufferList
	//   * copy m_FireList to m_DeleteList
	if (ev == 0) {
		for (bufferlist_t::iterator it = m_BufferList.begin(); it != m_BufferList.end(); it++)
			(*it)->onEventDeletion();
		for (firelist_t::iterator it = m_FireList.begin(); it != m_FireList.end(); it++)
			(*it)->onEventDeletion();
		clearCaches();
		m_BufferList.clear();
		// all remaining active events must not fire anymore
		m_DeleteList.insert(m_DeleteList.end(), m_FireList.begin(), m_FireList.end());

	// - ev != 0 -> remove single event
	//   * find/remove ev in m_BufferList
	//   * if ev in m_FireList, copy to m_DeleteList
	} else {
		ev->onEventDeletion();

		removeFromCaches(ev);
		m_BufferList.remove(ev);
		firelist_t::const_iterator it =
			std::find(m_FireList.begin(), m_FireList.end(), ev);
		if (it != m_FireList.end()) {
			m_DeleteList.push_back(ev);
		}
	}
}

EventManager::iterator EventManager::remove(iterator it)
{
	return m_remove(it, false);
}

EventManager::iterator EventManager::m_remove(iterator it, bool skip_deletelist)
{
	if (!skip_deletelist) {
		// If skip_deletelist = true, m_remove was called from makeActive. Accordingly, we
		// are not going to delete an event, instead we are "moving" an event object (= *it)
		// from the buffer list to the fire-list. Therefore we only need to call the simulator's
		// event handler (onEventDeletion), if m_remove is called with the primary intention
		// to *delete* (not "move") an event.
		(*it)->onEventDeletion();
		m_DeleteList.push_back(*it);

		// Cached events have their own BufferCache<T>::makeActive() implementation, which
		// calls this method and afterwards erase() in the cache class. This is why, when
		// called from any kind of makeActive() method, it is unnecessary to call
		// BufferCache<T>::remove() from m_remove().

		// NOTE: in case the semantics of skip_deletelist change, please adapt the following lines
		removeFromCaches(*it);
	}

	return m_BufferList.erase(it);
}

void EventManager::remove(ExperimentFlow* flow)
{
	// all events?
	if (flow == 0) {
		for (bufferlist_t::iterator it = m_BufferList.begin();
		     it != m_BufferList.end(); it++)
			(*it)->onEventDeletion(); // invoke event handler
		clearCaches();
		m_BufferList.clear();
	} else { // remove all events corresponding to a specific experiment ("flow"):
		for (bufferlist_t::iterator it = m_BufferList.begin();
		     it != m_BufferList.end(); ) {
			if ((*it)->getParent() == flow) {
				(*it)->onEventDeletion();
				it = m_BufferList.erase(it);
			} else {
				++it;
			}
		}
	}
	// events that just fired / are about to fire ...
	for (firelist_t::const_iterator it = m_FireList.begin();
		 it != m_FireList.end(); it++) {
		if (std::find(m_DeleteList.begin(), m_DeleteList.end(), *it)
		    != m_DeleteList.end()) {
			continue;  // (already in the delete-list? -> skip!)
		}
		// ... need to be pushed into m_DeleteList, as we're currently
		// iterating over m_FireList in fireActiveEvents() and cannot modify it
		if (flow == 0 || (*it)->getParent() == flow) {
			(*it)->onEventDeletion();
			m_DeleteList.push_back(*it);
		}
	}
}

EventManager::~EventManager()
{
	// nothing to do here yet
}

EventManager::iterator EventManager::makeActive(iterator it)
{
	assert(it != m_BufferList.end() &&
		   "FATAL ERROR: Iterator has already reached the end!");
	BaseEvent* ev = *it;
	assert(ev && "FATAL ERROR: Event object pointer cannot be NULL!");
	ev->decreaseCounter();
	if (ev->getCounter() > 0) {
		return ++it;
	}
	ev->resetCounter();
	// Note: This is the one and only situation in which remove() should NOT
	//       store the removed item in the delete-list.
	iterator it_next = m_remove(it, true); // remove event from buffer-list
	m_FireList.push_back(ev);
	return it_next;
}

void EventManager::fireActiveEvents()
{
	for (firelist_t::iterator it = m_FireList.begin();
		 it != m_FireList.end(); it++) {
		if (std::find(m_DeleteList.begin(), m_DeleteList.end(), *it)
		    == m_DeleteList.end()) { // not found in delete-list?
			m_pFired = *it;
			// Inform (call) the simulator's (internal) event handler that we are about
			// to trigger an event (*before* we actually toggle the experiment flow):
			m_pFired->onEventTrigger();
			ExperimentFlow* pFlow = m_pFired->getParent();
			assert(pFlow && "FATAL ERROR: The event has no parent experiment (owner)!");
			simulator.m_Flows.toggle(pFlow);
		}
	}
	m_FireList.clear();
	m_DeleteList.clear();
	// Note: Do NOT call any event handlers here!
}

size_t EventManager::getContextCount() const
{
	std::set<ExperimentFlow*> uniqueFlows; // count unique ExperimentFlow-ptr
	for (bufferlist_t::const_iterator it = m_BufferList.begin();
		 it != m_BufferList.end(); it++)
		uniqueFlows.insert((*it)->getParent());

	return uniqueFlows.size();
}

} // end-of-namespace: fail
