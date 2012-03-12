// Author: 	Martin Hoffmann, Richard Hellwig
// Date:   	07.10.11

// <iostream> needs to be included before *.pb.h, otherwise ac++/Puma chokes on the latter
#include <iostream>

#include "JobServer.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>

#include "jobserver/messagedefs/FailControlMessage.pb.h"
#include "SocketComm.hpp"
#include "controller/Minion.hpp"
#ifndef __puma
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#endif

using namespace std;

namespace fi {


void JobServer::addParam(ExperimentData* exp){
#ifndef __puma
	m_undoneJobs.Enqueue(exp);
#endif
}

ExperimentData *JobServer::getDone()
{
	// FIXME race condition, need to synchronize with
	// sendPendingExperimentData() and receiveExperimentResults()
#ifndef __puma
	if (m_undoneJobs.Size() == 0
	 && noMoreExperiments()
	 && m_runningJobs.Size() == 0
	 && m_doneJobs.Size() == 0) {
		// FRICKEL workaround
		sleep(1);
		ExperimentData *exp;
		if (m_doneJobs.Dequeue_nb(exp)) {
			return exp;
		}
		return 0;
	}
	return m_doneJobs.Dequeue();
#endif
}

#ifndef __puma
/**
 * This is a predicate class for the remove_if operator
 * on the thread list. The operator waits for
 * timeout milliseconds to join each thread in the list.
 * If the join was successful, the exited thread is deallocated
 * and removed from the list.
 */
struct timed_join_successful {
 int timeout_ms;
 timed_join_successful(int timeout) : timeout_ms(timeout){};
 
 bool operator()( boost::thread * threadelement ){
	boost::posix_time::time_duration timeout = boost::posix_time::milliseconds(timeout_ms);
	if(threadelement->timed_join(timeout)){
		delete threadelement;
		return true;
	}else{
		return false;
	}
	
}
};
#endif

void JobServer::run(){
	struct sockaddr_in clientaddr;
	socklen_t clen = sizeof(clientaddr);
	
	// implementation of server-client communication
	int s;
	if((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return;
	}

	/* Enable address reuse */
	int on = 1;
	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		perror("setsockopt");
		return;
	}
	
	/* IPv4, Port: 1111, accept all  IP adresses  */
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(m_port);
	saddr.sin_addr.s_addr = htons(INADDR_ANY);
 
	/* bind to port */
	if(bind(s, (struct sockaddr*) &saddr, sizeof(saddr)) == -1) {
		perror("bind");
		return;
	}
 
	/* Listen with a backlog of maxThreads */
	if(listen(s, m_maxThreads) == -1) {
		perror("listen");
		return;
	}
	cout << "JobServer listening...." << endl;
#ifndef __puma
	boost::thread* th;
	while(!m_finish){
		// Accept connection 
		int cs = accept(s, (struct sockaddr*)&clientaddr, &clen);
		if(cs == -1) {
			perror("accept");
			return;
		}
		// Spawn a thread for further communication,
		// and add this thread to a list threads
		// We can limit the generation of threads here.
		if(m_threadlist.size() < m_maxThreads){
			th = new boost::thread(CommThread(cs, *this));
			m_threadlist.push_back(th);
		}else{
			// Run over list with a timed_join,
			// removing finished threads.
			do {
				m_threadlist.remove_if( timed_join_successful(m_threadtimeout) );
			} while(m_threadlist.size() == m_maxThreads);
			// Start new thread	
			th = new boost::thread(CommThread(cs, *this));
			m_threadlist.push_back(th);
		}
		
	}
	close(s);
	// when all undone Jobs are distributed  -> call a timed_join on all spawned
	// TODO: interrupt threads that do not want to join..
	while(m_threadlist.size() > 0){
		m_threadlist.remove_if( timed_join_successful(m_threadtimeout) );
	}
#endif
}

/// Communication thread implementation
void CommThread::operator()()
{
	Minion minion;
	FailControlMessage ctrlmsg;
	minion.setSocketDescriptor(m_sock);

	if (!SocketComm::rcv_msg(minion.getSocketDescriptor(), ctrlmsg)) {
		cout << "!![Server] failed to read complete message from client" << endl;
		close(m_sock);
		return;
	}

	switch (ctrlmsg.command()) {
	case FailControlMessage_Command_NEED_WORK:
		// give minion something to do..
		sendPendingExperimentData(minion);
		break;
	case FailControlMessage_Command_RESULT_FOLLOWS:
		// get results and put to done queue.
		receiveExperimentResults(minion, ctrlmsg.workloadid());
		break;
	default:
		// hm.. don't know what to do. please die.
		cout << "!![Server] no idea what to do with command #"
		     << ctrlmsg.command() << ", telling minion to die." << endl;
		ctrlmsg.Clear();
		ctrlmsg.set_command(FailControlMessage_Command_DIE);
		ctrlmsg.set_build_id(42);
		SocketComm::send_msg(minion.getSocketDescriptor(), ctrlmsg);
	}

	close(m_sock);
}

bool CommThread::sendPendingExperimentData(Minion& minion)
{
			FailControlMessage ctrlmsg;
			ctrlmsg.set_build_id(42);
			ExperimentData * exp = 0;
			if(m_js.m_undoneJobs.Dequeue_nb(exp) == true){ 
			// Got an element from queue, assign ID to workload and send to minion
				uint32_t workloadID = m_js.m_counter.increment(); // increment workload counter
				exp->setWorkloadID(workloadID);			  // store ID for identification when receiving result
				if(!m_js.m_runningJobs.insert(workloadID, exp)){
					cout << "!![Server]could not insert workload id: [" << workloadID << "] double entry?" << endl;
				}
				ctrlmsg.set_command(FailControlMessage_Command_WORK_FOLLOWS);
				ctrlmsg.set_workloadid(workloadID); // set workload id
				//cout << ">>[Server] Sending workload [" << workloadID << "]" << endl;
				cout << ">>[" << workloadID << "] " << flush;
				SocketComm::send_msg(minion.getSocketDescriptor(), ctrlmsg);
				SocketComm::send_msg(minion.getSocketDescriptor(), exp->getMessage());
			}else if( m_js.noMoreExperiments() == false ){ 
			// Currently we have no workload, but the campaign is not over yet. Minion can try again later
				ctrlmsg.set_command(FailControlMessage_Command_COME_AGAIN);
				SocketComm::send_msg(minion.getSocketDescriptor(), ctrlmsg);
				cout << "--[Server] No workload, come again..."  <<  endl;
			}else{
			// No more elements, and campaign is over. Minion can die.
				ctrlmsg.set_command(FailControlMessage_Command_DIE);
				cout << "--[Server] No workload, and no campaign, please die." << endl;
				SocketComm::send_msg(minion.getSocketDescriptor(), ctrlmsg);
			}
			return true;
}


bool CommThread::receiveExperimentResults(Minion& minion, uint32_t workloadID)
{
		ExperimentData * exp; // Get exp* from running jobs
		//cout << "<<[Server] Received result for workload id [" << workloadID << "]" << endl;
		cout << "<<[" << workloadID << "] " << flush;
		if( m_js.m_runningJobs.remove(workloadID, exp) ){ /// ExperimentData* found
			SocketComm::rcv_msg(minion.getSocketDescriptor(), exp->getMessage() ); /// deserialize results.
			m_js.m_doneJobs.Enqueue(exp); /// Put results in done queue..
			return true;
		}else{
			cout << "!![Server] workload id not found in running jobs map :( [" << workloadID << "]" << endl;
			return false;
		}
}

};
