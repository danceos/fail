#include "CPUState.hpp"

namespace fail {

// FIXME: Bochs specific?  If not, at least get rid of this global variable.
int interrupt_to_fire = -1;

bool CPUState::isSuppressedInterrupt(unsigned interruptNum)
{
	for (size_t i = 0; i < m_SuppressedInterrupts.size(); i++)
		if ((m_SuppressedInterrupts[i] == interruptNum ||
			m_SuppressedInterrupts[i] == ANY_INTERRUPT) &&
			interruptNum != (unsigned)interrupt_to_fire + 32) {
				if ((int)interruptNum == interrupt_to_fire + 32) {
					interrupt_to_fire = -1;
					return true;
				}
			return true;
		}
	return false;
}

bool CPUState::addSuppressedInterrupt(unsigned interruptNum)
{
	// Check if already existing:
	if (isSuppressedInterrupt(interruptNum+32))
		return false; // already added: nothing to do here
		
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
			m_SuppressedInterrupts[i] == ANY_INTERRUPT)
			m_SuppressedInterrupts.erase(m_SuppressedInterrupts.begin() + i);
			return true;
	}
	return false;
}

} // end-of-namespace: fail
