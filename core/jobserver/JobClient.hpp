/**
 * \brief The Minion's JobClient requests ExperimentData and returns results.
 *
 * \author Martin Hoffmann
 */


#ifndef __JOB_CLIENT_H__
#define __JOB_CLIENT_H__

#include <string>
#include <ctime>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "SocketComm.hpp"
#include "controller/ExperimentData.hpp"
#include "jobserver/messagedefs/FailControlMessage.pb.h"

namespace fi {

  /**
   * \class JobClient
   * 
   * \brief Manages communication with JobServer
   * 
   */
  class JobClient {

    std::string m_server;
    int m_server_port;
    struct hostent* m_server_ent;
    int m_sockfd;

    bool connectToServer();

    FailControlMessage_Command tryToGetExperimentData(ExperimentData&  exp);
  public:
    JobClient(std::string server = "localhost", int port = 1111);
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


  
}


#endif
