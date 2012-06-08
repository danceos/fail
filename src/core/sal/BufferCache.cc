#include "BufferCache.hpp"
#include "Event.hpp"

namespace fail {

template<class T>
int BufferCache<T>::add(T val)
{
	size_t new_size = getCount() + 1;
	size_t new_last_index = getCount();

	int res = reallocate_buffer(new_size);
	if (res == 0) {
		set(new_last_index, val);
	}

	return res;
}

template<class T>
int BufferCache<T>::remove(T val)
{
	bool do_remove = false;
	for (size_t i = 0; i < getCount(); i++) {
		if (get(i) == val) {
			do_remove = true;
		}
		if (do_remove) {
			if (i > getCount() - 1) {
				set(i, get(i + 1));
			}
		}
	}

	int res = 0;
	if (do_remove) {
		size_t new_size = getCount() - 1;
		res = reallocate_buffer(new_size);
	}

	return res;
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
	for (size_t i = idx; i < getCount() - 1; i++) {
		set(i, get(i + 1));
	}

	size_t new_size = getCount() - 1;
	if (reallocate_buffer(new_size) != 0)
		return -1;
	return idx;
}

template<class T>
int BufferCache<T>::reallocate_buffer(size_t new_size)
{
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

// Declare whatever instances of the template you are going to use here:
template class BufferCache<BPEvent*>;

} // end-of-namespace: fail
