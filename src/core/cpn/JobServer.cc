// <iostream> needs to be included before *.pb.h, otherwise ac++/Puma chokes on the latter
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>

#include "comm/FailControlMessage.pb.h"
#include "comm/SocketComm.hpp"
#include "JobServer.hpp"
#include "Minion.hpp"

#ifndef __puma
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#endif

using namespace std;

namespace fail {

void JobServer::addParam(ExperimentData* exp)
{
#ifndef __puma
	m_undoneJobs.Enqueue(exp);
	m_inOutCounter.increment();
#endif
}

#ifdef SERVER_PERFORMANCE_MEASURE
volatile unsigned JobServer::m_DoneCount = 0;
#endif

#ifndef __puma
boost::mutex CommThread::m_CommMutex;
#endif

ExperimentData *JobServer::getDone()
{
	
#ifndef __puma
	boost::unique_lock<boost::mutex> lock(CommThread::m_CommMutex);

	if (m_undoneJobs.Size() == 0
	 && noMoreExperiments()
	 && m_runningJobs.Size() == 0
	 && m_doneJobs.Size() == 0
	 && m_inOutCounter.getValue() == 0) {
		return 0;
	}
	
	ExperimentData *exp = NULL;
	exp = m_doneJobs.Dequeue();
	m_inOutCounter.decrement();
	return exp;
#endif
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
	unsigned diff = 0;
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

#ifndef __puma
/**
 * This is a predicate class for the remove_if operator on the thread
 * list. The operator waits for timeout milliseconds to join each
 * thread in the list. If the join was successful, the exited thread
 * is deallocated and removed from the list.
 */
struct timed_join_successful {
	int timeout_ms;
	timed_join_successful(int timeout)
		: timeout_ms(timeout) { }

	bool operator()(boost::thread* threadelement)
	{
		boost::posix_time::time_duration timeout = boost::posix_time::milliseconds(timeout_ms);
		if (threadelement->timed_join(timeout)) {
			delete threadelement;
			return true;
		} else {
			return false;
		}
	}
};
#endif

void JobServer::run()
{
	struct sockaddr_in clientaddr;
	socklen_t clen = sizeof(clientaddr);
	
	// implementation of server-client communication
	int s;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		// TODO: Log-level?
		return;
	}

	/* Enable address reuse */
	int on = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		perror("setsockopt");
		// TODO: Log-level?
		return;
	}
	
	/* IPv4, bind to all interfaces */
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(m_port);
	saddr.sin_addr.s_addr = htons(INADDR_ANY);
 
	/* bind to port */
	if (::bind(s, (struct sockaddr*) &saddr, sizeof(saddr)) == -1) {
		perror("bind");
		// TODO: Log-level?
		return;
	}
 
	/* Listen with a backlog of maxThreads */
	if (listen(s, m_maxThreads) == -1) {
		perror("listen");
		// TODO: Log-level?
		return;
	}
	cout << "JobServer listening...." << endl;
	// TODO: Log-level?
#ifndef __puma
	boost::thread* th;
	while (!m_finish){
		// Accept connection 
		int cs = accept(s, (struct sockaddr*)&clientaddr, &clen);
		if (cs == -1) {
			perror("accept");
			// TODO: Log-level?
			return;
		}
		// Spawn a thread for further communication,
		// and add this thread to a list threads
		// We can limit the generation of threads here.
		if (m_threadlist.size() >= m_maxThreads) {
			// Run over list with a timed_join,
			// removing finished threads.
			do {
				m_threadlist.remove_if(timed_join_successful(m_threadtimeout));
			} while (m_threadlist.size() == m_maxThreads);
		}
		// Start new thread	
		th = new boost::thread(CommThread(cs, *this));
		m_threadlist.push_back(th);
	}
	close(s);
	// when all undone Jobs are distributed  -> call a timed_join on all spawned
	// TODO: interrupt threads that do not want to join..
	while (m_threadlist.size() > 0)
		m_threadlist.remove_if( timed_join_successful(m_threadtimeout) );
#endif
}

