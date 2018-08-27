// <iostream> needs to be included before *.pb.h, otherwise ac++/Puma chokes on the latter
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <thread>
#include <tuple>

#include "JobServer.hpp"

#ifndef __puma
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#endif

using namespace std;

namespace fail {

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::system;

/**
 * @class CommThread
 * Implementation of the communication threads.
 * This class implements the actual communication
 * with the minions.
 */
class CommThread {
private:
	tcp::socket m_socket;
	uint32_t m_job_size;
	JobServer &m_js; //! Calling jobserver

	// FIXME: Concerns are not really separated yet ;)
	/**
	 * Called after minion calls for work.
	 * Tries to deque a parameter set non blocking, and
	 * sends it back to the requesting minion.
	 * @param minion The minion asking for input
	 */
	void sendPendingExperimentData(yield_context yield);
	/**
	 * Called after minion offers a result message.
	 * Evaluates the Workload ID and puts the corresponding
	 * job result into the result queue.
	 * @param minion The minion offering results
	 * @param workloadID The workload id of the result message
	 */
	void receiveExperimentResults(FailControlMessage &ctrlmsg,
				      yield_context yield);

	enum ProgressType { Send, Receive, Resend };
	void print_progress(const enum ProgressType, const uint32_t,
			    const uint32_t);

public:
	CommThread(tcp::socket socket, JobServer &p)
	    : m_socket(std::move(socket)), m_job_size(1), m_js(p) {}

	/**
	 * The thread's entry point.
	 */
	void operator()(yield_context yield);
};

namespace AsyncSocket {

static bool rcvMsg(tcp::socket &socket, google::protobuf::Message &msg,
		   yield_context yield, bool drop = false)
{
	boost::system::error_code ec;
	int size;
	size_t len = async_read(socket, buffer(&size, sizeof(size)), yield[ec]);
	if (ec || len != sizeof(size)) {
		std::cerr << ec.message() << std::endl;
		std::cerr << "Read " << len << " instead of " << sizeof(size)
			  << " bytes from socket" << std::endl;
		return false;
	}
	const size_t msg_size = ntohl(size);

	std::vector<char> buf(msg_size);
	len = async_read(socket, buffer(buf), yield[ec]);
	if (ec || len != msg_size) {
		std::cerr << ec.message() << std::endl;
		std::cerr << "Read " << len << " instead of " << msg_size
			  << " bytes from socket" << std::endl;
		return false;
	}

	return drop ? true : msg.ParseFromArray(buf.data(), buf.size());
}

static bool dropMsg(tcp::socket &socket, yield_context yield) {
	FailControlMessage msg;
	return rcvMsg(socket, msg, yield, true);
}

static bool sendMsg(tcp::socket &socket, google::protobuf::Message &msg,
		    yield_context yield)
{
	std::string buf;
	if (!msg.SerializeToString(&buf)) {
		return false;
	}

	const int size = htonl(msg.ByteSize());
	boost::array<const_buffer, 2> bufs{buffer(&size, sizeof(size)),
					   buffer(buf)};
	boost::system::error_code ec;
	async_write(socket, bufs, yield[ec]);
	if (ec) {
		std::cerr << ec.message() << std::endl;
		return false;
	}

	return true;
}
}

struct JobServer::impl {
	io_service accept_service;
	io_service comm_service;
	std::thread comm_thread;
	std::atomic<uint64_t> redundant_results{0};

	//!  Campaign signaled last experiment data set
	std::atomic_bool noMoreExps{false};

	impl()
	    : comm_thread([this] {
		      io_service::work work(comm_service);
		      comm_service.run();
		      std::cout << "Received " << redundant_results
				<< " redundant results." << std::endl;
	      })
	{
	}

