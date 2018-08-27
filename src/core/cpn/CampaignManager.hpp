/**
 * \file CampaignManager.hpp
 * \brief The manager for an entire campaign
 */

#ifndef __CAMPAIGN_MANAGER_HPP__
#define __CAMPAIGN_MANAGER_HPP__

#include "sal/SALInst.hpp"
#include "comm/ExperimentData.hpp"
#include "Campaign.hpp"
#include "util/CommandLine.hpp"

namespace fail {

/**
 * \class CampaignManager
 *
 * The CampaignManager allows a user-campaign access to all constant
 * simulator information and forwards single experiments to the JobServer.
 */
class JobServer;
class CampaignManager {
private:
	JobServer *m_jobserver;
	Campaign* m_currentCampaign;
	CommandLine::option_handle port;
public:
	CampaignManager() : m_jobserver(0), m_currentCampaign(0)
	{
		fail::CommandLine &cmd = fail::CommandLine::Inst();
		port = cmd.addOption("", "port", Arg::Required,
				     "--port \tListening port of server; no "
				     "value chooses port automatically");
	}
	~CampaignManager();
	/**
	 * Executes a user campaign
	 */
	bool runCampaign(Campaign* c);
	/**
	 * Returns a const reference for acquiring constant simulator specific information.
	 * e.g., Registernames, to ease experiment data construction.
	 * The campaign description is not allowed to change the simulator
	 * state, as the actual simulation runs within another process (Minion)
	 * @return constant reference to the current simulator backend.
	 */
	SimulatorController const& getSimulator() const { return simulator; }
	/**
	 * Add a experiment parameter set.
	 * The user campaign has to allocate the Parameter object,
	 * and deallocate it after result reception.
	 * A Parameter set includes space for results.
	 * @param exp A pointer to a ExperimentData set.
	 */
	void addParam(ExperimentData* exp);
	/**
	 * A user campaign can request a single result (blocking) from the queue.
	 * @return Pointer to a parameter object with filled result data
	 * @see addParam()
	 */
	ExperimentData* getDone();
	/**
	 * Signal, that there will not come any further parameter sets.
	 */
	void noMoreParameters();
	/**
	 * Can optionally be used to tell the JobServer how many jobs to expect in
	 * total.  This count is used for progress reporting.  Make sure you also
	 * call skipJobs() if some of these early-on announced jobs will not be
	 * sent after all (e.g. because the campaign already found results for them
	 * in the database).
	 */
	void setTotalCount(uint64_t count);
	void skipJobs(uint64_t count);
	 /**
	 * User campaign has finished.
	 */
	void done();
	/**
	 * Wait actively, until all experiments expired.
	 */
//	void waitForCompletion();
};

extern CampaignManager campaignmanager;

} // end-of-namespace: fail

#endif // __CAMPAIGN_MANAGER_HPP__
