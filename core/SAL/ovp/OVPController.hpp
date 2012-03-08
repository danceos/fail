#ifndef __OVP_CONTROLLER_HPP__
  #define __OVP_CONTROLLER_HPP__

#include <string>
#include <cassert>
#include <string.h>
#include <string>

#include "../SimulatorController.hpp"
#include "../../controller/Event.hpp"

#include "../../../ovp/OVPPlatform.hpp"

#include "../Register.hpp"

using namespace std;

extern OVPPlatform ovpplatform;

/// Simulator Abstraction Layer namespace
namespace sal
{

/**
 * \class OVPController
 * OVP-specific implementation of a SimulatorController.
 */
class OVPController : public SimulatorController
{
	private:
		//FIXME: This should be obsolete now...
		//UniformRegisterSet *set; //(RT_GP);
		//UniformRegisterSet *setStatus;//(RT_ST);
		//UniformRegisterSet *setPC;//(RT_PC);

		// (FIXME: perhaps this should be declared as a static member)
		unsigned int m_currentRegId;
		// NOTE: Constants (such as GPRegisterId in SAL/bochs/BochsRegister.hpp)
		//       are much easier to read...
	public:
		// Initialize the controller.
		OVPController();
		virtual ~OVPController();
		virtual void onInstrPtrChanged(address_t instrPtr); 
		/**
		 * Save simulator state.
		 * @param path Location to store state information
		 */
		virtual void save(const string& path);
		/**
		 * Restore simulator state.
		 * @param path Location to previously saved state information
		 */
		virtual void restore(const string& path);
		/**
		 * Reboot simulator.
		 */
		virtual void reboot();
		/**
		 * Handles the control back to the previous coroutine which
		 * triggered the reboot. Need not to be called explicitly.
		 */
		void toPreviousCtx();
		/**
		 * Returns the current instruction pointer.
		 * @return the current eip
		 */

		/**
		 * Terminate simulator
		 */
		 virtual void terminate(int exCode = EXIT_SUCCESS);

		 virtual void fireTimer(uint32_t);		 

		 void makeGPRegister(int, void*, const string&);
		 void makeSTRegister(Register *, const string&);
		 void makePCRegister(int, void*, const string&);
		 
		 //DELETE-ME:This should be obsolete now...
		 /**
		  * Due to empty RegisterSets are not allowed, OVP platform
		  * must tell OVPController when it is finished
		  */
		 //void finishedRegisterCreation();

};
};

#endif
