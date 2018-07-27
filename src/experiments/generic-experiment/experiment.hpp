#ifndef __GENERIC_EXPERIMENT_EXPERIMENT_HPP__
#define __GENERIC_EXPERIMENT_EXPERIMENT_HPP__

#include "sal/SALInst.hpp"
#include "efw/DatabaseExperiment.hpp"
#include "sal/Listener.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include "../plugins/serialoutput/SerialOutputLogger.hpp"
#include <string>
#include <stdlib.h>
#include <map>
#include <set>


class GenericExperiment : public fail::DatabaseExperiment {
	fail::ElfReader *m_elf;
	std::string elf_file;

	std::string m_state_dir;

	fail::guest_address_t serial_port;
	SerialOutputLogger sol;
	bool serial_enabled;
	std::string serial_goldenrun;

	bool enabled_mem_text;
	fail::MemAccessListener l_mem_text;

	bool enabled_mem_outerspace;
	fail::MemAccessListener l_mem_outerspace;

	bool enabled_trap;
	fail::TrapListener l_trap;

	bool enabled_timeout;
	unsigned m_Timeout;
	fail::TimerListener l_timeout;

	std::map<fail::BaseListener *, const fail::ElfSymbol *> listener_to_symbol;

	typedef std::set<fail::BaseListener *> ListenerSet;

	ListenerSet end_markers;
	ListenerSet OK_marker;
	ListenerSet FAIL_marker;
	ListenerSet DETECTED_marker;
	ListenerSet GROUP1_marker;
	ListenerSet GROUP2_marker;
	ListenerSet GROUP3_marker;
	ListenerSet GROUP4_marker;

	std::map<std::string, ListenerSet * >  end_marker_groups;

	void parseSymbols(const std::string &args, std::set<fail::BaseListener *> *into);

public:
	GenericExperiment() : DatabaseExperiment("GenericExperiment"),
						  m_state_dir("state"),
						  sol(0),
						  l_trap(fail::ANY_TRAP), l_timeout(0) {
		enabled_mem_text = false;
		enabled_mem_outerspace = false;
		enabled_trap = false;
		enabled_timeout = false;

		end_marker_groups["ok-marker"] = &OK_marker;
		end_marker_groups["fail-marker"] = &FAIL_marker;
		end_marker_groups["detected-marker"] = &DETECTED_marker;
		end_marker_groups["group1-marker"] = &GROUP1_marker;
		end_marker_groups["group2-marker"] = &GROUP2_marker;
		end_marker_groups["group3-marker"] = &GROUP3_marker;
		end_marker_groups["group4-marker"] = &GROUP4_marker;
	}
	virtual ~GenericExperiment();

	/**
	 * Get path to the state directory
	 */
	virtual std::string cb_state_directory() { return m_state_dir; }

	/**
	 * Allocate enough space to hold the incoming ExperimentData message.
	 */
	virtual fail::ExperimentData* cb_allocate_experiment_data();

	/**
	 * Allocate a new result slot in the given experiment data
	 */
	virtual google::protobuf::Message* cb_new_result(fail::ExperimentData* data);

	/**
	 * Callback that is called, before the actual experiment
	 * starts. Simulation is terminated on false.
	 * @param The current result message
	 * @return \c true on success, \c false otherwise
	 */
	virtual bool cb_start_experiment();

	/**
	 * Callback that is called before the fast forward is done. This
	 * can be used to add additional event listeners during the fast
	 * forward phase. If returning false, the experiment is canceled.
	 * @return \c true on success, \c false otherwise
	 */
	virtual bool cb_before_fast_forward();

	/**
	 * Callback that is called before the resuming till crash has
	 * started. This is called after the fault was injected. Here the
	 * end listeners should be installed. Returns true on
	 * success. Otherwise the experiment is canceled.

	 * @return \c true on success, \c false otherwise
	 */
	virtual bool cb_before_resume();

	/**
	 * Callback that is called after the resume-till-crash phase with
	 * the last triggered listener. This callback should collect all
	 * data and fill up the result message.
	 */
	virtual void cb_after_resume(fail::BaseListener *event);
};

#endif // __GENERIC_EXPERIMENT_EXPERIMENT_HPP__
