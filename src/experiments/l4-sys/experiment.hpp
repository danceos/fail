#ifndef __L4SYS_EXPERIMENT_HPP__
  #define __L4SYS_EXPERIMENT_HPP__

#include <string>

#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"

class L4SysExperiment : public fail::ExperimentFlow {
	fail::JobClient m_jc;
public:
	L4SysExperiment() : m_jc("localhost") {}
	bool run();
private:
	// NOTE: It's good practise to use "const std::string&" as parameter type.
	//       Additionaly, if you don't need the return value to be copied,
	//       return a (const) reference to a class member or a static string-
	//       object.
	std::string sanitised(std::string in_str);
	fail::BaseEvent* waitIOOrOther(bool clear_output);
};

#endif // __L4SYS_EXPERIMENT_HPP__
