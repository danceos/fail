#include <sstream>

#include "BochsController.hpp"
#include "BochsMemory.hpp"
#include "BochsRegister.hpp"
#include "../Register.hpp"
#include "../SALInst.hpp"

namespace fail {

#ifdef DANCEOS_RESTORE
bx_bool restore_bochs_request = false;
bx_bool save_bochs_request    = false;
std::string  sr_path          = "";
#endif

bx_bool reboot_bochs_request        = false;
bx_bool interrupt_injection_request = false;

BochsController::BochsController()
	: SimulatorController(new BochsRegisterManager(), new BochsMemoryManager()),
	  m_CPUContext(NULL), m_CacheEntry(NULL)
{
	// -------------------------------------
	// Add the general purpose register:
  #if BX_SUPPORT_X86_64
	// -- 64 bit register --
  	const  std::string names[] = { "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI",
  	                               "RDI", "R8", "R9", "R10", "R11", "R12", "R13",
  	                               "R14", "R15" };
	for (unsigned short i = 0; i < 16; i++) {
		BxGPReg* pReg = new BxGPReg(i, 64, &(BX_CPU(0)->gen_reg[i].rrx));
		pReg->setName(names[i]);
		m_Regs->add(pReg);
	}
  #else
  	// -- 32 bit register --
  	const std::string names[] = { "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI",
							      "EDI" };
	for (unsigned short i = 0; i < 8; i++) {
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
	for (RegisterManager::iterator it = m_Regs->begin(); it != m_Regs->end(); it++)
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

void BochsController::onBreakpoint(address_t instrPtr, address_t address_space)
{
#ifdef DEBUG
	if (m_Regularity != 0 && ++m_Counter % m_Regularity == 0)
		(*m_pDest) << "0x" << std::hex << instrPtr;
#endif
	bool do_fire = false;
	// Check for active breakpoint-events:
	ListenerManager::iterator it = m_LstList.begin();
	BPEvent tmp(instrPtr, address_space);
	while (it != m_LstList.end()) {
		BaseListener* pLi = *it;
		BPListener* pBreakpt = dynamic_cast<BPListener*>(pLi);
		if (pBreakpt != NULL && pBreakpt->isMatching(&tmp)) {
			pBreakpt->setTriggerInstructionPointer(instrPtr);
			it = m_LstList.makeActive(it);
			do_fire = true;
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		it++;
	}
	if (do_fire)
		m_LstList.triggerActiveListeners();
	// Note: SimulatorController::onBreakpoint will not be invoked in this
	//       implementation.
}

void BochsController::updateBPEventInfo(BX_CPU_C *context, bxICacheEntry_c *cacheEntry)
{
	assert(context != NULL && "FATAL ERROR: Bochs internal member was NULL (not expected)!");
	assert(cacheEntry != NULL && "FATAL ERROR: Bochs internal member was NULL (not expected)!");
	m_CPUContext = context;
	m_CacheEntry = cacheEntry;
}

void BochsController::onIOPort(unsigned char data, unsigned port, bool out) {
	// Check for active IOPortListeners:
	ListenerManager::iterator it = m_LstList.begin();
	while (it != m_LstList.end()) {
		BaseListener* pLi = *it;
		IOPortListener* pIOPt = dynamic_cast<IOPortListener*>(pLi);
		if (pIOPt != NULL && pIOPt->isMatching(port, out)) {
			pIOPt->setData(data);
			it = m_LstList.makeActive(it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		it++;
	}
	m_LstList.triggerActiveListeners();
}

void BochsController::save(const std::string& path)
{
	int stat;
	
	stat = mkdir(path.c_str(), 0777);
	if (!(stat == 0 || errno == EEXIST))
		std::cout << "[FAIL] Can not create target-directory to save!" << std::endl;
		// TODO: (Non-)Verbose-Mode? Log-level? Maybe better: use return value to indicate failure?
	
	save_bochs_request = true;
	BX_CPU(0)->async_event |= 1;
	sr_path = path;
	m_CurrFlow = m_Flows.getCurrent();
	m_Flows.resume();
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
	BX_CPU(0)->async_event |= 1;
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

const std::string& BochsController::getMnemonic() const
{
	static std::string str;
	bxInstruction_c* pInstr = getICacheEntry()->i;
	assert(pInstr != NULL && "FATAL ERROR: Bochs internal member was NULL (not expected)!");
	const char* pszName = get_bx_opcode_name(pInstr->getIaOpcode());
	if (pszName != NULL)
		str = pszName;
	else
		str.clear();
	return str;
}

} // end-of-namespace: fail
