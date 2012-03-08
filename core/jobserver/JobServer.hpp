/**
 * \brief The JobServer supplies the Minions with ExperimentData's and receives the result data.
 *
 * \author Martin Hoffmann, Richard Hellwig
 */


#ifndef __JOB_SERVER_H__
#define __JOB_SERVER_H__

#include "controller/Minion.hpp"
#include "util/SynchronizedQueue.hpp"
#include "util/SynchronizedCounter.hpp"
#include "util/SynchronizedMap.hpp"

#include <list>
#ifndef __puma
#include <boost/thread.hpp>
#endif

namespace fi {

class CommThread;
  
/**
 * \class JobServer
 * Manages the campaigns parameter distributions. 
 * The Campaign Controller can add experiment parameter sets,
 * which the Jobserver will distribute to requesting clients.
 * The campaign controller can wait for all results, or a timeout.
 */
class JobServer 
{
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
	//! A of spawned threads
#ifndef __puma
	typedef std::list<boost::thread*> Tthreadlist;
	Tthreadlist m_threadlist;
	
	boost::thread* m_serverThread;
#endif // puma
	
	//! Atomic counter for Workload IDs.
	SynchronizedCounter m_counter;
	//! Map of running jobs (referenced by Workload ID
	SynchronizedMap<uint32_t, ExperimentData*> m_runningJobs;
	//! List of undone jobs, here the campaigns jobs enter
	SynchronizedQueue<ExperimentData*> m_undoneJobs;
	//! List of finished experiment results.
	SynchronizedQueue<ExperimentData*> m_doneJobs;
	friend class CommThread; //!< CommThread is allowed access the job queues.
	
public:	
	JobServer(int port = 1111) : m_port(port), m_finish(false), m_noMoreExps(false), m_maxThreads(32), m_threadtimeout(500) { 
#ifndef __puma
	  m_serverThread = new boost::thread(&JobServer::run, this); // run operator()() in a thread. 
#endif
	};
	~JobServer() {}

	private:  
		
	/**
	 * The actual startup of the Jobserver.
	 * Here we initalize the network socket
	 * and listen for connections.
	 */
	void run();
	
	void sendWork(int sockfd);

public:
	  
	/**
	 * Adds a new experiment data set to the job queue.
	 * @param data Pointer to an expoeriment data object
         */
	void addParam(ExperimentData* data);

	/**
	 * Retrieve an experiment result.
	 *
	 * Blocks if we currently have no results.  Returns NULL if no results
	 * are to be expected, because no parameter sets were enqueued
	 * beforehand.
	 */
	ExperimentData* getDone();
	
	/**
	 * The Campaign controller must signalize, that 
	 * there will be no more parameter sets.
	 * We need this, as we allow concurrent parameter
	 * generation and distribution.
	 */
	void setNoMoreExperiments(){ m_noMoreExps = true; };

	/**
	 * corresponding getter-method @see setNoMoreExperiments
	 */
	bool noMoreExperiments()const { return m_noMoreExps; };
	
	/**
	 * The Campaign Controller can signalize, that 
	 * the jobserver can stop listening for client
	 * connections.
	 */
	void done() { m_finish = true; };
};


/**
 * @class CommThread
 * Implementation of the communication threads.
 * This class implements the actual communication 
 * with the minions.
 */
class CommThread {
	int m_sock; //! Socket descriptor of the connection
	JobServer& m_js; //! Calling jobserver
public:
	CommThread(int sockfd, JobServer& p) : m_sock(sockfd), m_js(p) {};
	/**
	 * The thread's entry point
	 */
	void operator() ();
private:
  /// FIXME concerns are not really separated yet ;)
	/**
	 * Called after minion calls for work.
	 * Tries to deque a parameter set non blocking, and
	 * sends it back to the requesting minion.
	 * @param minion The minion asking for input
	 * @return FIXME return value not evaluated yet.
	 */
	bool sendPendingExperimentData(Minion& minion);

	/**
	 * Called after minion offers a result message.
	 * Evaluates the Workload ID and puts the corresponding
	 * job result into the result queue. 
	 * @param minion The minion offering results
	 * @param workloadID The workload id of the result message
	 * @return \c true if Worload ID could be mapped, \c false if not
	 */	
	bool receiveExperimentResults(Minion& minion, uint32_t workloadID);
};

};

#endif //__JOB_SERVER_H__
