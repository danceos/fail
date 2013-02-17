#include <iostream>
#include <sstream>

// getpid
#include <sys/types.h>
#include <unistd.h>

#include "util/Logger.hpp"

#include "experiment.hpp"
#include "campaign.hpp"

#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"

#define LOCAL 0

#define STR_STATS    "calculating stats"
#define STR_START    "starting test pass"
#define STR_TESTED   "tested "
#define STR_BADFRAME "bad frame at pfn "

using namespace std;
using namespace fail;

/*
// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif
*/

bool RAMpageExperiment::run()
{
	m_log << "startup" << std::endl;

	// not implemented yet for QEMU:
	//simulator.restore("rampage-cext-started");

	// We cannot include the auto-generated rampage.pb.h in experiment.hpp,
	// because it is included in an aspect header; therefore we need to work
	// around not having the type available there.
	m_param = new RAMpageExperimentData;
#if LOCAL
	//m_param->msg.set_mem_addr(1024*604+123);
	//m_param->msg.set_mem_addr(1024*1024*63+123);
	m_param->msg.set_mem_addr(73728);
	m_param->msg.set_mem_bit(7);
	//m_param->msg.set_errortype(m_param->msg.ERROR_STUCK_AT_0);
	m_param->msg.set_errortype(m_param->msg.ERROR_COUPLING);
	m_param->msg.set_local_timeout(1000*60*10); // 10m
	m_param->msg.set_global_timeout(1000*60*50); // 50m
#else
	if (!m_jc.getParam(*m_param)) {
		m_log << "Dying." << endl;
		// communicate that we were told to die
		simulator.terminate(1);
	}
#endif

	m_starttime = std::time(0);
	m_param->msg.set_mem_written(false);

	MemWriteListener l_mem1(m_param->msg.mem_addr());
	IOPortListener l_io(0x2f8, true); // ttyS1 aka COM2
	TimerListener l_timeout_local(m_param->msg.local_timeout());
	TimerListener l_timeout_global(m_param->msg.global_timeout());

	simulator.addListener(&l_mem1);
	simulator.addListener(&l_io);
	simulator.addListener(&l_timeout_local);
	simulator.addListener(&l_timeout_global);
	while (true) {
		BaseListener *l = simulator.resume();

		if (l == &l_mem1) {
			simulator.addListener(&l_mem1);
			handleMemWrite(l_mem1.getTriggerAddress());
			m_param->msg.set_mem_written(true);
		} else if (l == &l_io) {
			simulator.addListener(&l_io);
			if (handleIO(l_io.getData())) {
				// we saw some progress, restart local timer
				simulator.removeListener(&l_timeout_local);
				simulator.addListener(&l_timeout_local);
			}
		} else if (l == &l_timeout_local) {
			m_log << "local timeout" << std::endl;
			terminateExperiment(m_param->msg.LOCAL_TIMEOUT);
		} else if (l == &l_timeout_global) {
			m_log << "global timeout" << std::endl;
			terminateExperiment(m_param->msg.GLOBAL_TIMEOUT);
		}
	}
	return true;
}

void RAMpageExperiment::handleMemWrite(address_t addr)
{
	address_t addr1 = addr;
	unsigned bit1 = m_param->msg.mem_bit();
	unsigned bit2 = m_param->msg.mem_coupled_bit();
	unsigned char data = m_mm.getByte(addr1);

	m_log << "access to addr 0x" << std::hex << addr1 << ", FI" << endl;

	switch (m_param->msg.errortype()) {
	case RAMpageProtoMsg::ERROR_NONE:
		m_log << "no fault injected" << endl;
		break;
	case RAMpageProtoMsg::ERROR_STUCK_AT_0:
		m_log << "stuck-at-0, bit " << bit1 << endl;
		data &= ~(1 << bit1); // stuck-at-0
		break;
	case RAMpageProtoMsg::ERROR_STUCK_AT_1:
		m_log << "stuck-at-1, bit " << bit1 << endl;
		data |= 1 << bit1; // stuck-at-1
		break;
	case RAMpageProtoMsg::ERROR_COUPLING:
		m_log << "coupling, bit " << bit2 << " := bit " << bit1 << endl;
		data &= ~(1 << bit2); // coupling bit2 := bit1
		data |= ((data & (1 << bit1)) != 0) << bit2;
		break;
	case RAMpageProtoMsg::ERROR_INVERSE_COUPLING:
		m_log << "coupling, bit " << bit2 << " := !bit " << bit1 << endl;
		data &= ~(1 << bit2); // coupling bit2 := !bit1
		data |= ((data & (1 << bit1)) == 0) << bit2;
		break;
	default:
		m_log << "unknown error type" << std::endl;
	}
	m_mm.setByte(addr1, data);
/*
	unsigned char data2 = m_mm.getByte(addr2);
	data2 &= ~(1 << bit2); // coupling addr2:bit2 := !addr1:bit1
	data2 |= ((data & (1 << bit1)) == 0) << bit2;
	m_mm.setByte(addr2, data2);
*/
}

