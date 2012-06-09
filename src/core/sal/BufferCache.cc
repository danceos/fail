#include <algorithm>
#include <vector>
#include "BufferCache.hpp"
#include "Event.hpp"
#include "EventList.hpp"

namespace fail {

template<class T>
void BufferCache<T>::add(T val)
{
	int new_size = getCount() + 1;
	int new_last_index = getCount();

	int res = reallocate_buffer(new_size);
	assert (res == 0 && "FATAL ERROR: Could not add event to cache");

	set(new_last_index, val);
}

template<class T>
void BufferCache<T>::remove(T val)
{
	bool do_remove = false;
	for (int i = 0; i < getCount(); i++) {
		if (get(i) == val) {
			do_remove = true;
		}
		if (do_remove) {
			if (i > getCount() - 1) {
				set(i, get(i + 1));
			}
		}
	}

	if (do_remove) {
		int new_size = getCount() - 1;
		int res = reallocate_buffer(new_size);
		assert (res == 0 && "FATAL ERROR: Could not remove event from cache");
	}
}

template<class T>
void BufferCache<T>::clear()
{
	setCount(0);
	free(m_Buffer);
	m_Buffer = NULL;
}

template<class T>
int BufferCache<T>::erase(int idx)
{
	if(idx < 0 || idx >= getCount())
		return -2;

	for (int i = idx; i < getCount() - 1; i++) {
		set(i, get(i + 1));
	}

	int new_size = getCount() - 1;
	if (reallocate_buffer(new_size) != 0)
		return -1;
	return idx;
}

template<class T>
int BufferCache<T>::reallocate_buffer(int new_size)
{
	if (new_size < 0)
		return 20;

	if (new_size == 0) {
		clear();
		return 0;
	}
	void *new_buffer = realloc(m_Buffer, new_size * sizeof(T));
	if (new_buffer == NULL)
		return 10;
	m_Buffer = static_cast<T*>(new_buffer);
	setCount(new_size);
	return 0;
}

template<class T>
int BufferCache<T>::makeActive(EventList &ev_list, int idx)
{
	assert(idx < getCount() &&
		   "FATAL ERROR: Index larger than cache!");
	T ev = get(idx);
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

// Declare whatever instances of the template you are going to use here:
template class BufferCache<BPEvent*>;

} // end-of-namespace: fail
