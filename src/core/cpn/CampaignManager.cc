#include <cstdlib>

#include "CampaignManager.hpp"
#include "util/Logger.hpp"
#include "JobServer.hpp"

namespace fail {

static Logger log_send("CampaignManager");

CampaignManager campaignmanager;

CampaignManager::~CampaignManager() { delete m_jobserver; }

bool CampaignManager::runCampaign(Campaign *c) {
	fail::CommandLine &cmd = fail::CommandLine::Inst();
	if (!cmd.parse()) {
		log_send << "Error parsing arguments." << std::endl;
		exit(-1);
	}

	if (!m_jobserver) {
		m_jobserver = (cmd[port].count() > 0)
				  ? new JobServer(std::atoi(cmd[port].first()->arg))
				  : new JobServer;
	}
	m_currentCampaign = c;
	bool ret = c->run();
	m_jobserver->done();
	return ret;
}

void CampaignManager::addParam(ExperimentData *exp)
{
	m_jobserver->addParam(exp);
}

ExperimentData *CampaignManager::getDone() { return m_jobserver->getDone(); }

void CampaignManager::noMoreParameters()
{
	m_jobserver->setNoMoreExperiments();
}

void CampaignManager::setTotalCount(uint64_t count)
{
	m_jobserver->setTotalCount(count);
}

void CampaignManager::skipJobs(uint64_t count)
{
	m_jobserver->skipJobs(count);
}

void CampaignManager::done() { m_jobserver->done(); }
} // end-of-namespace: fail
