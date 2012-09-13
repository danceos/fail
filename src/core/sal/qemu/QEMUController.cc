#include <sstream>

#include "QEMUController.hpp"
#include "QEMUMemory.hpp"
#include "QEMURegister.hpp"
#include "../Register.hpp"
#include "../SALInst.hpp"

namespace fail {

QEMUController::QEMUController()
	: SimulatorController(new QEMURegisterManager(), new QEMUMemoryManager()),
	  m_cpuenv(0)
{
	// TODO: probably do additional RegisterManager initializations
}

QEMUController::~QEMUController()
{
	delete m_Regs;
	delete m_Mem;
}

// FIXME: copied from BochsController; remove redundancy!
void QEMUController::onIOPort(unsigned char data, unsigned port, bool out) {
	// Check for active IOPortListeners:
	io_cache_t &buffer_cache = m_LstList.getIOBuffer();
	io_cache_t::iterator it = buffer_cache.begin();
	while (it != buffer_cache.end()) {
		IOPortListener* pIOPt = (*it);
		if (pIOPt->isMatching(port, out)) {
			pIOPt->setData(data);
			it = buffer_cache.makeActive(m_LstList, it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		it++;
	}
	m_LstList.triggerActiveListeners();
}

void QEMUController::onTimerTrigger(TimerListener *pli)
{
	// FIXME: The timer logic can be modified to use only one timer in QEMU.
	//        (For now, this suffices.)

	// Check for a matching TimerListener. (In fact, we are only
	// interessted in the iterator pointing at pli.)
	ListenerManager::iterator it = std::find(m_LstList.begin(),
	                                         simulator.m_LstList.end(), pli);
	// TODO: This has O(|m_LstList|) time complexity. We can further improve this
	//       by creating a method such that makeActive(pli) works as well,
	//       reducing the time complexity to O(1).
	m_LstList.makeActive(it);
	m_LstList.triggerActiveListeners();
}

} // end-of-namespace: fail
