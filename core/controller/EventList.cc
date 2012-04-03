#include <set>

#include "EventList.hpp"
#include "../SAL/SALInst.hpp"

namespace fi
{

EventId EventList::add(BaseEvent* ev, ExperimentFlow* pExp)
{
	assert(ev != NULL && "FATAL ERROR: Event (of base type BaseEvent*) cannot be NULL!");
	// a zero counter does not make sense
	assert(ev->getCounter() != 0);
	ev->setParent(pExp); // event is linked to experiment flow
	m_BufferList.push_back(ev);
	return (ev->getId());
}

bool EventList::remove(BaseEvent* ev)
{
	if(ev != NULL)
	{
		iterator it = std::find(m_BufferList.begin(), m_BufferList.end(), ev);
		if(it != end())
		{
			m_BufferList.erase(it);
			m_DeleteList.push_back(ev);
			return (true);
		}
	}
	else
	{
		for(iterator it = m_BufferList.begin(); it != m_BufferList.end();
			it++)
			m_DeleteList.push_back(*it);
		m_BufferList.clear();
		return (true);
	}
	return (false);
}

EventList::iterator EventList::remove(iterator it)
{
	return (m_remove(it, false));
}

EventList::iterator EventList::m_remove(iterator it, bool skip_deletelist)
{
	if(!skip_deletelist)
		m_DeleteList.push_back(*it);
	return (m_BufferList.erase(it));
}

void EventList::remove(ExperimentFlow* flow)
{
	// all events?
	if (flow == 0) {
		m_BufferList.clear();
	} else {
		for (bufferlist_t::iterator it = m_BufferList.begin();
		     it != m_BufferList.end(); ) {
			if ((*it)->getParent() == flow) {
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
			continue;
		}
		// ... need to be pushed into m_DeleteList, as we're currently
		// iterating over m_FireList in fireActiveEvents() and cannot modify it
		if (flow == 0 || (*it)->getParent() == flow) {
			m_DeleteList.push_back(*it);
		}
	}
}

EventList::~EventList()
{
	// nothing to do here yet
}

BaseEvent* EventList::getEventFromId(EventId id)
{
	// Loop through all events:
	for(bufferlist_t::iterator it = m_BufferList.begin();
	    it != m_BufferList.end(); it++)
		if((*it)->getId() == id)
			return (*it);
	return (NULL); // Nothing found.
}

void EventList::makeActive(BaseEvent* ev)
{
	assert(ev && "FATAL ERROR: Event object pointer cannot be NULL!");
	ev->decreaseCounter();
	if (ev->getCounter() > 0) {
		return;
	}
	ev->resetCounter();
	if(remove(ev)) // remove event from buffer-list
		m_FireList.push_back(ev);
}

EventList::iterator EventList::makeActive(iterator it)
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
	return (it_next);
}

void EventList::fireActiveEvents()
{
	for(firelist_t::iterator it = m_FireList.begin();
		it != m_FireList.end(); it++)
	{
		if(std::find(m_DeleteList.begin(), m_DeleteList.end(), *it)
		   == m_DeleteList.end()) // not found in delete-list?
		{
			m_pFired = *it;
			ExperimentFlow* pFlow = m_pFired->getParent();
			assert(pFlow && "FATAL ERROR: The event has no parent experiment (owner)!");
			sal::simulator.m_Flows.toggle(pFlow);
		}
	}
	m_FireList.clear();
	m_DeleteList.clear();
}

size_t EventList::getContextCount() const
{
	set<ExperimentFlow*> uniqueFlows; // count unique ExperimentFlow-ptr
	for(bufferlist_t::const_iterator it = m_BufferList.begin();
		it != m_BufferList.end(); it++)
		uniqueFlows.insert((*it)->getParent());
	return (uniqueFlows.size());
}

} // end-of-namespace: fi
