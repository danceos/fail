#ifndef __JOB_SERVER_H__
#define __JOB_SERVER_H__

#include "util/SynchronizedQueue.hpp"
#include "util/SynchronizedCounter.hpp"
#include "util/SynchronizedMap.hpp"
#include "config/FailConfig.hpp"
#include "comm/ExperimentData.hpp"
#include "comm/FailControlMessage.pb.h"

#include <list>
#include <ctime>
#include <memory>
#include <string>

#ifndef __puma
#include <boost/thread.hpp>
#endif
#include <boost/optional.hpp>

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
	struct impl;
	std::shared_ptr<impl> m_d;

	//! The TCP Port number
	const unsigned short m_port;
	//! TODO nice termination concept
	bool m_finish;
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

	volatile uint64_t m_DoneCount = 0; //! the number of finished jobs
	boost::optional<uint64_t> m_TotalCount; //! the total number of jobs to be expected
#ifdef SERVER_PERFORMANCE_MEASURE
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
#ifndef __puma
	boost::mutex m_CommMutex; //! to synchronise the communication
#endif // __puma
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
  JobServer(const unsigned short port = SERVER_COMM_TCP_PORT);
	~JobServer()
	{
		done();
#ifndef __puma
		// Cleanup of m_serverThread, etc.
		m_serverThread->join();
		delete m_serverThread;
#ifdef SERVER_PERFORMANCE_MEASURE
		m_measureThread->join();
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
	void setNoMoreExperiments();
	/**
	 * Can optionally be used to tell the JobServer how many jobs to expect in
	 * total.  This count is used for progress reporting.  Make sure you also
	 * call skipJobs() if some of these early-on announced jobs will not be
	 * sent after all (e.g. because the campaign already found results for them
	 * in the database).
	 */
	void setTotalCount(uint64_t count) { m_TotalCount = count; }
	void skipJobs(uint64_t count) { ++m_DoneCount; /* FIXME assume atomic */ }
	/**
	 * Checks whether there are no more experiment parameter sets.
	 * @return \c true if no more parameter sets available, \c false otherwise
	 * @see setNoMoreExperiments
	 */
	bool noMoreExperiments() const;

	/**
	 * The Campaign Controller may signal that the jobserver can stop listening
	 * for client connections.
	 */
	void done();
};

} // end-of-namespace: fail

#endif //__JOB_SERVER_H__
