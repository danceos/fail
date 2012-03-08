#include "BochsController.hpp"
#include "BochsMemory.hpp"
#include "BochsRegister.hpp"
#include "../Register.hpp"

namespace sal
{

bx_bool restore_bochs_request = false;
bx_bool save_bochs_request = false;
bx_bool reboot_bochs_request = false;
std::string  sr_path = "";

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
	pFlagReg->setName("EIP");
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

void BochsController::onInstrPtrChanged(address_t instrPtr)
{
  #ifdef DEBUG
	if(m_Regularity != 0 && ++m_Counter % m_Regularity == 0)
		(*m_pDest) << "0x" << std::hex << instrPtr;
  #endif
	// Check for active breakpoint-events:
	fi::EventList::iterator it = m_EvList.begin();
	while(it != m_EvList.end())
	{
		// FIXME: Performance verbessern (dazu muss entsprechend auch die Speicherung
		// in EventList(.cc|.hpp) angepasst bzw. verbessert werden).
		fi::BPEvent* pEvBreakpt = dynamic_cast<fi::BPEvent*>(*it);
		if(pEvBreakpt && (instrPtr == pEvBreakpt->getWatchInstructionPointer() ||
		   pEvBreakpt->getWatchInstructionPointer() == fi::ANY_ADDR))
		{
			pEvBreakpt->setTriggerInstructionPointer(instrPtr);
			it = m_EvList.makeActive(it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		fi::BPRangeEvent* pEvRange = dynamic_cast<fi::BPRangeEvent*>(*it);
		if(pEvRange && pEvRange->isMatching(instrPtr))
		{
			pEvBreakpt->setTriggerInstructionPointer(instrPtr);
			it = m_EvList.makeActive(it);
			continue; // dito.
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

void BochsController::terminate(int exCode)
{
	// Attention: This could cause Problems, e.g. because of non-closed sockets
	std::cout << "[FAIL] Exit called by experiment with exit code: " << exCode << std::endl;
	// TODO: (Non-)Verbose-Mode?
	exit(exCode);
}

} // end-of-namespace: sal
