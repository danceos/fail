#ifndef __JOB_CLIENT_H__
  #define __JOB_CLIENT_H__

#include <string>
#include <ctime>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "comm/SocketComm.hpp"
#include "comm/ExperimentData.hpp"
#include "comm/FailControlMessage.pb.h"
#include "config/FailConfig.hpp"

namespace fail {

/**
* \class JobClient
* 
* \brief Manages communication with JobServer
* The Minion's JobClient requests ExperimentData and returns results.
*/
class JobClient {
private:
	std::string m_server;
	int m_server_port;
	struct hostent* m_server_ent;
	int m_sockfd;

	bool connectToServer();

	FailControlMessage_Command tryToGetExperimentData(ExperimentData& exp);
public:
	JobClient(const std::string& server = SERVER_COMM_HOSTNAME, int port = SERVER_COMM_TCP_PORT);
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
};

} // end-of-namespace: fail

#endif // __JOB_CLIENT_H__
