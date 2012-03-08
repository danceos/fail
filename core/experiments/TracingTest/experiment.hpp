#ifndef __TRACING_TEST_HPP__
  #define __TRACING_TEST_HPP__

#include "controller/ExperimentFlow.hpp"

class TracingTest : public fi::ExperimentFlow
{
public:
	bool run();
};

#endif /* __TRACING_TEST_HPP__ */
