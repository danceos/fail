#include <iostream>

#include "jobserver/messagedefs/FailControlMessage.pb.h"
#include "jobserver/SocketComm.hpp"

#include "experiments/AnExperiment/AnExperiment.pb.h"

#include <stdio.h>

using namespace std;

void error(const char *s)
{
	perror(s);
	exit(0);
}


template <typename Message>
Message *get_job(int sockfd)
{
	Message *msg = new Message;
	FailControlMessage ctrlmsg;

	ctrlmsg.set_command(FailControlMessage_Command_NEED_WORK);
	ctrlmsg.set_build_id(42);

	cout << "Sending need work msg: " << ctrlmsg.build_id() << ", Command: " << ctrlmsg.command() << endl;
	fi::SocketComm::send_msg(sockfd, ctrlmsg);
	cout << "sent ctrl message." << endl;
	fi::SocketComm::rcv_msg(sockfd, ctrlmsg);
         cout << "Received ctrl message: " << ctrlmsg.command() << endl;
	 switch(ctrlmsg.command()){
	   case FailControlMessage_Command_DIE: return 0;
	   case FailControlMessage_Command_WORK_FOLLOWS:
		fi::SocketComm::rcv_msg(sockfd, *msg);
		return msg;
	   default:
		cerr << "wtf?" << endl;
	 }
	return 0;
}

template <typename Message>
void return_result(int sockfd, Message *msg)
{
	FailControlMessage ctrlmsg;

	ctrlmsg.set_command(FailControlMessage_Command_RESULT_FOLLOWS);
	ctrlmsg.set_build_id(42);
	cout << "Sending Result msg: " << ctrlmsg.build_id() << ", Command: " << ctrlmsg.command() << endl;
	fi::SocketComm::send_msg(sockfd, ctrlmsg);
	fi::SocketComm::send_msg(sockfd, *msg);
	delete msg;
}

int main(int argc, char **argv){
	int portno;
	struct hostent *server;

	cout << "JobClient" << endl;

	if (argc < 3) {
		cerr << "usage: " << argv[0] << " hostname port" << endl;
		return 1;
	}
	portno = atoi(argv[2]);
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		cerr << "cannot resolve host " << argv[1] << endl;
		return 1;
	}
  
	int i = 1;
	while (1) {
		int sockfd;
		struct sockaddr_in serv_addr;
		cout << ">>>>>>>>>Durchgang " << i++ << endl;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			error("socket()");
		}
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
		serv_addr.sin_port = htons(portno);
		
		 if (connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
			error("connect()");
		}
  
		MHTestData *msg = get_job<MHTestData>(sockfd);
		if(!msg){
		   break;
		   close(sockfd);
		}
		cout << "[Minion] received job input: " << msg->input() << endl;
		cout << "[Minion] Calculating " << msg->input() << "^2 = " << msg->input() * msg->input() << endl; 
		msg->set_output(msg->input() * msg->input());
		sleep(1);
		cout << "[Minion] returning result: " << msg->output() << endl;

		return_result<MHTestData>(sockfd, msg);

		close(sockfd);
	}
	cout << "ByeBye" << endl;
	return 0;
}

