#include <iostream>

#include "OVPController.hpp"
#include "OVPMemory.hpp"
#include "OVPRegister.hpp"
#include "ovp/OVPStatusRegister.hpp"

namespace fail {

OVPController::OVPController()
	: SimulatorController(new OVPRegisterManager(), new OVPMemoryManager())
{
	// FIXME: This should be obsolete now...
//	set = new UniformRegisterSet(RT_GP);	
//	setStatus = new UniformRegisterSet(RT_ST);	
//	setPC = new UniformRegisterSet(RT_PC);	
}

OVPController::~OVPController()
{
	// FIXME: This should be obsolete now...
//	delete set;
//	delete setStatus;
//	delete setPC;

	// Free memory of OVPRegister objects (actually allocated in make*Register):
	for (RegisterManager::iterator it = m_Regs->begin(); it != m_Regs->end(); it++)
			delete (*it); // free the memory, allocated in the constructor
}

void OVPController::makeGPRegister(int width, void *link, const string& name)
{
	// Add general purpose register
	OVPRegister *reg = new OVPRegister(m_currentRegId++, width, link, RT_GP);
	reg->setName(name);
	m_Regs->add(reg);
	// Note: The RegisterManager (aka m_Regs) automatically assigns the 
	//       added registers (herein typed OVPRegister) to their matching
	//       UniformRegisterSet (@see RegisterManager::add).
}


void OVPController::makeSTRegister(Register *reg, const string& name)
{
	// Add status register
	reg->setName(name);
	cerr << "Add Status Register: " << reg << endl;
	m_Regs->add(reg);
}

void OVPController::makePCRegister(int width, void *link, const string& name)
{
	// Add general purpose register
	OVPRegister *reg = new OVPRegister(m_currentRegId++, width, link, RT_PC);
	reg->setName(name);
	m_Regs->add(reg);
}

// FIXME: This should be obsolete now...
/*
void OVPController::finishedRegisterCreation()
{
	m_Regs->add(*set);
	m_Regs->add(*setStatus);
	m_Regs->add(*setPC);
}
*/

void OVPController::onInstrPtrChanged(address_t instrPtr)
{
	// make some funny outputs
	unsigned int r0 = m_Regs->getSetOfType(RT_GP)->getRegister(0)->getData();
	OVPRegisterManager *ovp_Regs = (OVPRegisterManager*) m_Regs;
	// FIXME: This cast seems to be invalid: RT_GP vs. OVPStatusRegister (= RT_ST)?
	OVPStatusRegister *rid_st = (OVPStatusRegister*) m_Regs->getSetOfType(RT_GP)->first();
//	cerr << "Addr: " << rid_st << endl;
	unsigned int st = rid_st->getData();

//	save("/srv/scratch/sirozipp/test.txt");

//	ovpplatform.setPC(0x123);

//	restore("/srv/scratch/sirozipp/test.txt");
//	cerr << "instrPtr: 0x" << hex << instrPtr << " SP: 0x" << hex << m_Regs->getStackPointer() \
//		 << " R0: 0x" << hex << r0 << " ST: 0x" << hex << st  << endl;

	// Check for active breakpoint-Listeners:
	ListenerManager::iterator it = m_EvList.begin();
	while (it != m_EvList.end()) {
		// FIXME: Performance verbessern (dazu muss entsprechend auch die Speicherung
		// in ListenerManager(.cc|.hpp) angepasst bzw. verbessert werden).
		BPSingleListener* pEvBreakpt = dynamic_cast<BPSingleListener*>(*it);
		if (pEvBreakpt && (instrPtr == pEvBreakpt->getWatchInstructionPointer() ||
		    pEvBreakpt->getWatchInstructionPointer() == ANY_ADDR)) {
			pEvBreakpt->setTriggerInstructionPointer(instrPtr);
			it = m_EvList.makeActive(it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		BPRangeListener* pEvRange = dynamic_cast<BPRangeListener*>(*it);
		if (pEvRange && pEvRange->isMatching(instrPtr)) {
			pEvBreakpt->setTriggerInstructionPointer(instrPtr);
			it = m_EvList.makeActive(it);
			continue; // dito.
		}
		it++;
	}
	m_EvList.fireActiveListeners();
}

bool OVPController::save(const string& path)
{
	// TODO!
	ovpplatform.save(path);
	return false; // TODO
}

void OVPController::restore(const string& path)
{
	//TODO!
	assert(path.length() > 0 &&
		"FATAL ERROR: Tried to restore state without valid path!");
	ovpplatform.restore(path);
}

void OVPController::reboot()
{
	// TODO!
}

} // end-of-namespace: fail