void CommThread::operator()()
{
	// The communication thread implementation:

	Minion minion;
	FailControlMessage ctrlmsg;
	minion.setSocketDescriptor(m_sock);

	if (!SocketComm::rcvMsg(minion.getSocketDescriptor(), ctrlmsg)) {
		cout << "!![Server] failed to read complete message from client" << endl;
		close(m_sock);
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
			SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg);
		}
		// give minion something to do..
		sendPendingExperimentData(minion);
		break;
	case FailControlMessage::RESULT_FOLLOWS:
		// ignore old client's results
		if (!ctrlmsg.has_run_id() || ctrlmsg.run_id() != m_js.m_runid) {
			cout << "!![Server] ignoring old client's results" << endl;
			break;
		}
		// get results and put to done queue.
		receiveExperimentResults(minion, ctrlmsg.workloadid());
		break;
	default:
		// hm.. don't know what to do. please die.
		cout << "!![Server] no idea what to do with command #"
		     << ctrlmsg.command() << ", telling minion to die." << endl;
		ctrlmsg.Clear();
		ctrlmsg.set_command(FailControlMessage::DIE);
		ctrlmsg.set_build_id(42);
		SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg);
	}

	close(m_sock);
}

void CommThread::sendPendingExperimentData(Minion& minion)
{
	FailControlMessage ctrlmsg;
	ctrlmsg.set_build_id(42);
	ctrlmsg.set_run_id(m_js.m_runid);
	ExperimentData * exp = 0;
	if (m_js.m_undoneJobs.Dequeue_nb(exp) == true) { 
		// Got an element from queue, assign ID to workload and send to minion
		uint32_t workloadID = m_js.m_counter.increment(); // increment workload counter
		exp->setWorkloadID(workloadID);			  // store ID for identification when receiving result
		if (!m_js.m_runningJobs.insert(workloadID, exp)) {
			cout << "!![Server]could not insert workload id: [" << workloadID << "] double entry?" << endl;
		}
		ctrlmsg.set_command(FailControlMessage::WORK_FOLLOWS);
		ctrlmsg.set_workloadid(workloadID); // set workload id
		//cout << ">>[Server] Sending workload [" << workloadID << "]" << endl;
		cout << ">>[" << workloadID << "] " << flush;
		if (SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg)) {
			SocketComm::sendMsg(minion.getSocketDescriptor(), exp->getMessage());
		}
		return;
	}

  #ifndef __puma
	boost::unique_lock<boost::mutex> lock(m_CommMutex);
  #endif
	if ((exp = m_js.m_runningJobs.pickone()) != NULL) { // 2nd priority
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
		uint32_t workloadID = exp->getWorkloadID(); // (this ID has been set previously)
		// Resend the parameter-set.
		ctrlmsg.set_command(FailControlMessage::WORK_FOLLOWS);
		ctrlmsg.set_workloadid(workloadID); // set workload id
		//cout << ">>[Server] Re-sending workload [" << workloadID << "]" << endl;
		cout << ">>R[" << workloadID << "] " << flush;
		if (SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg)) {
			SocketComm::sendMsg(minion.getSocketDescriptor(), exp->getMessage());
		}
	} else if (m_js.noMoreExperiments() == false) { 
		// Currently we have no workload (even the running-job-queue is empty!), but
		// the campaign is not over yet. Minion can try again later.
		ctrlmsg.set_command(FailControlMessage::COME_AGAIN);
		SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg);
		cout << "--[Server] No workload, come again..."  <<  endl;
	} else {
		// No more elements, and campaign is over. Minion can die.
		ctrlmsg.set_command(FailControlMessage::DIE);
		cout << "--[Server] No workload, and no campaign, please die." << endl;
		SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg);
	}
}

void CommThread::receiveExperimentResults(Minion& minion, uint32_t workloadID)
{
	ExperimentData* exp = NULL; // Get exp* from running jobs
	//cout << "<<[Server] Received result for workload id [" << workloadID << "]" << endl;
	cout << "<<[" << workloadID << "] " << flush;
	if (m_js.m_runningJobs.remove(workloadID, exp)) { // ExperimentData* found
		// deserialize results, expect failures
		if (!SocketComm::rcvMsg(minion.getSocketDescriptor(), exp->getMessage())) {
			m_js.m_runningJobs.insert(workloadID, exp);
		} else {
			m_js.m_doneJobs.Enqueue(exp); // Put results in done queue
		}
	  #ifdef SERVER_PERFORMANCE_MEASURE
		++JobServer::m_DoneCount;
	  #endif
	} else {
		// We can receive several results for the same workload id because
		// we (may) distribute the (running) jobs to a *few* experiment-clients.
		cout << "[Server] Received another result for workload id ["
		     << workloadID << "] -- ignored." << endl;
		
		// TODO: Any need for error-handling here?
	}
}

} // end-of-namespace: fail
