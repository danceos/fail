#include "Event.hpp"
#include "SALInst.hpp"

namespace fail {

event_id_t BaseEvent::m_Counter = 0;

bool TroubleEvent::isMatching(unsigned troubleNum) const
{
	for (unsigned i = 0; i < m_WatchNumbers.size(); i++) {
		if (m_WatchNumbers[i] == troubleNum ||
		   m_WatchNumbers[i] == ANY_TRAP)
			return true;
	}
	return false;
}

bool TroubleEvent::removeWatchNumber(unsigned troubleNum) 
{
	for (unsigned i = 0; i < m_WatchNumbers.size(); i++) {
		if (m_WatchNumbers[i] == troubleNum) {
			m_WatchNumbers.erase(m_WatchNumbers.begin()+i);
			return true;
		}
	}
	return false;
}

bool TroubleEvent::addWatchNumber(unsigned troubleNumber) 
{
	for (unsigned i = 0; i < m_WatchNumbers.size(); i++) {
		if (m_WatchNumbers[i] == troubleNumber)
			return false;
	}	
	m_WatchNumbers.push_back(troubleNumber);
	return true;
}

bool MemAccessEvent::isMatching(address_t addr, accessType_t accesstype) const
{
	if (!(m_WatchType & accesstype))
		return false;
	else if (m_WatchAddr != addr &&
			   m_WatchAddr != ANY_ADDR)
		return false;
	return true;
}

bool BPEvent::aspaceIsMatching(address_t aspace) const
{
	if (m_CR3 == ANY_ADDR || m_CR3 == aspace)
		return true;
	return false;
}

void BPRangeEvent::setWatchInstructionPointerRange(address_t start, address_t end)
{
	m_WatchStartAddr = start;
	m_WatchEndAddr = end;
}

bool BPRangeEvent::isMatching(address_t addr, address_t aspace) const
{
	if (!aspaceIsMatching(aspace))
		return false;
	if ((m_WatchStartAddr != ANY_ADDR && addr < m_WatchStartAddr) ||
		(m_WatchEndAddr != ANY_ADDR && addr > m_WatchEndAddr))
		return false;
	return true;
}

bool BPSingleEvent::isMatching(address_t addr, address_t aspace) const
{
	if (aspaceIsMatching(aspace)) {
		if (m_WatchInstrPtr == ANY_ADDR || m_WatchInstrPtr == addr)
			return true;
	}
	return false;
}

} // end-of-namespace: fail
