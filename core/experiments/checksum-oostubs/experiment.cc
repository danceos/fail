#include <iostream>

#include "util/Logger.hpp"

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"

#include "SAL/SALConfig.hpp"
#include "SAL/SALInst.hpp"
#include "SAL/Memory.hpp"
#include "SAL/bochs/BochsRegister.hpp"
#include "controller/Event.hpp"

// you need to have the tracing plugin enabled for this
#include "plugins/tracing/TracingPlugin.hpp"

#include "checksum-oostubs.pb.h"
#include "ecc_region.hpp"

using std::endl;

bool CoolChecksumExperiment::run()
{
	Logger log("Checksum-OOStuBS", false);
	fi::BPEvent bp;
	
	log << "startup" << endl;

#if 1
	// STEP 0: record memory map with addresses of "interesting" objects
	fi::GuestEvent g;
	while (true) {
		sal::simulator.addEventAndWait(&g);
		std::cout << g.getData() << std::flush;
	}
#elif 0
	// STEP 1: run until interesting function starts, and save state
	bp.setWatchInstructionPointer(OOSTUBS_FUNC_ENTRY);
	sal::simulator.addEventAndWait(&bp);
	log << "test function entry reached, saving state" << endl;
	log << "EIP = " << std::hex << bp.getTriggerInstructionPointer() << " or " << sal::simulator.getRegisterManager().getInstructionPointer() << endl;
	log << "error_corrected = " << std::dec << ((int)sal::simulator.getMemoryManager().getByte(OOSTUBS_ERROR_CORRECTED)) << endl;
	sal::simulator.save("checksum-oostubs.state");
#elif 1
	// STEP 2: record trace for fault-space pruning
	log << "restoring state" << endl;
	sal::simulator.restore("checksum-oostubs.state");
	log << "EIP = " << std::hex << sal::simulator.getRegisterManager().getInstructionPointer() << endl;

	log << "enabling tracing" << endl;
	TracingPlugin tp;

	// restrict memory access logging to injection target
	MemoryMap mm;
	for (unsigned i = 0; i < sizeof(memoryMap)/sizeof(*memoryMap); ++i) {
		mm.add(memoryMap[i][0], memoryMap[i][1]);
	}
	tp.restrictMemoryAddresses(&mm);

	// record trace
	Trace trace;
	tp.setTraceMessage(&trace);

	// this must be done *after* configuring the plugin:
	sal::simulator.addFlow(&tp);

	bp.setWatchInstructionPointer(fi::ANY_ADDR);
	bp.setCounter(OOSTUBS_NUMINSTR);
	sal::simulator.addEventAndWait(&bp);
	log << "experiment finished after " << std::dec << OOSTUBS_NUMINSTR << " instructions" << endl;
	
	uint32_t results[OOSTUBS_RESULTS_BYTES / sizeof(uint32_t)];
	sal::simulator.getMemoryManager().getBytes(OOSTUBS_RESULTS_ADDR, sizeof(results), results);
	for (unsigned i = 0; i < sizeof(results) / sizeof(*results); ++i) {
		log << "results[" << i << "]: " << std::dec << results[i] << endl;
	}

	sal::simulator.removeFlow(&tp);

	// serialize trace to file
	char const *tracefile = "trace.pb";
	std::ofstream of(tracefile);
	if (of.fail()) {
		log << "failed to write " << tracefile << endl;
		return false;
	}
	trace.SerializeToOstream(&of);
	of.close();
	log << "trace written to " << tracefile << endl;
	
#elif 1
	// FIXME consider moving experiment repetition into Fail* or even the
	// SAL -- whether and how this is possible with the chosen backend is
	// backend specific
	for (int i = 0; i < 20; ++i) {

	// STEP 3: The actual experiment.
	log << "restoring state" << endl;
	sal::simulator.restore("coolecc.state");

	log << "asking job server for experiment parameters" << endl;
	CoolChecksumExperimentData param;
	if (!m_jc.getParam(param)) {
		log << "Dying." << endl;
		// communicate that we were told to die
		sal::simulator.terminate(1);
	}
	int id = param.getWorkloadID();
	int instr_offset = param.msg.instr_offset();
	int bit_offset = param.msg.bit_offset();
	log << "job " << id << " instr " << instr_offset << " bit " << bit_offset << endl;

	// FIXME could be improved (especially for backends supporting
	// breakpoints natively) by utilizing a previously recorded instruction
	// trace
	bp.setWatchInstructionPointer(fi::ANY_ADDR);
	for (int count = 0; count < instr_offset; ++count) {
		sal::simulator.addEventAndWait(&bp);
	}

	// inject
	sal::guest_address_t inject_addr = COOL_ECC_OBJUNDERTEST + bit_offset / 8;
	sal::MemoryManager& mm = sal::simulator.getMemoryManager();
	sal::byte_t data = mm.getByte(inject_addr);
	sal::byte_t newdata = data ^ (1 << (bit_offset % 8));
	mm.setByte(inject_addr, newdata);
	// note at what IP we did it
	int32_t injection_ip = sal::simulator.getRegisterManager().getInstructionPointer();
	param.msg.set_injection_ip(injection_ip);
	log << "inject @ ip " << injection_ip
	    << " offset " << std::dec << (bit_offset / 8)
	    << " (bit " << (bit_offset % 8) << ") 0x"
	    << std::hex << ((int)data) << " -> 0x" << ((int)newdata) << endl;

	// aftermath
	fi::BPEvent ev_done(COOL_ECC_CALCDONE);
	sal::simulator.addEvent(&ev_done);
	fi::BPEvent ev_timeout(fi::ANY_ADDR);
	ev_timeout.setCounter(COOL_ECC_NUMINSTR + 3000);
	sal::simulator.addEvent(&ev_timeout);
	fi::TrapEvent ev_trap(fi::ANY_TRAP);
	sal::simulator.addEvent(&ev_trap);

	fi::BaseEvent* ev = sal::simulator.waitAny();
	if (ev == &ev_done) {
		int32_t data = sal::simulator.getRegisterManager().getRegister(targetreg)->getData();
		log << std::dec << "Result EDX = " << data << endl;
		param.msg.set_resulttype(CoolChecksumProtoMsg_ResultType_CALCDONE);
		param.msg.set_resultdata(data);
	} else if (ev == &ev_timeout) {
		log << std::dec << "Result TIMEOUT" << endl;
		param.msg.set_resulttype(CoolChecksumProtoMsg_ResultType_TIMEOUT);
		param.msg.set_resultdata(sal::simulator.getRegisterManager().getInstructionPointer());
	} else if (ev == &ev_trap) {
		log << std::dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
		param.msg.set_resulttype(CoolChecksumProtoMsg_ResultType_TRAP);
		param.msg.set_resultdata(sal::simulator.getRegisterManager().getInstructionPointer());
	} else {
		log << std::dec << "Result WTF?" << endl;
		param.msg.set_resulttype(CoolChecksumProtoMsg_ResultType_UNKNOWN);
		param.msg.set_resultdata(sal::simulator.getRegisterManager().getInstructionPointer());

		std::stringstream ss;
		ss << "eventid " << ev << " EIP " << sal::simulator.getRegisterManager().getInstructionPointer();
		param.msg.set_details(ss.str());
	}
	int32_t error_corrected = sal::simulator.getMemoryManager().getByte(COOL_ECC_ERROR_CORRECTED);
	param.msg.set_error_corrected(error_corrected);
	m_jc.sendResult(param);

	}
#endif
	// Explicitly terminate, or the simulator will continue to run.
	sal::simulator.terminate();
}
