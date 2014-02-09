#include <sstream>

#include "PandaController.hpp"
#include "PandaMemory.hpp"
#include "../SALInst.hpp"
#include "../Listener.hpp"


#include "openocd_wrapper.hpp"

#if defined(CONFIG_FIRE_INTERRUPTS)
	#error Firing interrupts not implemented for Pandaboard
#endif

#if defined(CONFIG_SR_RESTORE) || defined(CONFIG_SR_SAVE)
	#error Save/Restore is not yet implemented for Pandaboard
#endif

#if defined(CONFIG_EVENT_IOPORT)
	#error IoPort events not implemented for pandaboard
#endif

namespace fail {

PandaController::PandaController()
	: SimulatorController(new PandaMemoryManager()), m_CurrFlow(NULL)
{
	addCPU(new ConcreteCPU(0));
}

PandaController::~PandaController()
{
	delete m_Mem;
	std::vector<ConcreteCPU*>::iterator it = m_CPUs.begin();
	while (it != m_CPUs.end()) {
		delete *it;
		it = m_CPUs.erase(it);
	}
}

void PandaController::onTimerTrigger(void* thisPtr)
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

bool PandaController::save(const std::string& path)
{
	// TODO

	return true;
}

void PandaController::restore(const std::string& path)
{
	clearListeners();
	// TODO
}

void PandaController::reboot()
{
	clearListeners();

	oocdw_reboot();
}

void PandaController::terminate(int exCode)
{
	oocdw_finish(exCode);
	/*
	 * Resume to let OpenOCD terminate properly
	 * This call does not return!
	 */
	m_Flows.resume();
}

simtime_t PandaController::getTimerTicks()
{
	return oocdw_read_cycle_counter();
}

simtime_t PandaController::getTimerTicksPerSecond()
{
	return 1200*1000*1000;
}

} // end-of-namespace: fail
