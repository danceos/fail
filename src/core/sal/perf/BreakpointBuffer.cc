#include "BreakpointBuffer.hpp"
#include "../SALInst.hpp"

namespace fail {

// FIXME: can not be inlined this way
ResultSet& PerfVectorBreakpoints::gather(BPEvent* pData)
{
	static ResultSet res;
	res.clear();
	// Search for all indices of matching listener objects:
	for(std::vector<index_t>::iterator it = m_BufList.begin(); it != m_BufList.end(); ++it) {
		BPListener* pLi = static_cast<BPListener*>(simulator.dereference(*it));
		if (pLi->isMatching(pData)) {
			// Update trigger IPtr:
			pLi->setTriggerInstructionPointer(pData->getTriggerInstructionPointer());
			res.add(*it);
		}
	}
	return res;
}

} // end-of-namespace: fail
