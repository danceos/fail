#ifndef __JOB_CLIENT_H__
#define __JOB_CLIENT_H__

#include <string>
#include <ctime>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <deque>
#include <memory>

// Xlib.h #defines this, which breaks protobuf headers.
#undef Status

#include "comm/ExperimentData.hpp"
#include "comm/FailControlMessage.pb.h"
#include "config/FailConfig.hpp"
#include "util/WallclockTimer.hpp"

namespace fail {

/**
* \class JobClient
*
* \brief Manages communication with JobServer
* The Minion's JobClient requests ExperimentData and returns results.
*/
class JobClient {
private:
	struct impl;
	impl *m_d; // meh. With a managed pointer everything needs to be >= C++11.

	std::string m_server;
	int m_server_port;

	uint64_t m_server_runid;

	WallclockTimer m_job_runtime;
	double m_job_runtime_total;
	int m_job_throughput;
	int m_job_total;
	std::deque<ExperimentData*> m_parameters;
	std::deque<ExperimentData*> m_results;

	bool m_connect_failed;

	bool connectToServer();
	bool sendResultsToServer();
	FailControlMessage_Command tryToGetExperimentData(ExperimentData& exp);

public:
	JobClient(const std::string& server = SERVER_COMM_HOSTNAME, int port = SERVER_COMM_TCP_PORT);
	~JobClient();
	/**
	* Receive experiment data set from (remote) JobServer
	* The caller (experiment developer) is responsible for
	* allocating his ExperimentData object.
	*
	* @param exp Reference to a ExperimentData object allocated by the caller!
	* @return \c true if parameter have been received and put into \c exp, \c false else.
	*/
	bool getParam(ExperimentData& exp);
	/**
	* Send back experiment result to the (remote) JobServer
	* The caller (experiment developer) is responsible for
	* destroying his ExperimentData object afterwards.
	*
	* @param result Reference to the ExperimentData holding result values
	* @return \c true Result successfully sent, \c false else.
	*/
	bool sendResult(ExperimentData& result);
	/**
	 * Return the number of undone jobs that have already been fetched from the server.
	 *
	 * @return the number of undone jobs.
	 */
	int getNumberOfUndoneJobs() { return m_parameters.size(); }
};

} // end-of-namespace: fail

#endif // __JOB_CLIENT_H__
