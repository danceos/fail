#include <set>

#include "ListenerManager.hpp"
#include "SALInst.hpp"

namespace fail {

void ListenerManager::addToCaches(BaseListener *li)
{
	BPListener *bps_li;
	if ((bps_li = dynamic_cast<BPListener*>(li)) != NULL)
		m_Bp_cache.add(bps_li);

	IOPortListener *io_li;
	if ((io_li = dynamic_cast<IOPortListener*>(li)) != NULL)
		m_Io_cache.add(io_li);
}

void ListenerManager::removeFromCaches(BaseListener *li)
{
	BPListener *bpr_li;
	if ((bpr_li = dynamic_cast<BPListener*>(li)) != NULL)
		m_Bp_cache.remove(bpr_li);

	IOPortListener *io_li;
	if ((io_li = dynamic_cast<IOPortListener*>(li)) != NULL)
		m_Io_cache.remove(io_li);
}

void ListenerManager::clearCaches()
{
	m_Bp_cache.clear();
	m_Io_cache.clear();
}

void ListenerManager::add(BaseListener* li, ExperimentFlow* pExp)
{
	assert(li != NULL && "FATAL ERROR: Listener (of base type BaseListener*) cannot be NULL!");
	// a zero counter does not make sense
	assert(li->getCounter() != 0);
	li->setParent(pExp); // listener is linked to experiment flow

	addToCaches(li);
	m_BufferList.push_back(li);
}

void ListenerManager::remove(BaseListener* li)
{
	// possible cases:
	// - li == 0 -> remove all listeners
	//   * clear m_BufferList
	//   * copy m_FireList to m_DeleteList
	if (li == 0) {
		for (bufferlist_t::iterator it = m_BufferList.begin(); it != m_BufferList.end(); it++)
			(*it)->onDeletion();
		for (firelist_t::iterator it = m_FireList.begin(); it != m_FireList.end(); it++)
			(*it)->onDeletion();
		clearCaches();
		m_BufferList.clear();
		// all remaining active listeners must not fire anymore
		m_DeleteList.insert(m_DeleteList.end(), m_FireList.begin(), m_FireList.end());

	// - li != 0 -> remove single listener
	//   * find/remove 'li' in 'm_BufferList'
	//   * if 'li' in 'm_FireList', copy to 'm_DeleteList'
	} else {
		li->onDeletion();

		removeFromCaches(li);
		m_BufferList.remove(li);
		firelist_t::const_iterator it =
			std::find(m_FireList.begin(), m_FireList.end(), li);
		if (it != m_FireList.end()) {
			m_DeleteList.push_back(li);
		}
	}
}

ListenerManager::iterator ListenerManager::m_remove(iterator it, bool skip_deletelist)
{
	if (!skip_deletelist) {
		// If skip_deletelist = true, m_remove was called from makeActive. Accordingly, we
		// are not going to delete an listener, instead we are "moving" a listener object (= *it)
		// from the buffer list to the fire-list. Therefore we only need to call the simulator's
		// listener handler (onDeletion), if m_remove is called with the primary intention
		// to *delete* (not "move") a listener.
		(*it)->onDeletion();
		m_DeleteList.push_back(*it);

		// Cached listeners have their own BufferCache<T>::makeActive() implementation, which
		// calls this method and afterwards erase() in the cache class. This is why, when
		// called from any kind of makeActive() method, it is unnecessary to call
		// BufferCache<T>::remove() from m_remove().

		// NOTE: in case the semantics of skip_deletelist change, please adapt the following lines
		removeFromCaches(*it);
	}

	return m_BufferList.erase(it);
}

void ListenerManager::remove(ExperimentFlow* flow)
{
	// all listeners?
	if (flow == 0) {
		for (bufferlist_t::iterator it = m_BufferList.begin();
		     it != m_BufferList.end(); it++)
			(*it)->onDeletion(); // invoke listener handler
		clearCaches();
		m_BufferList.clear();
	} else { // remove all listeners corresponding to a specific experiment ("flow"):
		for (bufferlist_t::iterator it = m_BufferList.begin();
		     it != m_BufferList.end(); ) {
			if ((*it)->getParent() == flow) {
				(*it)->onDeletion();
				it = m_BufferList.erase(it);
			} else {
				++it;
			}
		}
	}
	// listeners that just fired / are about to fire ...
	for (firelist_t::const_iterator it = m_FireList.begin();
		 it != m_FireList.end(); it++) {
		if (std::find(m_DeleteList.begin(), m_DeleteList.end(), *it)
		    != m_DeleteList.end()) {
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

ListenerManager::~ListenerManager()
{
	// nothing to do here yet
}

ListenerManager::iterator ListenerManager::makeActive(iterator it)
{
	assert(it != m_BufferList.end() &&
		   "FATAL ERROR: Iterator has already reached the end!");
	BaseListener* li = *it;
	assert(li && "FATAL ERROR: Listener object pointer cannot be NULL!");
	li->decreaseCounter();
	if (li->getCounter() > 0) {
		return ++it;
	}
	li->resetCounter();
	// Note: This is the one and only situation in which remove() should NOT
	//       store the removed item in the delete-list.
	iterator it_next = m_remove(it, true); // remove listener from buffer-list
	m_FireList.push_back(li);
	return it_next;
}

void ListenerManager::triggerActiveListeners()
{
	for (firelist_t::iterator it = m_FireList.begin();
		 it != m_FireList.end(); it++) {
		if (std::find(m_DeleteList.begin(), m_DeleteList.end(), *it)
		    == m_DeleteList.end()) { // not found in delete-list?
			m_pFired = *it;
			// Inform (call) the simulator's (internal) listener handler that we are about
			// to trigger an listener (*before* we actually toggle the experiment flow):
			m_pFired->onTrigger();
			ExperimentFlow* pFlow = m_pFired->getParent();
			assert(pFlow && "FATAL ERROR: The listener has no parent experiment (owner)!");
			simulator.m_Flows.toggle(pFlow);
		}
	}
	m_FireList.clear();
	m_DeleteList.clear();
	// Note: Do NOT call any listener handlers here!
}

size_t ListenerManager::getContextCount() const
{
	std::set<ExperimentFlow*> uniqueFlows; // count unique ExperimentFlow-ptr
	for (bufferlist_t::const_iterator it = m_BufferList.begin();
		 it != m_BufferList.end(); it++)
		uniqueFlows.insert((*it)->getParent());

	return uniqueFlows.size();
}

} // end-of-namespace: fail