bool RAMpageExperiment::handleIO(char c)
{
	if (c != '\n') {
		m_output += c;
		return false;
	}

	// calculating stats
	if (!m_output.compare(0, sizeof(STR_STATS)-1, STR_STATS)) {
		if (m_empty_passes > 0) {
			m_log << "no PFNs were tested this time (#" << dec << m_empty_passes << ")" << std::endl;
		}
		if (m_empty_passes >= m_param->msg.empty_passes()) {
			// result: NO_PFNS_TESTED
			m_log << "giving up" << std::endl;
			terminateExperiment(m_param->msg.NO_PFNS_TESTED);
		}
		m_log << STR_STATS << std::endl;

	// starting test pass
	} else if (!m_output.compare(0, sizeof(STR_START)-1, STR_START)) {
		++m_empty_passes;
		m_log << STR_START << std::endl;

	// tested %08x-%08x %08x-%08x ...
	} else if (!m_output.compare(0, sizeof(STR_TESTED)-1, STR_TESTED)) {
		m_empty_passes = 0;
		//m_log << STR_TESTED << std::endl;

		// test whether the failing PFN was listed
		stringstream ss;
		ss << m_output.substr(sizeof(STR_TESTED) - 1);
		while (!ss.eof()) {
			char c;
			uint32_t a, b;
			ss >> hex >> a >> c >> b;
			if (ss.fail()) {
				m_param->msg.set_details("unknown serial output: " + m_output);
				terminateExperiment(m_param->msg.UNKNOWN);
			}
			if (a <= (m_param->msg.mem_addr() >> 12) &&
			    (m_param->msg.mem_addr() >> 12) <= b) {
				// we abort even if errortype == ERROR_NONE
				m_log << "PF was tested but no error was found, aborting" << endl;
				terminateExperiment(m_param->msg.PFN_WAS_LISTED);
			}
		}

	// bad frame at pfn %08x
	} else if (!m_output.compare(0, sizeof(STR_BADFRAME)-1, STR_BADFRAME)) {
		m_log << m_output << std::endl;

		// test whether it was the right PFN
		uint64_t pfn;
		stringstream ss;
		ss << m_output.substr(sizeof(STR_BADFRAME) - 1);
		ss >> hex >> pfn;
		if (ss.fail()) {
			m_param->msg.set_details("unknown serial output: " + m_output);
			terminateExperiment(m_param->msg.UNKNOWN);
		}
		m_param->msg.set_error_detected_pfn(pfn);
		if ((m_param->msg.mem_addr() >> 12) == pfn) {
			terminateExperiment(m_param->msg.RIGHT_PFN_DETECTED);
		} else {
			terminateExperiment(m_param->msg.WRONG_PFN_DETECTED);
		}

	// unknown
	} else {
		m_log << "wtf unknown: " << m_output << std::endl;
		m_param->msg.set_details("unknown serial output: " + m_output);
		terminateExperiment(m_param->msg.UNKNOWN);
	}

	m_output.clear();
	return true;
}

void RAMpageExperiment::terminateExperiment(int resulttype)
{
	m_param->msg.set_resulttype((RAMpageProtoMsg::ResultType) resulttype);
	m_param->msg.set_experiment_time(std::time(0) - m_starttime);
#if !LOCAL
	m_jc.sendResult(*m_param);
#endif
	simulator.terminate();
}
