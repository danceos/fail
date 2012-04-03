#ifndef __SIMULATOR_CONTROLLER_HPP__
  #define __SIMULATOR_CONTROLLER_HPP__
  
// Author: Adrian Böckenkamp
// Date:   23.01.2012

#include <iostream>
#include <string>
#include <cassert>
#include <vector>

//<BEGIN ONLY FOR TEST
#include <fstream>
//>END ONLY FOR TEST

#include "../controller/Event.hpp"
#include "../controller/EventList.hpp"
#include "../controller/CoroutineManager.hpp"
#include "../controller/ExperimentData.hpp"
#include "SALConfig.hpp"

using namespace std;

namespace fi {
class ExperimentFlow;
}

/// Simulator Abstraction Layer namespace
namespace sal
{

// incomplete types suffice here
class RegisterManager;
class MemoryManager;

/**
 * \class SimulatorController
 *
 * \brief The abstract interface for controlling simulators and
 *        accessing experiment data/flows.
 *
 * This class manages (1..N) experiments and provides access to the underlying
 * simulator/debugger system. Experiments can enlist arbritrary events
 * (Breakpoint, Memory access, Traps, etc.). The SimulatorController then
 * activates the specific experiment There are further methods to read/write
 * registers and memory, and control the SUT (save/restore/reset).
 */
class SimulatorController
{
	protected:
		fi::EventList m_EvList; //!< storage where events are being buffered
		fi::CoroutineManager m_Flows; //!< managed experiment flows
		RegisterManager *m_Regs; //!< access to cpu register
		MemoryManager *m_Mem; //!< access to memory pool
		//! list of suppressed interrupts
		std::vector<unsigned> m_SuppressedInterrupts;
		friend class fi::EventList; //!< "outsources" the event management
	public:
		SimulatorController()
			: m_Regs(NULL), m_Mem(NULL) { }
		SimulatorController(RegisterManager* regs, MemoryManager* mem)
			: m_Regs(regs), m_Mem(mem) { }
		virtual ~SimulatorController() { }
		/**
		 * @brief Initialization function each implementation needs to call on
		 *        startup
		 *
		 * This function needs to be invoked once the simulator starts, and
		 * allows the SimulatorController to instantiate all needed experiment
		 * components.
		 */
		void startup();
		/**
		 * Experiments need to hook here.
		 */
		void initExperiments();
		/* ********************************************************************
		 * Standard Event Handler API
		 * ********************************************************************/
		/**
		 * Breakpoint event handler. This routine needs to be called in the
		 * simulator specific backend each time a breakpoint event occurs.
		 * @param instrPtr the instruction pointer of the breakpoint event
		 */
		void onBreakpointEvent(address_t instrPtr);
		/**
		 * Memory access event handler (read/write).
		 * @param addr the accessed memory address
		 * @param len the length of the accessed memory
		 * @param is_write \c true if memory is written, \c false if read
		 * @param instrPtr the address of the instruction causing the memory
		 *        access
		 * 
		 * FIXME: should instrPtr be part of this interface?
		 */
		void onMemoryAccessEvent(address_t addr, size_t len,
								 bool is_write, address_t instrPtr);
		/**
		 * Interrupt event handler.
		 * @param interruptNum the interrupt-type id
		 * @param nmi nmi-value from guest-system
		 */
		void onInterruptEvent(unsigned interruptNum, bool nmi);
		/**
		 * Trap event handler.
		 * @param trapNum the trap-type id
		 */
		void onTrapEvent(unsigned trapNum);
		/**
		 * Guest system communication handler.
		 * @param data the "message" from the guest system
		 * @param port the port of the event
		 */
		void onGuestSystemEvent(char data, unsigned port);
		/**
		 * (Conditional) Jump-instruction handler.
		 * @param flagTriggered \c true if the jump was triggered due to a
		 *        specific FLAG (zero/carry/sign/overflow/parity flag)
		 * @param opcode the opcode of the conrecete jump instruction
		 */
		void onJumpEvent(bool flagTriggered, unsigned opcode);
		/* ********************************************************************
		 * Simulator Controller & Access API (SCA-API):
		 * ********************************************************************/
		/**
		 * Save simulator state.
		 * @param path Location to store state information
		 */
		virtual void save(const string& path) = 0;
		/**
		 * Restore simulator state.
		 * @param path Location to previously saved state information
		 */
		virtual void restore(const string& path) = 0;
		/**
		 * Reboot simulator.
		 */
		virtual void reboot() = 0;
		/**
		 * Terminate simulator
		 * @param exCode Individual exit code
		 */
		void terminate(int exCode = EXIT_SUCCESS);
		/**
		 * Check whether the interrupt should be suppressed.
		 * @param interruptNum the interrupt-type id
		 * @return \c true if the interrupt is suppressed, \c false oterwise
		 */
		bool isSuppressedInterrupt(unsigned interruptNum);
		/**
		 * Add a Interrupt to the list of suppressed.
		 * @param interruptNum the interrupt-type id
		 * @return \c true if sucessfully added, \c false otherwise (already
		 *         existing)
		 */
		bool addSuppressedInterrupt(unsigned interruptNum);
		/**
		 * Remove a Interrupt from the list of suppressed.
		 * @param interruptNum the interrupt-type id
		 * @return \c true if sucessfully removed, \c false otherwise (not
		 *         found)
		 */
		bool removeSuppressedInterrupt(unsigned interruptNum);
		/**
		 * Returns the (constant) initialized register manager.
		 * @return a reference to the register manager
		 */
		RegisterManager& getRegisterManager() { return (*m_Regs); }
		const RegisterManager& getRegisterManager() const { return (*m_Regs); }
		/**
		 * Sets the register manager.
		 * @param pReg the new register manager (or a concrete derived class of
		 *        RegisterManager)
		 */
		void setRegisterManager(RegisterManager* pReg) { m_Regs = pReg; }
		/**
		 * Returns the (constant) initialized memory manager.
		 * @return a reference to the memory manager
		 */
		MemoryManager& getMemoryManager() { return (*m_Mem); }
		const MemoryManager& getMemoryManager() const { return (*m_Mem); }
		/**
		 * Sets the memory manager.
		 * @param pMem a new concrete memory manager
		 */
		void setMemoryManager(MemoryManager* pMem) { m_Mem = pMem; }
		/* ********************************************************************
		 * Experiment-Flow & Event Management API (EFEM-API):
		 * ********************************************************************/
		/**
		 * Adds the specified experiment or plugin and creates a coroutine to
		 * run it in.
		 * @param flow the experiment flow object to be added
		 */
		void addFlow(fi::ExperimentFlow* flow);
		/**
		 * Removes the specified experiment or plugin and destroys its coroutine
		 * and all associated events.
		 * @param flow the experiment flow object to be removed
		 */
		void removeFlow(fi::ExperimentFlow* flow);
		/**
		 * Add event ev to the event management. This causes the event to be
		 * active.
		 * @param ev the event pointer to be added for the current flow
		 * @return the id of the event used to identify the object on occurrence
		 */
		fi::EventId addEvent(fi::BaseEvent* ev);
		/**
		 * Removes the event with the specified id.
		 * @param ev the pointer of the event-object to be removed; if \a ev is
		 *        equal to \c NULL all events (for all experiments) will be
		 *        removed
		 */
		void removeEvent(fi::BaseEvent* ev) { m_EvList.remove(ev); }
		/**
		 * Removes all previously added events for all experiments. This is
		 * equal to removeEvent(NULL);
		 */
		void clearEvents() { removeEvent(NULL); }
		/**
		 * Waits on any events which have been added to the event management. If
		 * one of those events occour, waitAny() will return the id of that
		 * event.
		 * @return the previously occurred event
		 */
		fi::BaseEvent* waitAny();
		/**
		 * Add event \a ev to the global buffer and wait for it (combines
		 * \c addEvent() and \c waitAny()).
		 * @param ev the event pointer to be added
		 * @return the pointer of the occurred event (it is not guaranteed that
		 *         this pointer will be equal to ev)
		 */
		fi::BaseEvent* addEventAndWait(fi::BaseEvent* ev);
		/**
		 * Removes all residual events associated with the specified experiment
		 * flow \a pExp.
		 * @param pExp the experiment whose events should be removed
		 */
		void cleanup(fi::ExperimentFlow* pExp);
		/**
		 * Fetches data for the experiments from the Job-Server. 
		 * @return the Experiment-Data from the Job-Server.
		 */
		template <class T> T* getExperimentData();
};

} // end-of-namespace: sal

#endif /* __SIMULATOR_CONTROLLER_HPP__ */
