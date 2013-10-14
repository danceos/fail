#ifndef __EXPERIMENT_FLOW_HPP__
#define __EXPERIMENT_FLOW_HPP__

#include "sal/SALInst.hpp"

namespace fail {

/**
 * \class ExperimentFlow
 * Basic interface for user-defined experiments. To create a new experiment,
 * derive your own class from ExperimentFlow and define the run method.
 */
class ExperimentFlow {
public:
	ExperimentFlow() { }
	virtual ~ExperimentFlow()
	{
		simulator.clearListeners(this); // remove residual events
		simulator.removeFlow(this);
	}
	/**
	 * Defines the experiment flow.
	 * @return \c true if the experiment was successful, \c false otherwise
	 */
	virtual bool run() = 0;
	/**
	 * The entry point for this experiment's coroutine.
	 * Should do some cleanup afterwards.
	 */
	void coroutine_entry()
	{
		run();
		simulator.clearListeners(this); // remove residual events
	}
};

} // end-of-namespace: fail

#endif // __EXPERIMENT_FLOW_HPP__
