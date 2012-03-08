#ifndef __SIGNAL_HPP__
  #define __SIGNAL_HPP__

// Author: Adrian Böckenkamp
// Date:   15.06.2011

#include <cassert>
#include <memory>
#include <iostream>
#ifndef __puma
#include <boost/thread.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#endif

namespace fi
{

#ifndef __puma
typedef boost::mutex Mutex;                          // lock/unlock
typedef boost::mutex::scoped_lock ScopeLock;         // use RAII with lock/unlock mechanism
typedef boost::condition_variable ConditionVariable; // wait/notify_one
#else
typedef int Mutex;
typedef int ScopeLock;
typedef int ConditionVariable;
#endif

// Simulate a "private" semaphore using boost-mechanisms:
class Semaphore
{
	private:
		Mutex m_Mutex;
		ConditionVariable m_CondVar;
		unsigned long m_Value;
	public:
		// Create a semaphore object based on a mutex and a condition variable
		// and initialize it to value "init".
		Semaphore(unsigned long init = 0) : m_Value(init) { }

		void post()
		{
			ScopeLock lock(m_Mutex);
			++m_Value; // increase semaphore value:
#ifndef __puma
			m_CondVar.notify_one(); // wake up other thread, currently waiting on condition var.
#endif
		}

		void wait()
		{
			ScopeLock lock(m_Mutex);
#ifndef __puma
			while(!m_Value) // "wait-if-zero"
				m_CondVar.wait(lock);
#endif
			--m_Value; // decrease semaphore value
		}
};

class Signal
{
	private:
		static Mutex m_InstanceMutex; // used to sync calls to getInst()
		static std::auto_ptr<Signal> m_This; // the one and only instance

		Semaphore m_semBochs;
		Semaphore m_semContr;
		Semaphore m_semSimCtrl;
		bool m_Locked; // prevent misuse of thread-sync

		// Singleton class (forbid creation, copying and assignment):
		Signal()
			: m_semBochs(0), m_semContr(0),
			  m_semSimCtrl(0), m_Locked(false) { }
		Signal(Signal const& s)
			: m_semBochs(), m_semContr(),
			  m_semSimCtrl(), m_Locked(false) { } // never called.
		Signal& operator=(Signal const&) { return *this; } // dito.
		~Signal() { }
		friend class std::auto_ptr<Signal>;
	public:
		static Signal& getInst()
		{
			ScopeLock lock(m_InstanceMutex); // lock/unlock handled by RAII principle
			if(!m_This.get())
				m_This.reset(new Signal());
			return (*m_This);
		}

		// Called from Experiment-Controller class ("beyond Bochs"):
		void lockExperiment()
		{
			assert(!m_Locked &&
				"[Signal::lockExperiment]: lockExperiment called twice without calling unlockExperiment() in between.");
			m_Locked = true;
			m_semContr.wait(); // suspend experiment process
		}

		// Called from Experiment-Controller class ("beyond Bochs"):
		void unlockExperiment()
		{
			assert(m_Locked &&
				"[Signal::unlockExperiment]: unlockExperiment called twice without calling lockExperiment() in between.");
			m_Locked = false;
			m_semBochs.post(); // resume experiment (continue bochs simulation)
		}
	
		// Called from Advice-Code ("within Bochs") to trigger event occurrence:
		void signalEvent()
		{
			m_semContr.post(); // Signal event (to Experiment-Controller)
			m_semBochs.wait(); // Wait upon handling to finish
		}

		// Called from Experiment-Controller to allow simulation start:
		void startSimulation()
		{
			m_semSimCtrl.post();
		}
		
		// Called from Bochs, directly after thread creation for Experiment-Controller:
		// (This ensures that Bochs waits until the experiment has been set up in the
		//  Experiment-Controller.)
		void waitForStartup()
		{
			m_semSimCtrl.wait();
		}
};

} // end-of-namespace: fi

#endif /* __SIGNAL_HPP__ */

