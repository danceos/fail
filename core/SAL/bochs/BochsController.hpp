#ifndef __BOCHS_CONTROLLER_HPP__
  #define __BOCHS_CONTROLLER_HPP__

#define DEBUG

#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>

#include "failbochs.hpp"

#include "../SimulatorController.hpp"
#include "../../controller/Event.hpp"

#include "../../../bochs/bochs.h"
#include "../../../bochs/cpu/cpu.h"
#include "../../../bochs/config.h"

using namespace std;

namespace fi { class ExperimentFlow; }

/// Simulator Abstraction Layer namespace
namespace sal
{
	
/**
 * \class BochsController
 * Bochs-specific implementation of a SimulatorController.
 */
class BochsController : public SimulatorController
{
	private:
		/**
		 * stores the current flow for save/restore-operations
		 */
		fi::ExperimentFlow* m_CurrFlow;
	  #ifdef DEBUG
		unsigned m_Regularity;
		unsigned m_Counter;
		std::ostream* m_pDest;
	  #endif
	public:
		// Initialize the controller.
		BochsController();
		~BochsController();
		/* ********************************************************************
		 * Standard Event Handler API (SEH-API):
		 * ********************************************************************/
		/**
		 * Instruction pointer modification handler. This method is called (from
		 * the CPULoop aspect) every time when the Bochs-internal IP changes.
		 * @param instrPtr
		 */
		void onInstrPtrChanged(address_t instrPtr);
		/* ********************************************************************
		 * Simulator Controller & Access API (SCA-API):
		 * ********************************************************************/
		/**
		 * Save simulator state.
		 * @param path Location to store state information
		 */
		void save(const string& path);
		/**
		 * Save finished: Callback from Simulator
		 */
		void saveDone();
		/**
		 * Restore simulator state. Clears all Events.
		 * @param path Location to previously saved state information
		 */
		void restore(const string& path);
		/**
		 * Restore finished: Callback from Simulator
		 */
		void restoreDone();
		/**
		 * Reboot simulator. Clears all Events.
		 */
		void reboot();
		/**
		 * Reboot finished: Callback from Simulator
		 */
		void rebootDone();
		/**
		 * Terminate simulator
		 * @param exCode Individual exit code
		 */
		void terminate(int exCode = EXIT_SUCCESS);
	  #ifdef DEBUG
		/**
		 * Enables instruction pointer debugging output.
		 * @param regularity the output regularity; 1 to display every
		 *        instruction pointer, 0 to disable
		 * @param dest specifies the output destition; defaults to \c std::cout
		 */
		void dbgEnableInstrPtrOutput(unsigned regularity, std::ostream* dest = &cout);
	  #endif
};

} // end-of-namespace: sal

#endif /* __BOCHS_CONTROLLER_HPP__ */
