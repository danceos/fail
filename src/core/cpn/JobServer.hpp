#ifndef __JOB_SERVER_H__
  #define __JOB_SERVER_H__

#include "Minion.hpp"
#include "util/SynchronizedQueue.hpp"
#include "util/SynchronizedCounter.hpp"
#include "util/SynchronizedMap.hpp"
#include "config/FailConfig.hpp"
#include "comm/FailControlMessage.pb.h"
#include "comm/SocketComm.hpp"

#include <list>
#include <ctime>

#ifndef __puma
#include <boost/thread.hpp>
#endif

namespace fail {

class CommThread;

/**
 * \class JobServer
 * The server supplies the Minions with ExperimentData's and receives the result data.
 *
 * Manages the campaigns parameter distributions. The Campaign Controller can add
 * experiment parameter sets, which the Jobserver will distribute to requesting
 * clients. The campaign controller can wait for all results, or a timeout.
 */
class JobServer {
private:
	//! The TCP Port number
	int m_port;
	//! TODO nice termination concept
	bool m_finish;
	//!  Campaign signaled last expirement data set
	bool m_noMoreExps;
	//! the maximal number of threads spawned for TCP communication
	unsigned m_maxThreads;
	//! the maximal timeout per communication thread
	int m_threadtimeout;
	//! list of spawned threads
#ifndef __puma
	typedef std::list<boost::thread*> Tthreadlist;
	Tthreadlist m_threadlist;

	boost::thread* m_serverThread;
#endif // puma

	//! unique server run ID
	uint64_t m_runid;

#ifdef SERVER_PERFORMANCE_MEASURE
	static volatile unsigned m_DoneCount; //! the number of finished jobs
#ifndef __puma
	boost::thread* m_measureThread; //! the performance measurement thread
#endif
#endif
	SynchronizedCounter m_inOutCounter;
	//! Atomic counter for Workload IDs.
	SynchronizedCounter m_counter;
	//! Map of running jobs (referenced by Workload ID
	SynchronizedMap<uint32_t, ExperimentData*> m_runningJobs;
	//! List of undone jobs, here the campaigns jobs enter
	SynchronizedQueue<ExperimentData*> m_undoneJobs;
	//! List of finished experiment results.
	SynchronizedQueue<ExperimentData*> m_doneJobs;
	friend class CommThread; //!< CommThread is allowed access the job queues.
	/**
	 * The actual startup of the Jobserver.
	 * Here we initalize the network socket
	 * and listen for connections.
	 */
	void run();
#ifdef SERVER_PERFORMANCE_MEASURE
	void measure();
#endif
	void sendWork(int sockfd);

public:
	JobServer(int port = SERVER_COMM_TCP_PORT) : m_port(port), m_finish(false), m_noMoreExps(false),
		m_maxThreads(128), m_threadtimeout(0), m_undoneJobs(SERVER_OUT_QUEUE_SIZE)
	{
		SocketComm::init();
		m_runid = std::time(0);
#ifndef __puma
		m_serverThread = new boost::thread(&JobServer::run, this); // run operator()() in a thread.
#ifdef SERVER_PERFORMANCE_MEASURE
		m_measureThread = new boost::thread(&JobServer::measure, this);
#endif
#endif
	}
	~JobServer()
	{
#ifndef __puma
		// Cleanup of m_serverThread, etc.
		delete m_serverThread;
#ifdef SERVER_PERFORMANCE_MEASURE
		delete m_measureThread;
#endif
#endif // __puma
	}
	/**
	 * Adds a new experiment data set to the job queue.
	 * @param data Pointer to an expoeriment data object
     */
	void addParam(ExperimentData* data);
	/**
	 * Retrieve an experiment result. Blocks if we currently have no results.
	 * Returns \c NULL if no results are to be expected, because no parameter
	 * sets were enqueued beforehand.
	 * @return pointer to experiment result data
	 */
	ExperimentData* getDone();
	/**
	 * The Campaign controller must signal that there will be no more parameter
	 * sets.  We need this, as we allow concurrent parameter generation and
	 * distribution.
	 */
	void setNoMoreExperiments() { m_noMoreExps = true; }
	/**
	 * Checks whether there are no more experiment parameter sets.
	 * @return \c true if no more parameter sets available, \c false otherwise
	 * @see setNoMoreExperiments
	 */
	bool noMoreExperiments() const { return m_noMoreExps; }

	/**
	 * The Campaign Controller may signal that the jobserver can stop listening
	 * for client connections.
	 */
	void done() { m_finish = true; }
};

/**
 * @class CommThread
 * Implementation of the communication threads.
 * This class implements the actual communication
 * with the minions.
 */
class CommThread {
private:
	int m_sock; //! Socket descriptor of the connection
	uint32_t m_job_size;
	JobServer& m_js; //! Calling jobserver

	// FIXME: Concerns are not really separated yet ;)
	/**
	 * Called after minion calls for work.
	 * Tries to deque a parameter set non blocking, and
	 * sends it back to the requesting minion.
	 * @param minion The minion asking for input
	 */
	void sendPendingExperimentData(Minion& minion);
	/**
	 * Called after minion offers a result message.
	 * Evaluates the Workload ID and puts the corresponding
	 * job result into the result queue.
	 * @param minion The minion offering results
	 * @param workloadID The workload id of the result message
	 */
	void receiveExperimentResults(Minion& minion, FailControlMessage& ctrlmsg);
public:
#ifndef __puma
	static boost::mutex m_CommMutex; //! to synchronise the communication
#endif // __puma
	CommThread(int sockfd, JobServer& p)
		: m_sock(sockfd), m_job_size(1), m_js(p) { }
	/**
	 * The thread's entry point.
	 */
	void operator()();
};

} // end-of-namespace: fail

#endif //__JOB_SERVER_H__
