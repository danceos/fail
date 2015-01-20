#include "BreakpointBuffer.hpp"
#include "../SALInst.hpp"
#include "../Listener.hpp"

namespace fail {

// FIXME: Can not be inlined this way!
ResultSet& PerfVectorBreakpoints::gather(BPEvent* pData)
{
	static ResultSet res;
	res.clear(); // FIXME: This should not free the memory of the underlying std::vector.
	// Search for all indices of matching listener objects:
	for (std::vector<index_t>::iterator it = m_BufList.begin(); it != m_BufList.end(); ++it) {
		BPListener* pLi = static_cast<BPListener*>(simulator.dereference(*it));
		if (pLi->isMatching(pData)) {
			// Update trigger IPtr:
			pLi->setTriggerInstructionPointer(pData->getTriggerInstructionPointer());
			pLi->setTriggerCPU(pData->getTriggerCPU());
			res.add(pLi);
		}
	}
	return res;
}

} // end-of-namespace: fail
