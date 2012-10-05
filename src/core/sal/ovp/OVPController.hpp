#ifndef __OVP_CONTROLLER_HPP__
  #define __OVP_CONTROLLER_HPP__

#include <string>
#include <cassert>
#include <string.h>
#include <string>

#include "../SimulatorController.hpp"
#include "../Listener.hpp"
#include "../Register.hpp"
#include "ovp/OVPPlatform.hpp"

namespace fail {

extern OVPPlatform ovpplatform;

/**
 * \class OVPController
 * OVP-specific implementation of a SimulatorController.
 */
class OVPController : public SimulatorController {
private:
	// FIXME: This should be obsolete now...
//	UniformRegisterSet *set; //(RT_GP);
//	UniformRegisterSet *setStatus;//(RT_ST);
//	UniformRegisterSet *setPC;//(RT_PC);

	// FIXME: Perhaps this should be declared as a static member:
	unsigned int m_currentRegId;
	// NOTE: Constants (such as GPRegisterId in sal/bochs/BochsRegister.hpp)
	//       are much easier to read...
public:
	/**
	 * Initialize the controller.
	 */
	OVPController();
	~OVPController();
	void onInstrPtrChanged(address_t instrPtr); 
	/**
	 * Save simulator state.
	 * @param path Location to store state information
	 * @return \c true if the state has been successfully saved, \c false otherwise
	 */
	bool save(const std::string& path);
	/**
	 * Restore simulator state.
	 * @param path Location to previously saved state information
	 */
	void restore(const std::string& path);
	/**
	 * Reboot simulator.
	 */
	void reboot();
	/**
	 * Returns the current instruction pointer.
	 * @return the current eip
	 */
	void makeGPRegister(int, void*, const std::string&);
	void makeSTRegister(Register *, const std::string&);
	void makePCRegister(int, void*, const std::string&);
	/**
	 * Due to empty RegisterSets are not allowed, OVP platform
	 * must tell OVPController when it is finished
	 * 
	 * FIXME: This should be obsolete now...
	 */
//	void finishedRegisterCreation();
};

} // end-of-namespace: fail

#endif // __OVP_CONTROLLER_HPP__
