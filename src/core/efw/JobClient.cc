#include "JobClient.hpp"
#include "comm/SocketComm.hpp"

using namespace std;

namespace fail {

JobClient::JobClient(const std::string& server, int port)
{
	SocketComm::init();
	m_server_port = port;
	m_server = server;
	m_server_ent = gethostbyname(m_server.c_str());
  cout << "JobServer: " << m_server.c_str() << endl;
	if(m_server_ent == NULL) {
		perror("[Client@gethostbyname()]");
		// TODO: Log-level?
		exit(1);
	}
	srand(time(NULL)); // needed for random backoff (see connectToServer)
	m_server_runid = 0; // server accepts this for virgin clients
	m_job_total = 0;
	m_job_runtime_total = 0;
	m_job_throughput = 1; // client gets only one job at the first request
	m_connect_failed = false;
}

JobClient::~JobClient()
{
	// Send back completed jobs to the server
	sendResultsToServer();
}

bool JobClient::connectToServer()
{
	// don't retry server connects to speedup shutdown at campaign end
	if (m_connect_failed) {
		return false;
	}

	int retries = CLIENT_RETRY_COUNT;
	while (true) {
		// Connect to server
		struct sockaddr_in serv_addr;
		m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (m_sockfd < 0) {
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

		if (connect(m_sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
			perror("[Client@connect()]");
			close(m_sockfd);
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
			m_connect_failed = true;
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
	// die immediately if a previous connect already failed
	if (m_connect_failed) {
		return false;
	}

	while (1) { // Here we try to acquire a parameter set
		switch (tryToGetExperimentData(exp)) {
			// Jobserver will sent workload, params are set in \c exp
		case FailControlMessage::WORK_FOLLOWS:
			return true;
			// Nothing to do right now, but maybe later
		case FailControlMessage::COME_AGAIN:
			sleep(10);
			continue;
		default:
			return false;
		}
	}
}

FailControlMessage_Command JobClient::tryToGetExperimentData(ExperimentData& exp)
{

	FailControlMessage ctrlmsg;

	//Are there other jobs for the experiment
	if (m_parameters.size() == 0) {

		// Connection failed, minion can die
		if (!connectToServer()) {
			return FailControlMessage::DIE;
		}

		// Retrieve ExperimentData
		ctrlmsg.set_command(FailControlMessage::NEED_WORK);
		ctrlmsg.set_build_id(42);
		ctrlmsg.set_run_id(m_server_runid);
		ctrlmsg.set_job_size(m_job_throughput); //Request for a number of jobs

		if (!SocketComm::sendMsg(m_sockfd, ctrlmsg)) {
			// Failed to send message?  Retry.
			close(m_sockfd);
			return FailControlMessage::COME_AGAIN;
		}
		ctrlmsg.Clear();
		if (!SocketComm::rcvMsg(m_sockfd, ctrlmsg)) {
			// Failed to receive message?  Retry.
			close(m_sockfd);
			return FailControlMessage::COME_AGAIN;
		}

		// now we know the current run ID
		m_server_runid = ctrlmsg.run_id();

		switch (ctrlmsg.command()) {
		case FailControlMessage::WORK_FOLLOWS:
			uint32_t i;
			for (i = 0 ; i < ctrlmsg.job_size() ; i++) {
				ExperimentData* temp_exp = new ExperimentData(exp.getMessage().New());

				if (!SocketComm::rcvMsg(m_sockfd, temp_exp->getMessage())) {
					// Failed to receive message?  Retry.
					close(m_sockfd);
					delete temp_exp;
					return FailControlMessage::COME_AGAIN;
				}

				temp_exp->setWorkloadID(ctrlmsg.workloadid(i)); //Store workload id of experiment data
				m_parameters.push_back(temp_exp);
			}
			break;
		case FailControlMessage::COME_AGAIN:
			break;
		default:
			break;
		}
		close(m_sockfd);

		//start time measurement for throughput calculation
		m_job_runtime.startTimer();
	}

	if (m_parameters.size() != 0) {
		exp.getMessage().CopyFrom(m_parameters.front()->getMessage());
		exp.setWorkloadID(m_parameters.front()->getWorkloadID());

		delete &m_parameters.front()->getMessage();
		delete m_parameters.front();
		m_parameters.pop_front();

		return FailControlMessage::WORK_FOLLOWS;
	} else {
		return ctrlmsg.command();
	}


}

bool JobClient::sendResult(ExperimentData& result)
{
	//Create new ExperimentData for result
	ExperimentData* temp_exp = new ExperimentData(result.getMessage().New());
	temp_exp->getMessage().CopyFrom(result.getMessage());
	temp_exp->setWorkloadID(result.getWorkloadID());

	m_results.push_back( temp_exp );

	if (m_parameters.size() != 0) {
		//If job request time is over send back all existing results
		if (CLIENT_JOB_REQUEST_SEC < (double)m_job_runtime) {
			m_job_runtime_total += (double) m_job_runtime;
			m_job_runtime.reset();
			m_job_runtime.startTimer();
			m_job_total += m_results.size();
			// tell caller whether we failed phoning home
			return sendResultsToServer();
		}

		return true;
	} else {
		//Stop time measurement and calculate new throughput
		m_job_runtime.stopTimer();
		m_job_runtime_total += (double) m_job_runtime;
		m_job_total += m_results.size();
		m_job_throughput = 0.5 * m_job_throughput + 0.5*(CLIENT_JOB_REQUEST_SEC/(m_job_runtime_total/m_job_total));

		if (m_job_throughput > CLIENT_JOB_LIMIT) {
			m_job_throughput = CLIENT_JOB_LIMIT;
		} else if (m_job_throughput < 1) {
			m_job_throughput = 1;
		}

		//Timer/Counter cleanup
		m_job_runtime.reset();
		m_job_runtime_total = 0;
		m_job_total = 0;

		return sendResultsToServer();
	}
}

bool JobClient::sendResultsToServer()
{
	if (m_results.size() != 0) {
		if (!connectToServer()) {
			// clear results, although we didn't get them to safety; otherwise,
			// subsequent calls to sendResult() may and the destructor will
			// retry sending them, resulting in a large shutdown time
			while (m_results.size()) {
				delete &m_results.front()->getMessage();
				delete m_results.front();
				m_results.pop_front();
			}
			return false;
		}

		//Send back results
		FailControlMessage ctrlmsg;
		ctrlmsg.set_command(FailControlMessage::RESULT_FOLLOWS);
		ctrlmsg.set_build_id(42);
		ctrlmsg.set_run_id(m_server_runid);
		ctrlmsg.set_job_size(m_results.size()); //Store how many results will be sent

		cout << "[Client] Sending back result [";

		uint32_t i;
		for (i = 0; i < m_results.size() ; i++) {
			ctrlmsg.add_workloadid(m_results[i]->getWorkloadID());
			cout << std::dec << m_results[i]->getWorkloadID();
			cout << " ";
		}
		cout << "]";

		// TODO: Log-level?
		SocketComm::sendMsg(m_sockfd, ctrlmsg);

		for (i = 0; i < ctrlmsg.job_size() ; i++) {
			SocketComm::sendMsg(m_sockfd, m_results.front()->getMessage());
			delete &m_results.front()->getMessage();
			delete m_results.front();
			m_results.pop_front();
		}

		// Close connection.
		close(m_sockfd);
		return true;
	}
	return true;
}

} // end-of-namespace: fail
