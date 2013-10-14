#ifndef __GEM5_CONTROLLER_HPP__
#define __GEM5_CONTROLLER_HPP__

#include <string>

#include "config/FailConfig.hpp"
#include "../SimulatorController.hpp"
#include "Gem5Memory.hpp"

// gem5 forward declarations:
class System;

namespace fail {

/**
 * \class Gem5Controller
 *
 * Gem5-specific implementation of a SimulatorController.
 *
 * \todo setRegisterContent() does not work with the program counter (RI_IP).
 */
class Gem5Controller : public SimulatorController {
private:
	System* m_System; //!< the gem5 system object
	ExperimentFlow* m_CurrFlow; //!< Stores the current flow for save/restore-operations
#if defined(CONFIG_EVENT_BREAKPOINTS) ||\
    defined(CONFIG_EVENT_BREAKPOINTS_RANGE)
	std::string m_Mnemonic; //!< mnemonic of the instr. (only with BPs)
#endif
	bool restore_request;
	std::string restore_path;
public:
	void startup();
	~Gem5Controller();

	bool save(const std::string &path);
	void restore(const std::string &path);
	void onRestore();
	bool isRestoreRequest();
	void reboot();
	virtual simtime_t getTimerTicks();
	virtual simtime_t getTimerTicksPerSecond();
#if defined(CONFIG_EVENT_BREAKPOINTS) ||\
    defined(CONFIG_EVENT_BREAKPOINTS_RANGE)
	void setMnemonic(const std::string& mn) { m_Mnemonic = mn; }
	const std::string& getMnemonic() const { return m_Mnemonic; }
#endif
};

} // end-of-namespace: fail

#endif // __GEM5_CONTROLLER_HPP__
