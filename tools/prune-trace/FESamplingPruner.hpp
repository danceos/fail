#ifndef __FESAMPLING_PRUNER_H__
#define __FESAMPLING_PRUNER_H__

#include <stdint.h>
#include "Pruner.hpp"
#include "util/CommandLine.hpp"

///
/// FESamplingPruner: implements sampling with Fault Expansion
///
/// The FESamplingPruner implements the fault-expansion variance reduction
/// technique (FE-VRT) as described in: Smith, D. Todd and Johnson, Barry W.
/// and Andrianos, Nikos and Profeta, III, Joseph A., "A variance-reduction
/// technique via fault-expansion for fault-coverage estimation" (1997),
/// 366--374.
///
class FESamplingPruner : public Pruner {
	fail::CommandLine::option_handle SAMPLESIZE;
	fail::CommandLine::option_handle USE_KNOWN_RESULTS;
	fail::CommandLine::option_handle NO_WEIGHTING;

	uint64_t m_samplesize;
	bool m_use_known_results, m_weighting;

public:
	FESamplingPruner() : m_samplesize(0), m_use_known_results(false), m_weighting(true) { }
	virtual std::string method_name() { return "FESampling"; }
	virtual bool commandline_init();
	virtual bool prune_all();

	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("FESamplingPruner");
	}

private:
	bool sampling_prune(const fail::Database::Variant& variant);
};

#endif
