#ifndef __SAMPLING_PRUNER_H__
#define __SAMPLING_PRUNER_H__

#include <stdint.h>
#include "Pruner.hpp"
#include "util/CommandLine.hpp"

///
/// SamplingPruner: implements sampling with equivalence-class reuse
///
/// Unlike the FESamplingPruner, the SamplingPruner implements uniform
/// fault-space sampling that counts multiple hits of an equivalence class.
///
class SamplingPruner : public Pruner {
	fail::CommandLine::option_handle SAMPLESIZE;
	fail::CommandLine::option_handle USE_KNOWN_RESULTS;
	fail::CommandLine::option_handle NO_WEIGHTING;

	uint64_t m_samplesize;
	bool m_use_known_results, m_weighting, m_incremental;

public:
	SamplingPruner() : m_samplesize(0), m_use_known_results(false), m_weighting(true), m_incremental(false) { }
	virtual std::string method_name() { return "sampling"; }
	virtual bool commandline_init();
	virtual bool prune_all();

	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("SamplingPruner");
		aliases->push_back("sampling");
	}

	virtual bool set_incremental(bool incremental) { m_incremental = incremental; return true; }

private:
	bool sampling_prune(const fail::Database::Variant& variant);
};

#endif
