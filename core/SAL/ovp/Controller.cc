#include <iostream>
#include "OVPController.hpp"
#include "OVPMemory.hpp"
#include "OVPRegister.hpp"
#include "../../../ovp/OVPStatusRegister.hpp"

namespace sal {

OVPController::OVPController()
	: SimulatorController(new OVPRegisterManager(), new OVPMemoryManager())
{
	//FIXME: This should be obsolete now...
	//set = new UniformRegisterSet(RT_GP);	
	//setStatus = new UniformRegisterSet(RT_ST);	
	//setPC= new UniformRegisterSet(RT_PC);	
}

OVPController::~OVPController()
{
	//FIXME: This should be obsolete now...
	//delete set;
	//delete setStatus;
	//delete setPC;

	// Free memory of OVPRegister objects (actually allocated in make*Register):
	for(RegisterManager::iterator it = m_Regs->begin(); it != m_Regs->end(); it++)
			delete (*it); // free the memory, allocated in the constructor
}

void OVPController::makeGPRegister(int width, void *link, const string& name) {
	// Add general purpose register
	OVPRegister *reg = new OVPRegister(m_currentRegId++, width, link, RT_GP);
	reg->setName(name);
	m_Regs->add(reg);
	// Note: The RegisterManager (aka m_Regs) automatically assigns the 
	//       added registers (herein typed OVPRegister) to their matching
	//       UniformRegisterSet (@see RegisterManager::add).
}


void OVPController::makeSTRegister(Register *reg, const string& name) {
	// Add status register
	reg->setName(name);
	cerr << "Add Status Register: " << reg << endl;
	m_Regs->add(reg);
}

void OVPController::makePCRegister(int width, void *link, const string& name) {
	// Add general purpose register
	OVPRegister *reg = new OVPRegister(m_currentRegId++, width, link, RT_PC);
	reg->setName(name);
	m_Regs->add(reg);
}

//FIXME: This should be obsolete now...
//void OVPController::finishedRegisterCreation() {
//	m_Regs->add(*set);
//	m_Regs->add(*setStatus);
//	m_Regs->add(*setPC);
//}

void OVPController::onInstrPtrChanged(address_t instrPtr)
{
	// make some funny outputs
	unsigned int r0 = m_Regs->getSetOfType(RT_GP)->getRegister(0)->getData();
	OVPRegisterManager *ovp_Regs = (OVPRegisterManager *) m_Regs;
	// FIXME: This cast seems to be invalid: RT_GP vs. OVPStatusRegister (= RT_ST)?
	OVPStatusRegister *rid_st = (OVPStatusRegister *) m_Regs->getSetOfType(RT_GP)->first();
//	cerr << "Addr: " << rid_st << endl;
	unsigned int st = rid_st->getData();
//	cerr << "instrPtr: 0x" << hex << instrPtr << " SP: 0x" << hex << m_Regs->getStackPointer() \
		<< " R0: 0x" << hex << r0 << " ST: 0x" << hex << st  << endl;

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
}

void OVPController::save(const string& path)
{
	//TODO
	//SIM->save_state(path.c_str());
	//bx_gui_c::power_handler();
}

void OVPController::restore(const string& path)
{
	//TODO
	//bx_pc_system.restore_bochs_request = true;
	assert(path.length() > 0 &&
		"FATAL ERROR: Tried to restore state without valid path!");
	//strncpy(bx_pc_system.sr_path, path.c_str(), 512 /*CI_PATH_LENGTH, @see gui/textconfig.cc*/);
}

void OVPController::reboot()
{
	//TODO
	//bx_pc_system.Reset(BX_RESET_HARDWARE);
	//bx_gui_c::reset_handler();//TODO: leider protected, so geht das also nicht...
}

void OVPController::toPreviousCtx()
{
	// TODO
}

void OVPController::terminate(int exCode){

}


void OVPController::fireTimer(uint32_t){

};

};
