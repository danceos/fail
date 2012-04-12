/**
 * \brief Socket based communictaion
 *
 * \author Horst Schirmeier, Martin Hoffmann
 */

#ifndef __SOCKETCOMM_HPP__
#define __SOCKETCOMM_HPP__

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <google/protobuf/message.h>
#define USE_SIZE_PREFIX

namespace fi {

class SocketComm {
public:	
  
	/**
	 * Send Protobuf-generated message
	 * @param sockfd open socket descriptor to write to
	 * @param Msg Reference to Protobuf generated message type
	 * \return false if message sending failed
	 */
	static bool send_msg(int sockfd, google::protobuf::Message& msg);
  
	/**
	 * Receive Protobuf-generated message
	 * @param sockfd open socket descriptor to read from
	 * @param Msg Reference to Protobuf generated message type
	 * \return false if message reception failed
	 */
	static bool rcv_msg(int sockfd, google::protobuf::Message& msg);
};
  
}

#endif 
