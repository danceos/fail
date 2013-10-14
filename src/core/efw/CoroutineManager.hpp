#ifndef __COROUTINE_MANAGER_HPP__
#define __COROUTINE_MANAGER_HPP__

#include <map>
#include <stack>

#include <pcl.h> // the underlying "portable coroutine library"

namespace fail {

class ExperimentFlow;

/**
 * \class CoroutineManager
 * Manages the experiments flow encapsulated in coroutines.
 */
class CoroutineManager {
private:
	//! the default stack size for coroutines (= experiment flows)
	static const unsigned STACK_SIZE_DEFAULT = 4096*4096;
	//! the abstraction for coroutine identification
	typedef coroutine_t corohandle_t;
	typedef std::map<ExperimentFlow*,corohandle_t> flowmap_t;
	//! the mapping "flows <-> coro-handle"
	flowmap_t m_Flows;
	//! the simulator/backend coroutine handle
	corohandle_t m_simCoro;
	//! stack of coroutines that explicitly activated another one with toggle()
	std::stack<corohandle_t> m_togglerstack;
	//! manages the run-calls for each ExperimentFlow-object
	static void m_invoke(void* pData);
	//! \c true if terminated explicitly using simulator.terminate()
	bool m_Terminated;
public:
	static const ExperimentFlow* SIM_FLOW; //!< the simulator coroutine flow

	CoroutineManager() : m_simCoro(co_current()), m_Terminated(false) { }
	~CoroutineManager();
	/**
	 * Creates a new coroutine for the specified experiment flow.
	 * @param flow the flow to be executed in the newly created coroutine
	 */
	void create(ExperimentFlow* flow);
	/**
	 * Destroys coroutine for the specified experiment flow.
	 * @param flow the flow to be removed
	 */
	void remove(ExperimentFlow* flow);
	/**
	 * Switches the control flow to the experiment \a flow. If \a flow is
	 * equal to \c SIM_FLOW, the control will be handed back to the
	 * simulator.  The current control flow is pushed onto an
	 * internal stack.
	 * @param flow the destination control flow or \c SIM_FLOW (= \c NULL )
	 */
	void toggle(ExperimentFlow* flow);
	/**
	 * Gives the control back to the coroutine that toggle()d the
	 * current one, by drawing from the internal stack built from
	 * calls to toggle().
	 */
	void resume();
	/**
	 * Retrieves the current (active) coroutine (= flow).
	 * @return the current experiment flow.
	 */
	ExperimentFlow* getCurrent();
	/**
	 * Sets the termination flag. This should be called when Fail
	 * exists due to a call to \c ::exit() (used, e.g., in
	 * \c SimulatorController::terminate()). This cannot be undone.
	 */
	 void setTerminated() { m_Terminated = true; }
};

} // end-of-namespace: fail

#endif // __COROUTINE_MANAGER_HPP__
