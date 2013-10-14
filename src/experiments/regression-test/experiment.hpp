#ifndef __REGRESSION_TEST_HPP__
  #define __REGRESSION_TEST_HPP__

#include "efw/ExperimentFlow.hpp"

class RegressionTest : public fail::ExperimentFlow
{
	public:
		RegressionTest() { }

		bool run();
};

#endif // __REGRESSION_TEST_HPP__
