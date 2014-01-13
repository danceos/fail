/**
 * \brief Socket based communication wrapper functions.
 */

#ifndef __SOCKET_COMM_HPP__
  #define __SOCKET_COMM_HPP__

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <google/protobuf/message.h>

namespace fail {

class SocketComm {
public:
	/**
	 * This allows us to ignore SIGPIPE.
	 */
	static void init();
	/**
	 * Send Protobuf-generated message
	 * @param sockfd open socket descriptor to write to
	 * @param Msg Reference to Protobuf generated message type
	 * \return false if message sending failed
	 */
	static bool sendMsg(int sockfd, google::protobuf::Message& msg);
	/**
	 * Receive Protobuf-generated message
	 * @param sockfd open socket descriptor to read from
	 * @param Msg Reference to Protobuf generated message type
	 * \return false if message reception failed
	 */
	static bool rcvMsg(int sockfd, google::protobuf::Message& msg);

private:
	static ssize_t safe_write(int fd, const void *buf, size_t count);
	static ssize_t safe_read(int fd, void *buf, size_t count);
};
  
} // end-of-namespace: fail

#endif // __SOCKET_COMM_HPP__
