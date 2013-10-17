#ifndef __EZS_LOGGER_EXPERIMENT_HPP__
#define __EZS_LOGGER_EXPERIMENT_HPP__


#include "sal/SALInst.hpp"
#include "efw/ExperimentFlow.hpp"
#include "util/Logger.hpp"
#include <vector>
#include <string>
#include "util/ElfReader.hpp"

/**
 * @file
 * @brief Base system for EZS realtime lecture
 */

/**
 * @class
 * @brief Experiment implementation
 */
class EZSLogger : public fail::ExperimentFlow {
public:

private:
	fail::Logger m_log; //! debug output

    /**
     * @brief Tiniy little helper to setup a RealtimeLogger instance
     */
    void setupLog(const fail::ElfSymbol & target, const std::string& prefix);

public:
	EZSLogger() : m_log("EZSLogger", false) {}

	bool run();
};

#endif
