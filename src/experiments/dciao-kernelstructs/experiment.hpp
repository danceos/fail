#ifndef __DCIAO_KERNEL_EXPERIMENT_HPP__
#define __DCIAO_KERNEL_EXPERIMENT_HPP__


#include "sal/SALInst.hpp"
#include "efw/ExperimentFlow.hpp"
#include "efw/JobClient.hpp"
#include "util/Logger.hpp"
#include "util/ElfReader.hpp"
#include <vector>
#include <string>

class DCIAOKernelStructs : public fail::ExperimentFlow {
public:
	/** timer_markers indicate the activation flow inside the
		coper_mock application. For a correct run the activation flow
		is deterministic. Therefore we can say wheter a the
		application had a correct control flow */
	struct time_marker {
		uint32_t time;
		uint32_t at;
	};

	typedef std::vector<time_marker> time_markers_t;

private:
	fail::JobClient m_jc;
	fail::Logger m_log;
	fail::MemoryManager& m_mm;
	fail::ElfReader m_elf;

	unsigned injectBitFlip(fail::address_t data_address, unsigned bitpos);
	time_markers_t getTimeMarkerList();
	static int time_markers_compare(const time_markers_t &, const time_markers_t &);


public:
	DCIAOKernelStructs() : m_log("DCIAOKernelStructs", false), m_mm(fail::simulator.getMemoryManager()) {}

	bool run();
};

#endif // __KESO_REFS_EXPERIMENT_HPP__
