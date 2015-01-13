#include "WatchpointBuffer.hpp"
#include "../Listener.hpp"
#include "../Event.hpp"
#include "../SALInst.hpp"

namespace fail {

// FIXME: Can not be inlined this way!
ResultSet& PerfVectorWatchpoints::gather(MemAccessEvent* pData)
{
	static ResultSet res;
	res.clear(); // FIXME: This should not free the memory of the underlying std::vector.
	// Search for all indices of matching listener objects:
	for (std::vector<index_t>::iterator it = m_BufList.begin(); it != m_BufList.end(); ++it) {
		MemAccessListener* pmal = static_cast<MemAccessListener*>(simulator.dereference(*it));
		if (pmal->isMatching(pData)) {
			// Update trigger data:
			pmal->setTriggerInstructionPointer(pData->getTriggerInstructionPointer());
			pmal->setTriggerAddress(pData->getTriggerAddress());
			pmal->setTriggerWidth(pData->getTriggerWidth());
			pmal->setTriggerAccessType(pData->getTriggerAccessType());
			pmal->setTriggerCPU(pData->getTriggerCPU());
			res.add(pmal);
		}
	}
	return res;
}

} // end-of-namespace: fail
