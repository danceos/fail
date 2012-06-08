#ifndef __TRACING_TEST_HPP__
  #define __TRACING_TEST_HPP__

#include "efw/ExperimentFlow.hpp"

class TracingTest : public fail::ExperimentFlow {
public:
	bool run();
};

#endif // __TRACING_TEST_HPP__
