#ifndef __EXPERIMENT_FLOW_HPP__
  #define __EXPERIMENT_FLOW_HPP__

// Author: Adrian Böckenkamp
// Date:   09.09.2011

#include "../SAL/SALInst.hpp"

namespace fi
{

/**
 * \class ExperimentFlow
 * Basic interface for user-defined experiments. To create a new
 * experiment, derive your own class from ExperimentFlow and
 * define the run method.
 */
class ExperimentFlow
{
	public:
		ExperimentFlow() { }
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
			sal::simulator.cleanup(this); // remove residual events
		}
};

}

#endif /* __EXPERIMENT_FLOW_HPP__ */