	~impl()
	{
		comm_service.stop();
		if (comm_thread.joinable()) {
			comm_thread.join();
		}
	}
};

JobServer::JobServer(const unsigned short port)
    : m_d(std::make_shared<impl>()), m_port(port), m_finish(false),
      m_threadtimeout(0), m_undoneJobs(SERVER_OUT_QUEUE_SIZE)
{
	m_runid = std::time(0);
#ifndef __puma
	m_serverThread = new boost::thread(&JobServer::run, this);
#ifdef SERVER_PERFORMANCE_MEASURE
	m_measureThread = new boost::thread(&JobServer::measure, this);
#endif
#endif
}

void JobServer::addParam(ExperimentData* exp)
{
#ifndef __puma
	m_inOutCounter.increment();
	m_undoneJobs.Enqueue(exp);
#endif
}

ExperimentData *JobServer::getDone()
{
#ifndef __puma
	ExperimentData *exp = m_doneJobs.Dequeue();
	if (exp) {
		m_inOutCounter.decrement();
	}
	return exp;
#endif
}

bool JobServer::noMoreExperiments() const { return m_d->noMoreExps; }

void JobServer::setNoMoreExperiments()
{
	boost::unique_lock<boost::mutex> lock(m_CommMutex);
	m_d->noMoreExps = true;
	m_undoneJobs.setIsFinished();

	if (m_undoneJobs.Size() == 0 &&
	    m_runningJobs.Size() == 0) {
		m_doneJobs.setIsFinished();
	}
}

#ifdef SERVER_PERFORMANCE_MEASURE
void JobServer::measure()
{
	// TODO: Log-level?
	cout << "\n[Server] Logging throughput in \"" << SERVER_PERF_LOG_PATH << "\"..." << endl;
	ofstream m_file(SERVER_PERF_LOG_PATH, std::ios::trunc); // overwrite existing perf-logs
	if (!m_file.is_open()) {
		cerr << "[Server] Perf-logging has been enabled"
		     << "but I was not able to write the log-file \""
		     << SERVER_PERF_LOG_PATH << "\"." << endl;
		exit(1);
	}
	unsigned counter = 0;

	m_file << "time\tthroughput" << endl;
	uint64_t diff = 0;
	while (!m_finish) {
		// Format: 1st column (seconds)[TAB]2nd column (throughput)
		m_file << counter << "\t" << (m_DoneCount - diff) << endl;
		counter += SERVER_PERF_STEPPING_SEC;
		diff = m_DoneCount;
		sleep(SERVER_PERF_STEPPING_SEC);
	}
	// NOTE: Summing up the values written in the 2nd column does not
	// necessarily yield the number of completed experiments/jobs
	// (due to thread-scheduling behaviour -> not sync'd!)
}
#endif // SERVER_PERFORMANCE_MEASURE

void JobServer::done()
{
	m_d->accept_service.stop();
	m_d->comm_service.stop();
}

void JobServer::run()
{
	const auto ep = tcp::endpoint(tcp::v4(), m_port);
	auto acceptor = tcp::acceptor(m_d->accept_service, ep, true);
	{
		const auto local_ep = acceptor.local_endpoint();
		std::cout << "Listening on " << local_ep.address() << ":"
			  << local_ep.port() << std::endl;
	}

	spawn(m_d->accept_service, [this, &acceptor](yield_context yield) {
		for (;;) {
			tcp::socket socket(m_d->comm_service);
			boost::system::error_code ec;
			acceptor.async_accept(socket, yield[ec]);
			if (ec) {
				std::cerr
				    << "Error accept()ing a new connection "
				    << ec.message() << std::endl;
				continue;
			}

			spawn(m_d->comm_service,
			      [ socket = std::move(socket),
				this ](yield_context yield) mutable {
				      CommThread coro(std::move(socket), *this);
				      coro(yield);
			      });
		}
	});

	m_d->accept_service.run();
}

void CommThread::print_progress(const enum ProgressType type,
				const uint32_t w_id, const uint32_t count)
{
	using namespace std::chrono;
	const auto now = steady_clock::now();
	const auto delay = milliseconds{500};
	static steady_clock::time_point last = steady_clock::now() - delay;

	if (last + delay > now) {
		return;
	}

	const auto rate_alpha = .1;
	static float rate = 0;
	static uint64_t donecount_last = 0;
	uint64_t donecount_cur = m_js.m_DoneCount; // assuming atomic here
	rate = rate_alpha *
		(donecount_cur - donecount_last) / duration<float>(now - last).count() +
		(1 - rate_alpha) * rate;

	std::cout
		<< std::setw(6) << m_js.m_undoneJobs.Size() << " out/"
		<< std::setw(6) << m_js.m_runningJobs.Size() << " run/"
		<< std::setw(6) << donecount_cur << " tot/ ("
		<< std::setw(6) << std::setprecision(1) << std::fixed << rate << "/s)  ";
	if (m_js.m_TotalCount) {
		float percentage = (float) donecount_cur / *m_js.m_TotalCount * 100.0;
		std::cout << std::setw(4) << std::setprecision(1) << std::fixed << percentage << "%";
		if (rate > 0) {
			float ETA_s = std::max(.0f, (*m_js.m_TotalCount - donecount_cur) / rate);
			std::cout << " (ETA " << std::dec
				<< std::setw(2) << std::setfill('0') <<  ((int64_t)ETA_s / (60*60)) << ':'
				<< std::setw(2) << std::setfill('0') << (((int64_t)ETA_s / 60) % 60) << ':'
				<< std::setw(2) << std::setfill('0') <<  ((int64_t)ETA_s % 60)
				<< std::setfill(' ') << ')';
		}
		std::cout << "   ";
	}
	const char *sep;
	if (type == ProgressType::Send) {
		sep = " >";
	} else if (type == ProgressType::Resend) {
		sep = ">>";
	} else {
		sep = " <";
	}
	std::cout << sep << '[' << w_id << '+' << count << "]  \r" << std::flush;

	last = now;
	donecount_last = donecount_cur;
}

void CommThread::operator()(yield_context yield)
{
	// The communication thread implementation:
	FailControlMessage ctrlmsg;

	if (!AsyncSocket::rcvMsg(m_socket, ctrlmsg, yield)) {
		cout << "!![Server] failed to read complete message from client" << endl;
		return;
	}

	switch (ctrlmsg.command()) {
	case FailControlMessage::NEED_WORK:
		// let old clients die (run_id == 0 -> possibly virgin client)
		if (!ctrlmsg.has_run_id() || (ctrlmsg.run_id() != 0 && ctrlmsg.run_id() != m_js.m_runid)) {
			cout << "!![Server] telling old client to die" << endl;
			ctrlmsg.Clear();
			ctrlmsg.set_command(FailControlMessage::DIE);
			ctrlmsg.set_build_id(42);
			AsyncSocket::sendMsg(m_socket, ctrlmsg, yield);
			break;
		}
		// give minion something to do..
		m_job_size = ctrlmsg.job_size();
		sendPendingExperimentData(yield);
		break;
	case FailControlMessage::RESULT_FOLLOWS:
		// ignore old client's results
		if (!ctrlmsg.has_run_id() || ctrlmsg.run_id() != m_js.m_runid) {
			cout << "!![Server] ignoring old client's results" << endl;
			break;
		}
		// get results and put to done queue.
		receiveExperimentResults(ctrlmsg, yield);
		break;
	default:
		// hm.. don't know what to do. please die.
		cout << "!![Server] no idea what to do with command #"
		     << ctrlmsg.command() << ", telling minion to die." << endl;
		ctrlmsg.Clear();
		ctrlmsg.set_command(FailControlMessage::DIE);
		ctrlmsg.set_build_id(42);
		AsyncSocket::sendMsg(m_socket, ctrlmsg, yield);
	}
}

void CommThread::sendPendingExperimentData(yield_context yield)
{
	uint32_t i;
	uint32_t workloadID;
	std::deque<ExperimentData*> exp;
	ExperimentData* temp_exp = 0;
	FailControlMessage ctrlmsg;

	ctrlmsg.set_build_id(42);
	ctrlmsg.set_run_id(m_js.m_runid);
	ctrlmsg.set_command(FailControlMessage::WORK_FOLLOWS);

	for (i = 0; i < m_job_size; i++) {
		if (m_js.m_undoneJobs.Dequeue_nb(temp_exp) == true) {
			// Got an element from queue, assign ID to workload and send to minion
			workloadID = m_js.m_counter.increment(); // increment workload counter
			temp_exp->setWorkloadID(workloadID); // store ID for identification when receiving result
			ctrlmsg.add_workloadid(workloadID);
			exp.push_back(temp_exp);
		} else {
			break;
		}
	}
	if (exp.size() != 0) {
		ctrlmsg.set_job_size(exp.size());

		print_progress(ProgressType::Send, ctrlmsg.workloadid(0),
			       exp.size());

		if (AsyncSocket::sendMsg(m_socket, ctrlmsg, yield)) {
			for (i = 0; i < ctrlmsg.job_size(); i++) {
				if (AsyncSocket::sendMsg(m_socket, exp.front()->getMessage(), yield)) {

					// delay insertion into m_runningJobs until here, as
					// getMessage() won't work anymore if this job is re-sent,
					// received, and deleted in the meantime
					if (!m_js.m_runningJobs.insert(exp.front()->getWorkloadID(), exp.front())) {
						cout << "!![Server]could not insert workload id: [" << workloadID << "] double entry?" << endl;
					}

					exp.pop_front();
				} else {
					// add remaining jobs back to the queue
					cout << "!![Server] failed to send scheduled " << exp.size() << " jobs" << endl;
					while (exp.size()) {
						m_js.m_undoneJobs.Enqueue(exp.front());
						exp.pop_front();
					}
					break;
				}

			}
		}
		return;
	}

	{ // Don't indent properly to reduce patch-noise.
#ifndef __puma
	// Prevent receiveExperimentResults from modifying (or indirectly, via
	// getDone and the campaign, deleting) jobs in the m_runningJobs queue.
	// (See details in receiveExperimentResults)
	boost::unique_lock<boost::mutex> lock(m_js.m_CommMutex);
#endif
	if ((temp_exp = m_js.m_runningJobs.pickone()) != NULL) { // 2nd priority
		// (This picks one running job.)
		// TODO: Improve selection of parameter set to be resent:
		//  -  currently: Linear complexity!
		//  -  pick entry at random?
		//  -  retry counter for each job?

		// Implement resend of running-parameter sets to improve campaign speed
		// and to prevent result loss due to (unexpected) termination of experiment
		// clients.
		// (Note: Therefore we need to be aware of receiving multiple results for a
		//        single parameter-set, @see receiveExperimentResults.)
		uint32_t workloadID = temp_exp->getWorkloadID(); // (this ID has been set previously)
		// Resend the parameter-set.
		ctrlmsg.set_command(FailControlMessage::WORK_FOLLOWS);
		ctrlmsg.add_workloadid(workloadID); // set workload id
		ctrlmsg.set_job_size(1); // In 2nd priority the jobserver send only one job
		print_progress(ProgressType::Resend, workloadID, 1);
	} else if (m_js.noMoreExperiments() == false) {
		// Currently we have no workload (even the running-job-queue is empty!), but
		// the campaign is not over yet. Minion can try again later.
		ctrlmsg.set_command(FailControlMessage::COME_AGAIN);
	} else {
		// No work & Camapaign is done. Go away.
		ctrlmsg.set_command(FailControlMessage::DIE);
	}
	}

	const bool success = AsyncSocket::sendMsg(m_socket, ctrlmsg, yield);
	if (temp_exp != nullptr && success) {
		AsyncSocket::sendMsg(m_socket, temp_exp->getMessage(), yield);
	}
}

void CommThread::receiveExperimentResults(FailControlMessage &ctrlmsg,
					  yield_context yield)
{
	int i;
	ExperimentData* exp = NULL; // Get exp* from running jobs
	if (ctrlmsg.workloadid_size() > 0) {
		print_progress(ProgressType::Receive, ctrlmsg.workloadid(0),
			       ctrlmsg.workloadid_size());
	}

	// No yielding under locks ;)
	std::vector<std::tuple<ExperimentData *, uint32_t>> msgs;
	msgs.reserve(ctrlmsg.workloadid_size());

	{ // Don't indent properly to reduce patch-noise.
#ifndef __puma
	// Prevent re-sending jobs in sendPendingExperimentData:
	// a) sendPendingExperimentData needs an intact job to serialize and send it.
	// b) After moving the job to m_doneJobs, it may be retrieved and deleted
	//    by the campaign at any time.
	// Additionally, receiving a result overwrites the job's contents.  This
	// already may cause breakage in sendPendingExperimentData (a).
	boost::unique_lock<boost::mutex> lock(m_js.m_CommMutex);
#endif
	for (i = 0; i < ctrlmsg.workloadid_size(); i++) {
		const uint32_t id = ctrlmsg.workloadid(i);
		const bool success = m_js.m_runningJobs.remove(id, exp);
		msgs.emplace_back(success ? exp : nullptr, id);
	}
	}

	/* Do I/O */
	std::vector<std::tuple<ExperimentData *, uint32_t, bool>> recvd;
	msgs.reserve(ctrlmsg.workloadid_size());
	for (auto &&msg : msgs) {
		auto &&exp = std::get<0>(msg);
		if (exp != nullptr) {
			const auto w_id = std::get<1>(msg);
			const bool success = AsyncSocket::rcvMsg(
			    m_socket, exp->getMessage(), yield);
			recvd.emplace_back(exp, w_id, success);
			++m_js.m_DoneCount;
		} else {
			// We can receive several results for the same workload id because
			// we (may) distribute the (running) jobs to a *few* experiment-clients.
			++m_js.m_d->redundant_results;
			AsyncSocket::dropMsg(m_socket, yield);
		}
	}

#ifndef __puma
	boost::unique_lock<boost::mutex> lock(m_js.m_CommMutex);
#endif
	for (auto &&rcv : recvd) {
		auto &&exp = std::get<0>(rcv);
		const auto received = std::get<2>(rcv);
		if (received) {
			m_js.m_doneJobs.Enqueue(exp);
		} else {
			const auto w_id = std::get<1>(rcv);
			m_js.m_runningJobs.insert(w_id, exp);
		}
	}

	// all results complete?
	if (m_js.m_undoneJobs.Size() == 0 &&
	    m_js.noMoreExperiments() &&
	    m_js.m_runningJobs.Size() == 0) {
		m_js.m_doneJobs.setIsFinished();
	}
}

} // end-of-namespace: fail
