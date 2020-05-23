#ifndef __DATABASE_EXPERIMENT_HPP__
#define __DATABASE_EXPERIMENT_HPP__

#include <google/protobuf/message.h>
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include <string>
#include <stdlib.h>

namespace fail {
class ExperimentData;

class DatabaseExperiment : public fail::ExperimentFlow {
	fail::JobClient *m_jc;

	unsigned injectFault(address_t data_address, unsigned bitpos, bool inject_burst,
		bool inject_registers, bool force_registers, bool randomjump);

	/**
	   The current experiment data as returned by the job client. This
	   allocated by cb_allocate_experiment_data()
	*/
	ExperimentData *m_current_param;
	google::protobuf::Message *m_current_result;

public:
	DatabaseExperiment(const std::string &name)
		: m_log(name, false), m_mm(fail::simulator.getMemoryManager()) {

		/* The fail server can be set with an environent variable,
		   otherwise the JOBSERVER configured by cmake ist used */
		char *server_host = getenv("FAIL_SERVER_HOST");
		if (server_host != NULL){
			this->m_jc = new fail::JobClient(std::string(server_host));
		} else {
			this->m_jc = new fail::JobClient();
		}
	}

	virtual ~DatabaseExperiment();

	bool run();


protected:
	fail::Logger m_log;
	fail::MemoryManager& m_mm;

	/** Returns the currently running experiment message as returned
	 * by the job client
	 */
	ExperimentData * get_current_experiment_data() { return m_current_param; }

	/** Returns the currently result message, that was allocated by
	 * cb_allocate_new_result.
	 */
	google::protobuf::Message * get_current_result() { return m_current_result; }


	//////////////////////////////////////////////////////////////////
	// Can be overwritten by experiment
	//////////////////////////////////////////////////////////////////

	/**
	 * Get path to the state directory
	 */
	virtual std::string cb_state_directory() { return "state"; }

	/**
	 * Callback that is called, before the actual experiment
	 * starts. Simulation is terminated on false.
	 * @param The current result message
	 * @return \c true on success, \c false otherwise
	 */
	virtual bool cb_start_experiment() { return true; };

	/**
	 * Allocate enough space to hold the incoming ExperimentData
	 * message. The can be accessed during the experiment through
	 * get_current_experiment_data()
	 */
	virtual ExperimentData* cb_allocate_experiment_data() = 0;
	virtual void cb_free_experiment_data(ExperimentData *) {};


	/**
	 * Allocate a new result slot in the given experiment data. The
	 * returned pointer can be obtained by calling
	 * get_current_result()
	 */
	virtual google::protobuf::Message* cb_new_result(ExperimentData*) = 0;

	/**
	 * Callback that is called before the fast forward is done. This
	 * can be used to add additional event listeners during the fast
	 * forward phase. If returning false, the experiment is canceled.
	 * @return \c true on success, \c false otherwise
	 */
	virtual bool cb_before_fast_forward() { return true; };

	/**
	 * Callback that is called during the fast forward, when an event
	 * has triggered, but it was not the fast forward listener. This
	 * can be used to collect additional information during the fast
	 * forward If returning false, the fast forwarding is stopped.
	 *
	 * @return \c true on should continue, \c false stop ff
	 */
	virtual bool cb_during_fast_forward(fail::BaseListener *) { return false; };

	/**
	 * Callback that is called after the fast forward, with the last
	 * triggered event forward If returning false, the experiment is
	 * canceled.
	 *
	 * @return \c true on success, \c false otherwise
	 */
	virtual bool cb_after_fast_forward(fail::BaseListener *) { return true; };

	/**
	 * Callback that is called before the resuming till crash has
	 * started. This is called after the fault was injected. Here the
	 * end listeners should be installed. Returns true on
	 * success. Otherwise the experiment is canceled.

	 * @return \c true on success, \c false otherwise
	 */
	virtual bool cb_before_resume() = 0;

	/**
	 * Callback that is called during the resume-till-crash phase,
	 * when an event has triggered, This can be used to collect
	 * additional information during the resuming phse. If returning
	 * false, the resuming has finished and the experiment has stopped.
	 *
	 * @return \c true on should continue ff, \c false stop ff
	 */
	virtual bool cb_during_resume(fail::BaseListener *) { return false; };

	/**
	 * Callback that is called after the resume-till-crash phase with
	 * the last triggered listener. This callback should collect all data and
	 *
	 */
	virtual void cb_after_resume(fail::BaseListener *) = 0;

private:
	void redecodeCurrentInstruction();
};

}

#endif // __DATABASE_EXPERIMENT_HPP__
