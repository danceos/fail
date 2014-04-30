#ifndef __FESAMPLING_PRUNER_H__
#define __FESAMPLING_PRUNER_H__

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

	unsigned m_samplesize;

public:
	FESamplingPruner() : m_samplesize(0) { }
	virtual std::string method_name() { return "FESampling"; }
	virtual bool commandline_init();
	virtual bool prune_all();

private:
	bool sampling_prune(const fail::Database::Variant& variant);
};

#endif
