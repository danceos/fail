#include <algorithm>
#include <vector>
#include "BufferCache.hpp"
#include "Event.hpp"
#include "EventList.hpp"

namespace fail {

template<class T>
typename BufferCache<T>::iterator BufferCache<T>::makeActive(EventList &ev_list, BufferCache<T>::iterator idx)
{
	assert(idx != end() &&
		   "FATAL ERROR: Index larger than cache!");
	T ev = *idx;
	assert(ev && "FATAL ERROR: Object pointer cannot be NULL!");
	ev->decreaseCounter();
	if (ev->getCounter() > 0) {
		return ++idx;
	}
	ev->resetCounter();
	// Note: This is the one and only situation in which remove() should NOT
	//       store the removed item in the delete-list.
	EventList::iterator it = std::find(ev_list.begin(), ev_list.end(), static_cast<BaseEvent*>(ev));
	ev_list.m_remove(it, true); // remove event from buffer-list
	ev_list.m_FireList.push_back(ev);
	return erase(idx);
}

// Declare here whatever instances of the template you are going to use:
template class BufferCache<BPEvent*>;
template class BufferCache<IOPortEvent*>;

} // end-of-namespace: fail
