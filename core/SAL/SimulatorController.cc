
// Author: Adrian Böckenkamp
// Date:   23.01.2012

#include "SimulatorController.hpp"
#include "SALInst.hpp"
#include "../controller/Event.hpp"

namespace sal
{

// External reference declared in SALInst.hpp
ConcreteSimulatorController simulator;

fi::EventId SimulatorController::addEvent(fi::BaseEvent* ev)
{
	return (m_EvList.add(ev, m_Flows.getCurrent()));
}

fi::BaseEvent* SimulatorController::waitAny(void)
{
	m_Flows.resume();
	assert(m_EvList.getLastFired() != NULL &&
		   "FATAL ERROR: getLastFired() expected to be non-NULL!");
	return (m_EvList.getLastFired());
}

void SimulatorController::startup()
{
	// Some greetings to the user:
	std::cout << "[SimulatorController] Initializing..." << std::endl;
	
	// TODO: Retrieve ExperimentData from the job-server (*before* each
	//       experiment-routine gets started)...!
	
	// Activate previously added experiments to allow initialization:
	initExperiments();
}

void SimulatorController::initExperiments()
{
	/* empty. */
}

void SimulatorController::onBreakpointEvent(address_t instrPtr)
{
	assert(false &&
	"FIXME: SimulatorController::onBreakpointEvent() has not been tested before");
	
	// FIXME: Performanz verbessern

	// Loop through all events of type BP*Event:
	fi::EventList::iterator it = m_EvList.begin();
	while(it != m_EvList.end())
	{
		fi::BaseEvent* pev = *it;
		fi::BPEvent* pbp; fi::BPRangeEvent* pbpr;
		if((pbp = dynamic_cast<fi::BPEvent*>(pev)) && pbp->isMatching(instrPtr))
		{
			pbp->setTriggerInstructionPointer(instrPtr);
			it = m_EvList.makeActive(it);
			// "it" has already been set to the next element (by calling
			// makeActive()):
			continue; // -> skip iterator increment
		}
		else if((pbpr = dynamic_cast<fi::BPRangeEvent*>(pev)) &&
		        pbpr->isMatching(instrPtr))
		{
			pbpr->setTriggerInstructionPointer(instrPtr);
			it = m_EvList.makeActive(it);
			continue; // dito
		}
		++it;
	}
	m_EvList.fireActiveEvents();
}

void SimulatorController::onMemoryAccessEvent(address_t addr, size_t len,
                                              bool is_write, address_t instrPtr)
{
	// FIXME: Performanz verbessern (falls Iteratorlogik bleibt, wäre
	//        ein "hasMoreOf(typeid(MemEvents))" denkbar...

	fi::MemAccessEvent::accessType_t accesstype =
		is_write ? fi::MemAccessEvent::MEM_WRITE
		         : fi::MemAccessEvent::MEM_READ;

	fi::EventList::iterator it = m_EvList.begin();
	while(it != m_EvList.end()) // check for active events
	{
		fi::BaseEvent* pev = *it;
		fi::MemAccessEvent* ev = dynamic_cast<fi::MemAccessEvent*>(pev);
		// Is this a MemAccessEvent? Correct access type?
		if(!ev || !ev->isMatching(addr, accesstype))
		{
			++it;
			continue; // skip event activation
		}
		ev->setTriggerAddress(addr);
		ev->setTriggerWidth(len);
		ev->setTriggerInstructionPointer(instrPtr);
		ev->setTriggerAccessType(accesstype);
		it = m_EvList.makeActive(it);
	}
	m_EvList.fireActiveEvents();
}

void SimulatorController::onInterruptEvent(unsigned interruptNum, bool nmi)
{
	fi::EventList::iterator it = m_EvList.begin();
	while(it != m_EvList.end()) // check for active events
	{
		fi::BaseEvent* pev = *it;
		fi::InterruptEvent* pie = dynamic_cast<fi::InterruptEvent*>(pev);
		if(!pie || !pie->isMatching(interruptNum))
		{
			++it;
			continue; // skip event activation
		}
		pie->setTriggerNumber(interruptNum);
		pie->setNMI(nmi);
		it = m_EvList.makeActive(it);
	}
	m_EvList.fireActiveEvents();
}

bool SimulatorController::isSuppressedInterrupt(unsigned interruptNum)
{
	for(size_t i = 0; i < m_SuppressedInterrupts.size(); i++)
		if((m_SuppressedInterrupts[i] == interruptNum ||
		   m_SuppressedInterrupts[i] == fi::ANY_INTERRUPT) && interruptNum != (unsigned) interrupt_to_fire+32  ){
				if((int)interruptNum == interrupt_to_fire+32){
					interrupt_to_fire = -1;
					return(true);
				}
			return (true);
		}
	return (false);
}

bool SimulatorController::addSuppressedInterrupt(unsigned interruptNum)
{
	// Check if already existing:
	if(isSuppressedInterrupt(interruptNum+32))
		return (false); // already added: nothing to do here
		
	if(interruptNum == fi::ANY_INTERRUPT){
		m_SuppressedInterrupts.push_back(interruptNum);
		return (true);
	}else{
		m_SuppressedInterrupts.push_back(interruptNum+32);
		return (true);
	}
}

bool SimulatorController::removeSuppressedInterrupt(unsigned interruptNum)
{
	for(size_t i = 0; i < m_SuppressedInterrupts.size(); i++)
	{	
		if(m_SuppressedInterrupts[i] == interruptNum+32 || m_SuppressedInterrupts[i] == fi::ANY_INTERRUPT)
		{
			m_SuppressedInterrupts.erase(m_SuppressedInterrupts.begin() + i);
			return (true);
		}
	}
	return (false);
}

void SimulatorController::onTrapEvent(unsigned trapNum)
{
	fi::EventList::iterator it = m_EvList.begin();
	while(it != m_EvList.end()) // check for active events
	{
		fi::BaseEvent* pev = *it;
		fi::TrapEvent* pte = dynamic_cast<fi::TrapEvent*>(pev);
		if(!pte || !pte->isMatching(trapNum))
		{
			++it;
			continue; // skip event activation
		}
		pte->setTriggerNumber(trapNum);
		it = m_EvList.makeActive(it);
	}
	m_EvList.fireActiveEvents();
}

void SimulatorController::onGuestSystemEvent(char data, unsigned port)
{
	fi::EventList::iterator it = m_EvList.begin();
	while(it != m_EvList.end()) // check for active events
	{
		fi::BaseEvent* pev = *it;
		fi::GuestEvent* pge = dynamic_cast<fi::GuestEvent*>(pev);
		if(pge != NULL)
		{
			pge->setData(data);
			pge->setPort(port);
			it = m_EvList.makeActive(it);
			continue; // dito.
		}
		++it;
	}
	m_EvList.fireActiveEvents();
}

void SimulatorController::onJumpEvent(bool flagTriggered, unsigned opcode)
{
	fi::EventList::iterator it = m_EvList.begin();
	while(it != m_EvList.end()) // check for active events
	{
		fi::JumpEvent* pje = dynamic_cast<fi::JumpEvent*>(*it);
		if(pje != NULL)
		{
			pje->setOpcode(opcode);
			pje->setFlagTriggered(flagTriggered);
			it = m_EvList.makeActive(it);
			continue; // dito.
		}
		++it;
	}
	m_EvList.fireActiveEvents();
}

void SimulatorController::addFlow(fi::ExperimentFlow* flow)
{
	// Store the (flow,corohandle)-tuple internally and create its coroutine:
	m_Flows.create(flow);
	// let it run for the first time
	m_Flows.toggle(flow);
}

void SimulatorController::removeFlow(fi::ExperimentFlow* flow)
{
	// remove all remaining events of this flow
	clearEvents(flow);
	// remove coroutine
	m_Flows.remove(flow);
}

fi::BaseEvent* SimulatorController::addEventAndWait(fi::BaseEvent* ev)
{
	addEvent(ev);
	return (waitAny());
}

template <class T>
T* SimulatorController::getExperimentData()
{
	//BEGIN ONLY FOR TESTING------REMOVE--------REMOVE---------REMOVE--------REMOVE-------REMOVE-------
	//Daten in Struktur schreiben und in Datei speichern
	ofstream fileWrite;
	fileWrite.open("test.txt");

	T* faultCovExWrite =  new T();;
	faultCovExWrite->set_data_name("Testfall 42");

	//Namen setzen
	faultCovExWrite->set_data_name("Testfall 42");
	//Instruktionpointer 1
	faultCovExWrite->set_m_instrptr1(0x4711);
	//Instruktionpointer 2
	faultCovExWrite->set_m_instrptr2(0x1122);

	//In ExperimentData verpacken
	fi::ExperimentData exDaWrite(faultCovExWrite);
	//Serialisierung ueber Wrapper-Methode in ExperimentData
	exDaWrite.serialize(&fileWrite);

	//cout << "Ausgabe: " << out << endl;

	fileWrite.close(); 
	
	ifstream fileRead;
	fileRead.open("test.txt");
	//END ONLY FOR TESTING------REMOVE--------REMOVE---------REMOVE--------REMOVE-------REMOVE-------
	//TODO: implement server-client communication----------------------------------------------
	
	T* concreteExpDat = new T();
	fi::ExperimentData exDaRead(concreteExpDat);
	exDaRead.unserialize(&fileRead);

	return (concreteExpDat);
}

void SimulatorController::terminate(int exCode)
{
	// Attention: This could cause problems, e.g., because of non-closed sockets
	std::cout << "[FAIL] Exit called by experiment with exit code: " << exCode << std::endl;
	// TODO: (Non-)Verbose-Mode?
	exit(exCode);
}

} // end-of-namespace: sal
