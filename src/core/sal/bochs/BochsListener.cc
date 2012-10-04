#include "BochsListener.hpp"
#include "../Listener.hpp"
#include "../ListenerManager.hpp"

namespace fail {

void onTimerTrigger(void* thisPtr)
{
	// FIXME: The timer logic can be modified to use only one timer in Bochs.
	//        (For now, this suffices.)
	TimerListener* pli = static_cast<TimerListener*>(thisPtr);
	// Check for a matching TimerListener. (In fact, we are only
	// interessted in the iterator pointing at pli.)
	ListenerManager::iterator it = std::find(simulator.m_LstList.begin(),
											simulator.m_LstList.end(), pli);
	// TODO: This has O(|m_LstList|) time complexity. We can further improve this
	//       by creating a method such that makeActive(pli) works as well,
	//       reducing the time complexity to O(1).
	simulator.m_LstList.makeActive(it);
	simulator.m_LstList.triggerActiveListeners();
}

} // end-of-namespace: fail
