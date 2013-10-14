#ifndef __BREAKPOINT_BUFFER_HPP__
#define __BREAKPOINT_BUFFER_HPP__

#include "BufferInterface.hpp"

// TODOs:
//  - Make these implementations even faster (see below: continue PerfVecSortedSingleBP).

namespace fail {

class BPEvent;

/**
 * Concrete implementation for the \c BPSingleListener class.
 */
class PerfVectorBreakpoints : public DefPerfVector<BPEvent> {
public:
	ResultSet& gather(BPEvent* pData);
};

/**
 * \class PerfVecSortedSingleBP
 *
 * This class implements a faster mechanism to store BPSingleListener
 * based on binary search on their corresponding instruction pointer.
 */
/*
class PerfVecSortedSingleBP : public PerfVectorBreakpoints {
private:
	SimulatorController* m_Sim;
public:
	PerfVecSortedSingleBP(SimulatorController* pSim) : m_Sim(pSim) { }
	**
	 * TODO.
	 * @warning The method expects that \c idx is a valid index within the main
	 * buffer-list. Therefore we are allowed to call \c SimulatorController::dereference().
	 * Additionally, the indexed listener is expected to be of type \c BPListener*.
	 *
	void add(index_t idx)
	{
		address_t watchIP = static_cast<BPSingleListener*>(m_Sim->dereference(idx))->getWatchInstructionPointer();
		// Keep the indices sorted in ascending order regarding their instr. ptr.
		// Search for the slot to insert the element (skip lower IPs):
		int pos = binarySearch(m_BufList, 0, m_BufList.size()-1, idx);
		assert(pos < 0 && "FATAL ERROR: Element already inserted!");
		m_BufList.insert(it + (pos * (-1) + 1), idx);
//		for (std::vector<index_t>::iterator it = m_BufList.begin(); it != m_BufList.end(); ++it) {
//			if (static_cast<BPSingleListener*>(
//			    m_Sim->dereference(*it))->getWatchInstructionPointer() > watchIP) {
//				m_BufList.insert(it, idx); // Insert the element before "it"
//				break;
//			}
		}
	}

	static bool CompareInstrPtr(index_t arg1, index_t arg2, void* pStuff)
	{
		SimulatorController* pSim = static_cast<SimulatorController*>(pStuff);
		#define VAL(idx) \
		static_cast<BPSingleListener*>(pSim->dereference(idx))->getWatchInstructionPointer()
		return (VAL(arg1) > VAL(arg2);
	}

	**
	 * Searches vec[first]...vec[last] for \c key. Returns the index of the matching
	 * element if it finds key, otherwise -(index where it could be inserted)-1.
	 * @param vec array of sorted (ascending) values.
	 * @param first lower  subscript bound
	 * @param last upper subscript bound
	 * @param key value to search for
	 * @param pfnCmp a function pointer to a comparison function, returning \c true
	 *        if the first argument is greater than the second, otherwise \c false.
	 *        The last parameter equals the last parameter provided by calling this
	 *        method. It can be used to pass additional data.
	 * @return index of key, or -insertion_position -1 if key is not in the array.
	 *         This value can easily be transformed into the position to insert it.
	 *
	int binarySearch(const std::vector<index_t>& vec, index_t first, index_t last, index_t key, bool (*pfnCmp)(index_t, index_t, void*), void* pStuff = NULL)
	{

		while (first <= last) {
			int mid = (first + last) / 2; // compute mid point.
			if (VAL(key) > VAL(vec[mid]))
				first = mid + 1; // repeat search in top half.
			else if (VAL(key) < VAL(vec[mid]))
				last = mid - 1; // repeat search in bottom half.
			else
				return mid; // found it. return position
		}
		return -(first + 1); // failed to find key
	}
	void remove(index_t idx)
	{
		int pos = binarySearch(m_BufList, 0, m_BufList.size()-1, idx, CompareInstrPtr, m_Sim);
		if (pos >= 0)
			m_BufList.erase(m_BufList.begin() + pos);
	}
	ResultSet& gather(BPEvent* pData)
	{
		// TODO: Improve this by using binary search, too!
		ResultSet res;
		// Search for all indices of matching listener objects:
		for (std::vector<index_t>::iterator it = m_BufList.begin(); it != m_BufList.end(); ++it) {
			BPListener* pLi = static_cast<BPListener*>(simulator.dereference(*it));
			if (pLi->isMatching(pData)) {
				// Update trigger IPtr:
				pLi->setTriggerInstructionPointer(pData->getTriggerInstructionPointer());
				res.add(*it);
			}
		}
		return res;
	}
};
*/

} // end-of-namespace: fail

#endif // __BREAKPOINT_BUFFER_HPP__
