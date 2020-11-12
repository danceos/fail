#include "Listener.hpp"
#include "SALInst.hpp"

namespace fail {

BaseListener::~BaseListener()
{
	simulator.removeListener(this);
}

void BaseListener::onTrigger()
{
	assert(m_Parent && "FATAL ERROR: The listener has no parent!");
	simulator.toggle(m_Parent);
}

bool TroubleListener::isMatching(const TroubleEvent* pEv) const
{
	for (unsigned i = 0; i < m_WatchNumbers.size(); i++) {
		if (m_CPU == NULL || m_CPU == pEv->getTriggerCPU()) {
			if (m_WatchNumbers[i] == pEv->getTriggerNumber() ||
			   m_WatchNumbers[i] == ANY_TRAP)
				return true;
		}
	}
	return false;
}

bool TroubleListener::removeWatchNumber(unsigned troubleNum)
{
	for (unsigned i = 0; i < m_WatchNumbers.size(); i++) {
		if (m_WatchNumbers[i] == troubleNum) {
			m_WatchNumbers.erase(m_WatchNumbers.begin()+i);
			return true;
		}
	}
	return false;
}

bool TroubleListener::addWatchNumber(unsigned troubleNumber)
{
	for (unsigned i = 0; i < m_WatchNumbers.size(); i++) {
		if (m_WatchNumbers[i] == troubleNumber)
			return false;
	}
	m_WatchNumbers.push_back(troubleNumber);
	return true;
}

bool MemAccessListener::isMatching(const MemAccessEvent* pEv) const
{
	if (!(m_WatchType & pEv->getTriggerAccessType())) {
		return false;
	} else if (m_WatchAddr != ANY_ADDR
	           && (m_WatchAddr >= pEv->getTriggerAddress() + pEv->getTriggerWidth()/8
	           || m_WatchAddr + m_WatchWidth <= pEv->getTriggerAddress())) {
        // FIXME:  getTriggerWidth()  is # bits of accessed and we need to compare if the accessed _address_ is within this watchpoints range bytewise.
        // Ideally one would distinguish between width of address range watched and width of access within an address, but since getTriggerWidth is used very rarely its easier to convert from bits to bytes here.
		return false;
    } else if (m_WatchMemType != ANY_MEMORY && m_WatchMemType != pEv->getMemoryType()) {
        return false;
	} else if (m_CPU != NULL && m_CPU != pEv->getTriggerCPU()) {
		return false;
	}
	return true;
}

bool BPListener::aspaceIsMatching(address_t address_space) const
{
	if (m_Data.getAddressSpace() == ANY_ADDR || m_Data.getAddressSpace() == address_space)
		return true;
	return false;
}

void BPRangeListener::setWatchInstructionPointerRange(address_t start, address_t end)
{
	m_WatchStartAddr = start;
	m_WatchEndAddr = end;
}

bool BPRangeListener::isMatching(const BPEvent* pEv) const
{
	if (!aspaceIsMatching(pEv->getAddressSpace()))
		return false;
	if (m_CPU != NULL && m_CPU != pEv->getTriggerCPU()) {
		return false;
	}
	if ((m_WatchStartAddr != ANY_ADDR && pEv->getTriggerInstructionPointer() < m_WatchStartAddr) ||
		(m_WatchEndAddr != ANY_ADDR && pEv->getTriggerInstructionPointer() > m_WatchEndAddr))
		return false;
	return true;
}

bool BPSingleListener::isMatching(const BPEvent* pEv) const
{
	if (aspaceIsMatching(pEv->getAddressSpace())) {
		if (m_CPU == NULL || m_CPU == pEv->getTriggerCPU()) {
			if (m_WatchInstrPtr == ANY_ADDR || m_WatchInstrPtr == pEv->getTriggerInstructionPointer())
				return true;
		}
	}
	return false;
}

} // end-of-namespace: fail
