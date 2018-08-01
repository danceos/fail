#include <chrono>
#include <random>
#include <string>
#include <thread>

#include <boost/optional.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "JobClient.hpp"

using namespace std;
using namespace boost::asio;

namespace fail {

struct JobClient::impl {
	io_service ios;
	ip::tcp::socket socket;
	boost::optional<ip::tcp::endpoint> endpoint;

	impl() : socket(ios) {}
};

JobClient::JobClient(const std::string& server, int port)
	: m_d(new impl), m_server(server), m_server_port(port),
	m_server_runid(0), // server accepts this for virgin clients
	m_job_runtime_total(0),
	m_job_throughput(CLIENT_JOB_INITIAL), // will be corrected after measurement
	m_job_total(0),
	m_connect_failed(false)
{
	cout << "JobServer: " << server << ":" << port << endl;
	srand(time(NULL)); // needed for random backoff (see connectToServer)
}

JobClient::~JobClient()
{
	// Send back completed jobs to the server
	sendResultsToServer();
	delete m_d;
}

bool JobClient::connectToServer()
{
	// don't retry server connects to speedup shutdown at campaign end
	if (m_connect_failed) {
		return false;
	}

	// random engine for backoff.
	std::mt19937_64 engine(time(NULL));

	for (int tries = CLIENT_RETRY_COUNT + 1; tries > 0; --tries) {
		// resolve endpoint lazily
		if (!m_d->endpoint) {
			boost::asio::ip::tcp::resolver resolver(m_d->ios);
			boost::asio::ip::tcp::resolver::query query(
				m_server, std::to_string(m_server_port));

			for (ip::tcp::resolver::iterator end,
				 addrs = resolver.resolve(query);
				 addrs != end; ++addrs) {
				// server listens on IPv4 endpoint only, skip IPv6 endpoints
				if (addrs->endpoint().address().is_v4()) {
					m_d->endpoint = addrs->endpoint();
					break;
				}
			}

			if (!m_d->endpoint) {
				cerr << "[Client] Failed to resolve " << m_server << endl;
				continue;
			}
		}

		boost::system::error_code error;
		m_d->socket.connect(*m_d->endpoint, error);
		if (!error) {
			cout << "[Client] Connection established!" << endl;
			return true;
		}
		perror("[Client@connect()]");
		m_d->socket.close();

		if (tries > 1) {
			std::uniform_real_distribution<> distribution(
				CLIENT_RAND_BACKOFF_TSTART, CLIENT_RAND_BACKOFF_TEND);
			const auto delay = std::chrono::duration<double>(distribution(engine));
			cout << "[Client] Retrying to connect to server in ~" << delay.count()
				 << "s..." << endl;
			std::this_thread::sleep_for(delay);
		}
	}

	cout << "[Client] Unable to reconnect (tried " << CLIENT_RETRY_COUNT
	     << " times); I'll give it up!" << endl;
	m_connect_failed = true;
	return false;
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

template <typename Socket>
bool sendMsg(Socket &s, google::protobuf::Message &msg)
{
	int size = htonl(msg.ByteSize());
	const auto msg_size = msg.ByteSize() + sizeof(size);
	std::string buf;

	if (!msg.SerializeToString(&buf))
		return false;

	boost::array<const_buffer, 2> bufs{buffer(&size, sizeof(size)),
					   buffer(buf)};

	boost::system::error_code ec;
	const auto len = boost::asio::write(s, bufs, ec);
	if (ec || len != msg_size) {
		std::cerr << ec.message() << std::endl;
		std::cerr << "Sent " << len << " instead of " << msg_size
			  << " bytes from socket" << std::endl;
		return false;
	}

	return true;
}

template <typename Socket>
bool rcvMsg(Socket &s, google::protobuf::Message &msg)
{
	int size;
	std::size_t len;
	boost::system::error_code ec;

	len = boost::asio::read(s, buffer(&size, sizeof(size)), ec);
	if (ec || len != sizeof(size)) {
		std::cerr << ec.message() << std::endl;
		std::cerr << "Read " << len << " instead of " << sizeof(size)
			  << " bytes from socket" << std::endl;
		return false;
	}

	const auto msglen = ntohl(size);
	std::vector<char> buf(msglen);
	len = boost::asio::read(s, buffer(buf), ec);
	if (ec || len != msglen) {
		std::cerr << ec.message() << std::endl;
		std::cerr << "Read " << len << " instead of " << msglen
			  << " bytes from socket" << std::endl;
		return false;
	}

	return msg.ParseFromArray(buf.data(), buf.size());
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

		if (!sendMsg(m_d->socket, ctrlmsg)) {
			m_d->socket.close();
			// Failed to send message?  Retry.
			return FailControlMessage::COME_AGAIN;
		}
		ctrlmsg.Clear();
		if (!rcvMsg(m_d->socket, ctrlmsg)) {
			m_d->socket.close();
			// Failed to receive message?  Retry.
			return FailControlMessage::COME_AGAIN;
		}

		// now we know the current run ID
		m_server_runid = ctrlmsg.run_id();

		switch (ctrlmsg.command()) {
		case FailControlMessage::WORK_FOLLOWS:
			uint32_t i;
			for (i = 0; i < ctrlmsg.job_size(); i++) {
				ExperimentData* temp_exp = new ExperimentData(exp.getMessage().New());

				if (!rcvMsg(m_d->socket, temp_exp->getMessage())) {
					// looks like we won't receive more jobs now, cleanup
					delete &temp_exp->getMessage();
					delete temp_exp;
					// did a previous loop iteration succeed?
					if (m_parameters.size() > 0) {
						break;
					} else {
						// nothing to do now, retry later
						return FailControlMessage::COME_AGAIN;
					}
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
		m_d->socket.close();

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

	m_results.push_back(temp_exp);

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
		for (i = 0; i < m_results.size(); i++) {
			ctrlmsg.add_workloadid(m_results[i]->getWorkloadID());
			cout << std::dec << m_results[i]->getWorkloadID();
			cout << " ";
		}
		cout << "]";

		// TODO: Log-level?
		if (!sendMsg(m_d->socket, ctrlmsg)) {
			m_d->socket.close();
			return false;
		}

		for (i = 0; i < ctrlmsg.job_size(); i++) {
			if (!sendMsg(m_d->socket, m_results.front()->getMessage())) {
				m_d->socket.close();
				return false;
			}
			delete &m_results.front()->getMessage();
			delete m_results.front();
			m_results.pop_front();
		}

		// Close connection.
		m_d->socket.close();
		return true;
	}
	return true;
}

} // end-of-namespace: fail
