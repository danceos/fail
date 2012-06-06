#include <sstream>

#include "BochsController.hpp"
#include "BochsMemory.hpp"
#include "BochsRegister.hpp"
#include "../Register.hpp"
#include "../SALInst.hpp"

namespace sal
{

#ifdef DANCEOS_RESTORE
bx_bool restore_bochs_request = false;
bx_bool save_bochs_request    = false;
string  sr_path               = "";
#endif

bx_bool reboot_bochs_request        = false;
bx_bool interrupt_injection_request = false;
int     interrupt_to_fire           = -1;

BochsController::BochsController()
	: SimulatorController(new BochsRegisterManager(), new BochsMemoryManager())
{
	// -------------------------------------
	// Add the general purpose register:
  #if BX_SUPPORT_X86_64
	// -- 64 bit register --
  	const string names[] = { "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI",
  	                         "RDI", "R8", "R9", "R10", "R11", "R12", "R13",
  	                         "R14", "R15" };
	for(unsigned short i = 0; i < 16; i++)
	{
		BxGPReg* pReg = new BxGPReg(i, 64, &(BX_CPU(0)->gen_reg[i].rrx));
		pReg->setName(names[i]);
		m_Regs->add(pReg);
	}
  #else
  	// -- 32 bit register --
  	const string names[] = { "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI",
							 "EDI" };
	for(unsigned short i = 0; i < 8; i++)
	{
		BxGPReg* pReg = new BxGPReg(i, 32, &(BX_CPU(0)->gen_reg[i].dword.erx));
		pReg->setName(names[i]);
		m_Regs->add(pReg);
	}
  #endif // BX_SUPPORT_X86_64
  #ifdef DEBUG
	m_Regularity = 0; // disabled
	m_Counter = 0;
	m_pDest = NULL;
  #endif
	// -------------------------------------
	// Add the Program counter register:
  #if BX_SUPPORT_X86_64
	BxPCReg* pPCReg = new BxPCReg(RID_PC, 64, &(BX_CPU(0)->gen_reg[BX_64BIT_REG_RIP].rrx));
	pPCReg->setName("RIP");
  #else
    BxPCReg* pPCReg = new BxPCReg(RID_PC, 32, &(BX_CPU(0)->gen_reg[BX_32BIT_REG_EIP].dword.erx));
	pPCReg->setName("EIP");
  #endif // BX_SUPPORT_X86_64
    // -------------------------------------
	// Add the Status register (x86 cpu FLAGS):
	BxFlagsReg* pFlagReg = new BxFlagsReg(RID_FLAGS, reinterpret_cast<regdata_t*>(&(BX_CPU(0)->eflags)));
	// Note: "eflags" is (always) of type Bit32u which matches the regdata_t only in
	//       case of the 32 bit version (id est !BX_SUPPORT_X86_64). Therefor we need
	//       to ensure to assign only 32 bit to the Bochs internal register variable
	//       (see SAL/bochs/BochsRegister.hpp, setData) if we are in 64 bit mode.
	pFlagReg->setName("FLAGS");
	m_Regs->add(pFlagReg);
	m_Regs->add(pPCReg);
}

BochsController::~BochsController()
{
	for(RegisterManager::iterator it = m_Regs->begin(); it != m_Regs->end(); it++)
			delete (*it); // free the memory, allocated in the constructor
	m_Regs->clear();
	delete m_Regs;
	delete m_Mem;
}

#ifdef DEBUG
void BochsController::dbgEnableInstrPtrOutput(unsigned regularity, std::ostream* dest)
{
	m_Regularity = regularity;
	m_pDest = dest;
	m_Counter = 0;
}
#endif // DEBUG

void BochsController::onInstrPtrChanged(address_t instrPtr, address_t address_space)
{
#if 0
	//the original code - performs magnitudes worse than
	//the code below and is responsible for most (~87 per cent)
	//of the slowdown of FailBochs
#ifdef DEBUG
	if(m_Regularity != 0 && ++m_Counter % m_Regularity == 0)
		(*m_pDest) << "0x" << std::hex << instrPtr;
#endif
	// Check for active breakpoint-events:
	fi::EventList::iterator it = m_EvList.begin();
	while(it != m_EvList.end())
	{
		// FIXME: Maybe we need to improve the performance of this check.
		fi::BPEvent* pEvBreakpt = dynamic_cast<fi::BPEvent*>(*it);
		if(pEvBreakpt && pEvBreakpt->isMatching(instrPtr, address_space))
		{
			pEvBreakpt->setTriggerInstructionPointer(instrPtr);
			it = m_EvList.makeActive(it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		it++;
	}
	m_EvList.fireActiveEvents();
#endif
	//this code is highly optimised for the average case, so it me appear a bit ugly
	bool do_fire = false;
	unsigned i = 0;
	fi::BufferCache<fi::BPEvent*> *buffer_cache = m_EvList.getBPBuffer();
	while(i < buffer_cache->get_count()) {
		fi::BPEvent *pEvBreakpt = buffer_cache->get(i);
		if(pEvBreakpt->isMatching(instrPtr, address_space)) {
			pEvBreakpt->setTriggerInstructionPointer(instrPtr);

			//transition to STL: find the element we are working on in the Event List
			fi::EventList::iterator it = std::find(m_EvList.begin(), m_EvList.end(), pEvBreakpt);
			it = m_EvList.makeActive(it);
			//find out how much elements need to be skipped to get in sync again
			//(should be one or none, the loop is just to make sure)
			for(unsigned j = i; j < buffer_cache->get_count(); j++) {
				if(buffer_cache->get(j) == (*it)) {
					i = j;
					break;
				}
			}
			// we now know we need to fire the active events - usually we do not have to
			do_fire = true;
			// "i" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip loop  increment
		}
		i++;
	}
	if(do_fire)
		m_EvList.fireActiveEvents();
	// Note: SimulatorController::onBreakpointEvent will not be invoked in this
	//       implementation.
}

void BochsController::onIOPortEvent(unsigned char data, unsigned port, bool out) {
	// Check for active breakpoint-events:
	fi::EventList::iterator it = m_EvList.begin();
	while(it != m_EvList.end())
	{
		// FIXME: Maybe we need to improve the performance of this check.
		fi::IOPortEvent* pIOPt = dynamic_cast<fi::IOPortEvent*>(*it);
		if(pIOPt && pIOPt->isMatching(port, out))
		{
			pIOPt->setData(data);
			it = m_EvList.makeActive(it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		it++;
	}
	m_EvList.fireActiveEvents();
	// Note: SimulatorController::onBreakpointEvent will not be invoked in this
	//       implementation.
}

void BochsController::save(const string& path)
{
	int stat;
	
	stat = mkdir(path.c_str(), 0777);
	if(!(stat == 0 || errno == EEXIST))
		std::cout << "[FAIL] Can not create target-directory to save!" << std::endl;
		// TODO: (Non-)Verbose-Mode? Maybe better: use return value to indicate failure?
	
	save_bochs_request = true;
	sr_path = path;
	m_CurrFlow = m_Flows.getCurrent();
	m_Flows.resume();
}

void BochsController::saveDone()
{
	save_bochs_request = false;
	m_Flows.toggle(m_CurrFlow);
}

void BochsController::restore(const string& path)
{
	clearEvents();
	restore_bochs_request = true;
	sr_path = path;
	m_CurrFlow = m_Flows.getCurrent();
	m_Flows.resume();
}

void BochsController::restoreDone()
{
	restore_bochs_request = false;
	m_Flows.toggle(m_CurrFlow);
}

void BochsController::reboot()
{
	clearEvents();
	reboot_bochs_request = true;
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
	m_CurrFlow = m_Flows.getCurrent();
	m_Flows.resume();
}

void BochsController::fireInterruptDone()
{
	interrupt_injection_request = false;
	m_Flows.toggle(m_CurrFlow);
}

void BochsController::m_onTimerTrigger(void* thisPtr)
{
	// FIXME: The timer logic can be modified to use only one timer in Bochs.
	//        (For now, this suffices.)
	fi::TimerEvent* pTmEv = static_cast<fi::TimerEvent*>(thisPtr);
	// Check for a matching TimerEvent. (In fact, we are only
	// interessted in the iterator pointing at pTmEv.):
	fi::EventList::iterator it = std::find(simulator.m_EvList.begin(),
											simulator.m_EvList.end(), pTmEv);
	// TODO: This has O(|m_EvList|) time complexity. We can further improve this
	//       by creating a method such that makeActive(pTmEv) works as well,
	//       reducing the time complexity to O(1).
	simulator.m_EvList.makeActive(it);
	simulator.m_EvList.fireActiveEvents();
}

timer_id_t BochsController::m_registerTimer(fi::TimerEvent* pev)
{
	assert(pev != NULL);
	return static_cast<timer_id_t>(
		bx_pc_system.register_timer(pev, m_onTimerTrigger, pev->getTimeout(), !pev->getOnceFlag(),
									1/*start immediately*/, "Fail*: BochsController"/*name*/));
}

bool BochsController::m_unregisterTimer(fi::TimerEvent* pev)
{
	assert(pev != NULL);
	bx_pc_system.deactivate_timer(static_cast<unsigned>(pev->getId()));
	return bx_pc_system.unregisterTimer(static_cast<unsigned>(pev->getId()));
}

bool BochsController::onEventAddition(fi::BaseEvent* pev)
{
	fi::TimerEvent* tmev;
	// Register the timer event in the Bochs simulator:
	if ((tmev = dynamic_cast<fi::TimerEvent*>(pev)) != NULL) {
		tmev->setId(m_registerTimer(tmev));
		if(tmev->getId() == -1)
			return false; // unable to register the timer (error in Bochs' function call)
	}
	// Note: Maybe more stuff to do here for other event types.
	return true;
}

void BochsController::onEventDeletion(fi::BaseEvent* pev)
{
	fi::TimerEvent* tmev;
	// Unregister the time event:
	if ((tmev = dynamic_cast<fi::TimerEvent*>(pev)) != NULL) {
		m_unregisterTimer(tmev);
	}
	// Note: Maybe more stuff to do here for other event types.
}

void BochsController::onEventTrigger(fi::BaseEvent* pev)
{
	fi::TimerEvent* tmev;
	// Unregister the time event, if once-flag is true:
	if ((tmev = dynamic_cast<fi::TimerEvent*>(pev)) != NULL) {
		if (tmev->getOnceFlag()) // deregister the timer (timer = single timeout)
			m_unregisterTimer(tmev);
		else // re-add the event (repetitive timer), tunneling the onEventAddition-handler
			m_EvList.add(tmev, tmev->getParent());
	}
	// Note: Maybe more stuff to do here for other event types.
}

} // end-of-namespace: sal
