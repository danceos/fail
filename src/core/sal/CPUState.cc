#include "CPUState.hpp"

namespace fail {

int interrupt_to_fire = -1;

bool CPUState::isSuppressedInterrupt(unsigned interruptNum) const
{
	for (size_t i = 0; i < m_SuppressedInterrupts.size(); i++)
		if ((m_SuppressedInterrupts[i] == interruptNum ||
			m_SuppressedInterrupts[i] == ANY_INTERRUPT) &&
			interruptNum != (unsigned)interrupt_to_fire + 32) {
				// FIXME: This should be dead code...(?)
				if ((int)interruptNum == interrupt_to_fire + 32) {
					interrupt_to_fire = -1;
					return true;
				}
			return true;
		}
	// FIXME: This is simulator-(x86)-specific stuff... (?)
	return false;
}

bool CPUState::addSuppressedInterrupt(unsigned interruptNum)
{
	// Check if already existing:
	if (isSuppressedInterrupt(interruptNum+32))
		return false; // already added: nothing to do here

	// FIXME: addSuppressedInterrupt(ANY_INTERRUPT) can still be called more
	//        than once. This is not handled by the if-statement above.
	if (interruptNum == ANY_INTERRUPT)
		m_SuppressedInterrupts.push_back(interruptNum);
	else
		m_SuppressedInterrupts.push_back(interruptNum+32);
	return true;
}

bool CPUState::removeSuppressedInterrupt(unsigned interruptNum)
{
	for (size_t i = 0; i < m_SuppressedInterrupts.size(); i++) {
		if (m_SuppressedInterrupts[i] == interruptNum+32 ||
			m_SuppressedInterrupts[i] == ANY_INTERRUPT) {
			m_SuppressedInterrupts.erase(m_SuppressedInterrupts.begin() + i);
			return true;
		}
	}
	return false;
}

} // end-of-namespace: fail
