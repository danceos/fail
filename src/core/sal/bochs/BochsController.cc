#include <sstream>

#include "BochsController.hpp"
#include "BochsMemory.hpp"
#include "../SALInst.hpp"
#include "../Listener.hpp"

namespace fail {

#ifdef DANCEOS_RESTORE
bx_bool restore_bochs_request = false;
bx_bool restore_bochs_finished = false;
bx_bool save_bochs_request    = false;
std::string  sr_path          = "";
#endif

bx_bool reboot_bochs_request        = false;
bx_bool interrupt_injection_request = false;

BochsController::BochsController()
	: SimulatorController(new BochsMemoryManager()),
	  m_CurrFlow(NULL), m_CPUContext(NULL), m_CurrentInstruction(NULL)
{
	for (unsigned i = 0; i < BX_SMP_PROCESSORS; i++)
		addCPU(new ConcreteCPU(i));
}

BochsController::~BochsController()
{
	delete m_Mem;
	std::vector<ConcreteCPU*>::iterator it = m_CPUs.begin();
	while (it != m_CPUs.end()) {
		delete *it;
		it = m_CPUs.erase(it);
	}
}

void BochsController::updateBPEventInfo(BX_CPU_C *context, bxInstruction_c *instr)
{
	assert(context != NULL && "FATAL ERROR: Bochs internal member was NULL (not expected)!");
	assert(instr != NULL && "FATAL ERROR: Bochs internal member was NULL (not expected)!");
	m_CPUContext = context;
	m_CurrentInstruction = instr;
}

void BochsController::onTimerTrigger(void* thisPtr)
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

void BochsController::onIOPort(ConcreteCPU* cpu, unsigned char data, unsigned port, bool out) {
	// Check for active IOPortListeners:
	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) {
		BaseListener* pLi = *it;
		IOPortListener* pIOPt = dynamic_cast<IOPortListener*>(pLi);
		if (pIOPt != NULL && pIOPt->isMatching(port, out)) {
			pIOPt->setData(data);
			pIOPt->setTriggerCPU(cpu);
			it = m_LstList.makeActive(it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		it++;
	}
	m_LstList.triggerActiveListeners();
}

bool BochsController::save(const std::string& path)
{
	int stat;

	stat = mkdir(path.c_str(), 0777);
	if (!(stat == 0 || errno == EEXIST)) {
		return false;
		// std::cout << "[FAIL] Can not create target-directory to save!" << std::endl;
		// TODO: (Non-)Verbose-Mode? Log-level? Maybe better: use return value to indicate failure?
	}

	save_bochs_request = true;
	BX_CPU(0)->async_event |= 1;
	sr_path = path;
	m_CurrFlow = m_Flows.getCurrent();
	m_Flows.resume();
	return true;
}

void BochsController::saveDone()
{
	save_bochs_request = false;
	m_Flows.toggle(m_CurrFlow);
}

void BochsController::restore(const std::string& path)
{
	clearListeners();
	restore_bochs_request = true;
	restore_bochs_finished = false;
	BX_CPU(0)->async_event |= 1;
	sr_path = path;
	m_CurrFlow = m_Flows.getCurrent();
	m_Flows.resume();
}

void BochsController::restoreDone()
{
	m_Flows.toggle(m_CurrFlow);
}

void BochsController::reboot()
{
	clearListeners();
	reboot_bochs_request = true;
	BX_CPU(0)->async_event |= 1;
	m_CurrFlow = m_Flows.getCurrent();
	m_Flows.resume();
}

void BochsController::rebootDone()
{
	reboot_bochs_request = false;
	m_Flows.toggle(m_CurrFlow);
}

void BochsController::fireInterrupt(unsigned irq)
{
	interrupt_injection_request = true;
	interrupt_to_fire = irq;
	// FIXME needed?  BX_CPU(0)->async_event |= 1;
	m_CurrFlow = m_Flows.getCurrent();
	m_Flows.resume();
}

void BochsController::fireInterruptDone()
{
	interrupt_injection_request = false;
	m_Flows.toggle(m_CurrFlow);
}

const std::string& BochsController::getMnemonic() const
{
	static std::string str;
	bxInstruction_c* pInstr = getCurrentInstruction();
	assert(pInstr != NULL && "FATAL ERROR: Bochs internal member was NULL (not expected)!");
	const char* pszName = get_bx_opcode_name(pInstr->getIaOpcode());
	if (pszName != NULL)
		str = pszName;
	else
		str.clear();
	return str;
}

ConcreteCPU& BochsController::detectCPU(BX_CPU_C* pCPU) const
{
	unsigned i = 0;
#if BX_SUPPORT_SMP
	for (; i < BX_SMP_PROCESSORS; i++) {
		if (BX_CPU_C[i] == pCPU) // cmp this ptr with all possible CPU objects
			break; // index "i" found! -> stop!
	}
#endif
	return getCPU(i);
}

} // end-of-namespace: fail
