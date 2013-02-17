#ifndef __DATA_RETRIEVAL_EXPERIMENT_HPP__
  #define __DATA_RETRIEVAL_EXPERIMENT_HPP__

#include "../controller/ExperimentFlow.hpp"

class DataRetrievalExperiment : public fail::ExperimentFlow
{
	public:
		DataRetrievalExperiment() { }

		bool run();
};

#endif // __DATA_RETRIEVAL_EXPERIMENT_HPP__
