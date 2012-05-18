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
#include "config/FailConfig.hpp"

#if COOL_FAULTSPACE_PRUNING
#include "plugins/tracing/TracingPlugin.hpp"
#endif

#include "coolchecksum.pb.h"

using std::endl;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_SUPPRESS_INTERRUPTS) || \
    !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, suppressed-interrupts, traps, save, and restore. Enable these in the configuration.
#endif

bool CoolChecksumExperiment::run()
{
	Logger log("CoolChecksum", false);
	fi::BPEvent bp;
	
	log << "startup" << endl;

#if 0
	// STEP 1: run until interesting function starts, and save state
	bp.setWatchInstructionPointer(COOL_ECC_FUNC_ENTRY);
	sal::simulator.addEventAndWait(&bp);
	log << "test function entry reached, saving state" << endl;
	log << "EIP = " << std::hex << bp.getTriggerInstructionPointer() << " or " << sal::simulator.getRegisterManager().getInstructionPointer() << endl;
	log << "error_corrected = " << std::dec << ((int)sal::simulator.getMemoryManager().getByte(COOL_ECC_ERROR_CORRECTED)) << endl;
	sal::simulator.save("coolecc.state");
#elif 0

	// STEP 2: determine # instructions from start to end
	log << "restoring state" << endl;
	sal::simulator.restore("coolecc.state");
	log << "EIP = " << std::hex << sal::simulator.getRegisterManager().getInstructionPointer() << endl;

#if COOL_FAULTSPACE_PRUNING
	// STEP 2.5: Additionally do a golden run with memory access tracing
	// for fault-space pruning. (optional!)
	log << "enabling tracing" << endl;
	TracingPlugin tp;

	// restrict memory access logging to injection target
	MemoryMap mm;
	mm.add(COOL_ECC_OBJUNDERTEST, COOL_ECC_OBJUNDERTEST_SIZE);
	tp.restrictMemoryAddresses(&mm);

	// record trace
	Trace trace;
	tp.setTraceMessage(&trace);

	// this must be done *after* configuring the plugin:
	sal::simulator.addFlow(&tp);
#endif

	// make sure the timer interrupt doesn't disturb us
	sal::simulator.addSuppressedInterrupt(0);

	int count;
	bp.setWatchInstructionPointer(fi::ANY_ADDR);
	for (count = 0; bp.getTriggerInstructionPointer() != COOL_ECC_CALCDONE; ++count) {
		sal::simulator.addEventAndWait(&bp);
		// log << "EIP = " << std::hex << sal::simulator.getRegisterManager().getInstructionPointer() << endl;
	}
	log << "test function calculation position reached after " << std::dec << count << " instructions" << endl;
	sal::Register* reg = sal::simulator.getRegisterManager().getRegister(sal::RID_CDX);
	log << std::dec << reg->getName() << " = " << reg->getData() << endl;

#if COOL_FAULTSPACE_PRUNING
	sal::simulator.removeFlow(&tp);

	// serialize trace to file
	std::ofstream of("trace.pb");
	if (of.fail()) {
		log << "failed to write trace.pb" << endl;
		sal::simulator.clearEvents(this);
		return false;
	}
	trace.SerializeToOstream(&of);
	of.close();
#endif

#elif 1
	// FIXME consider moving experiment repetition into Fail* or even the
	// SAL -- whether and how this is possible with the chosen backend is
	// backend specific
	for (int i = 0; i < 2000; ++i) {

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
	    << " (offset " << std::dec << instr_offset << ")"
	    << " bit " << bit_offset << ": 0x"
	    << std::hex << ((int)data) << " -> 0x" << ((int)newdata) << endl;
	// sanity check (only works if we're working with an instruction trace)
	if (param.msg.has_instr_address() &&
	    injection_ip != param.msg.instr_address()) {
		std::stringstream ss;
		ss << "SANITY CHECK FAILED: " << injection_ip
		   << " != " << param.msg.instr_address() << endl;
		log << ss.str();
		param.msg.set_resulttype(param.msg.UNKNOWN);
		param.msg.set_resultdata(injection_ip);
		param.msg.set_details(ss.str());

		sal::simulator.clearEvents();
		m_jc.sendResult(param);
		continue;
	}

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
		sal::Register* pRegRes = sal::simulator.getRegisterManager().getRegister(sal::RID_CDX);
		int32_t data = pRegRes->getData();
		log << std::dec << "Result " << pRegRes->getName() << " = " << data << endl;
		param.msg.set_resulttype(param.msg.CALCDONE);
		param.msg.set_resultdata(data);
	} else if (ev == &ev_timeout) {
		log << std::dec << "Result TIMEOUT" << endl;
		param.msg.set_resulttype(param.msg.TIMEOUT);
		param.msg.set_resultdata(sal::simulator.getRegisterManager().getInstructionPointer());
	} else if (ev == &ev_trap) {
		log << std::dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
		param.msg.set_resulttype(param.msg.TRAP);
		param.msg.set_resultdata(sal::simulator.getRegisterManager().getInstructionPointer());
	} else {
		log << std::dec << "Result WTF?" << endl;
		param.msg.set_resulttype(param.msg.UNKNOWN);
		param.msg.set_resultdata(sal::simulator.getRegisterManager().getInstructionPointer());

		std::stringstream ss;
		ss << "eventid " << ev << " EIP " << sal::simulator.getRegisterManager().getInstructionPointer();
		param.msg.set_details(ss.str());
	}
	sal::simulator.clearEvents();
	int32_t error_corrected = sal::simulator.getMemoryManager().getByte(COOL_ECC_ERROR_CORRECTED);
	param.msg.set_error_corrected(error_corrected);
	m_jc.sendResult(param);
	}

	// we do not want the simulator to continue running, especially for
	// headless and distributed experiments
	sal::simulator.terminate();
#endif
	// simulator continues to run
	sal::simulator.clearEvents(this);
	return true;
}
