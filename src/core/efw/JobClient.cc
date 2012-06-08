#include "JobClient.hpp"

using namespace std;

namespace fail {

JobClient::JobClient(const std::string& server, int port)
{
	m_server_port = port;
	m_server = server;
	m_server_ent = gethostbyname(m_server.c_str());
	if(m_server_ent == NULL) {
		perror("[Client@gethostbyname()]");
		// TODO: Log-level?
		exit(1);
	}
	srand(time(NULL)); // needed for random backoff (see connectToServer)
}

bool JobClient::connectToServer()
{
	// Connect to server
	struct sockaddr_in serv_addr;
	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sockfd < 0) {
		perror("[Client@socket()]");
		// TODO: Log-level?
		exit(0);
	}

	/* Enable address reuse */
	int on = 1;
	setsockopt( m_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, m_server_ent->h_addr, m_server_ent->h_length);
	serv_addr.sin_port = htons(m_server_port);
	
	int retries = CLIENT_RETRY_COUNT;
	while (true) {
		if (connect(m_sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
			perror("[Client@connect()]");
			// TODO: Log-level?
			if (retries > 0) {
				// Wait CLIENT_RAND_BACKOFF_TSTART to RAND_BACKOFF_TEND seconds:
				int delay = rand() % (CLIENT_RAND_BACKOFF_TEND-CLIENT_RAND_BACKOFF_TSTART) + CLIENT_RAND_BACKOFF_TSTART;
				cout << "[Client] Retrying to connect to server in ~" << delay << "s..." << endl;
				// TODO: Log-level?
				sleep(delay);
				usleep(rand() % 1000000);
				--retries;
				continue;
			}
			cout << "[Client] Unable to reconnect (tried " << CLIENT_RETRY_COUNT << " times); "
			     << "I'll give it up!" << endl;
			     // TODO: Log-level?
			return false; // finally: unable to connect, give it up :-(
		}
		break; // connected! :-)
	}
	cout << "[Client] Connection established!" << endl;
	// TODO: Log-level?

	return true;
}

bool JobClient::getParam(ExperimentData& exp)
{
	while (1) { // Here we try to acquire a parameter set
		switch (tryToGetExperimentData(exp)) {
			// Jobserver will sent workload, params are set in \c exp
		case FailControlMessage_Command_WORK_FOLLOWS: return true;
			// Nothing to do right now, but maybe later
		case FailControlMessage_Command_COME_AGAIN:
			sleep(1);
			continue;
		default:
			return false;
		}
	}
}

FailControlMessage_Command JobClient::tryToGetExperimentData(ExperimentData& exp)
{
	// Connection failed, minion can die
	if (!connectToServer())
		return FailControlMessage_Command_DIE;

	// Retrieve ExperimentData
	FailControlMessage ctrlmsg;
	ctrlmsg.set_command(FailControlMessage_Command_NEED_WORK);
	ctrlmsg.set_build_id(42);

	SocketComm::sendMsg(m_sockfd, ctrlmsg);
	ctrlmsg.Clear();
	SocketComm::rcvMsg(m_sockfd, ctrlmsg);

	switch (ctrlmsg.command()) {
	case FailControlMessage_Command_WORK_FOLLOWS:
		SocketComm::rcvMsg(m_sockfd, exp.getMessage());
		exp.setWorkloadID(ctrlmsg.workloadid()); // Store workload id of experiment data
		break;
	case FailControlMessage_Command_COME_AGAIN:
		break;
	default:
		break;  
	}
	close(m_sockfd);
	return ctrlmsg.command();
}

bool JobClient::sendResult(ExperimentData& result)
{
	if (!connectToServer())
		return false;

	// Send back results
	FailControlMessage ctrlmsg;
	ctrlmsg.set_command(FailControlMessage_Command_RESULT_FOLLOWS);
	ctrlmsg.set_build_id(42);
	ctrlmsg.set_workloadid(result.getWorkloadID());	
	cout << "[Client] Sending back result [" << std::dec << result.getWorkloadID() << "]..."  << endl;
	// TODO: Log-level?
	SocketComm::sendMsg(m_sockfd, ctrlmsg);
	SocketComm::sendMsg(m_sockfd, result.getMessage());

	// Close connection.
	close(m_sockfd);
	return true;
}

} // end-of-namespace: fail
