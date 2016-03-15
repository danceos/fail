#include <iostream>

#include "util/Logger.hpp"
#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include "config/FailConfig.hpp"

#if COOL_FAULTSPACE_PRUNING
#include "../plugins/tracing/TracingPlugin.hpp"
#endif

#include "coolchecksum.pb.h"

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_SUPPRESS_INTERRUPTS) || \
    !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, suppressed-interrupts, traps, save, and restore. Enable these in the configuration.
#endif

bool CoolChecksumExperiment::run()
{
	Logger log("CoolChecksum", false);
	BPSingleListener bp;

	log << "startup" << endl;

#if 1
	// STEP 1: run until interesting function starts, and save state
	bp.setWatchInstructionPointer(COOL_ECC_FUNC_ENTRY);
	simulator.addListenerAndResume(&bp);
	log << "test function entry reached, saving state" << endl;
	log << "EIP = " << hex << bp.getTriggerInstructionPointer() << " or " << simulator.getRegisterManager().getInstructionPointer() << endl;
	log << "error_corrected = " << dec << ((int)simulator.getMemoryManager().getByte(COOL_ECC_ERROR_CORRECTED)) << endl;
	simulator.save("coolecc.state");
#elif 0

	// STEP 2: determine # instructions from start to end
	log << "restoring state" << endl;
	simulator.restore("coolecc.state");
	log << "EIP = " << hex << simulator.getRegisterManager().getInstructionPointer() << endl;

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
	ofstream of("trace.pb");
	tp.setTraceFile(&of);

	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);
#endif

	// make sure the timer interrupt doesn't disturb us
	simulator.addSuppressedInterrupt(0);

	int count;
	bp.setWatchInstructionPointer(ANY_ADDR);
	for (count = 0; bp.getTriggerInstructionPointer() != COOL_ECC_CALCDONE; ++count) {
		simulator.addListenerAndResume(&bp);
		// log << "EIP = " << hex << simulator.getRegisterManager().getInstructionPointer() << endl;
	}
	log << "test function calculation position reached after " << dec << count << " instructions" << endl;
	Register* reg = simulator.getRegisterManager().getRegister(RID_CDX);
	log << dec << reg->getName() << " = " << reg->getData() << endl;

#if COOL_FAULTSPACE_PRUNING
	simulator.removeFlow(&tp);

	// serialize trace to file
	if (of.fail()) {
		log << "failed to write trace.pb" << endl;
		simulator.clearListeners(this);
		return false;
	}
	of.close();
#endif

#elif 1
	// FIXME consider moving experiment repetition into FAIL* or even the
	// SAL -- whether and how this is possible with the chosen backend is
	// backend specific
	for (int i = 0; i < 2000; ++i) {

	// STEP 3: The actual experiment.
	log << "restoring state" << endl;
	simulator.restore("coolecc.state");

	log << "asking job server for experiment parameters" << endl;
	CoolChecksumExperimentData param;
	if (!m_jc.getParam(param)) {
		log << "Dying." << endl;
		// communicate that we were told to die
		simulator.terminate(1);
	}
	int id = param.getWorkloadID();
	int instr_offset = param.msg.instr_offset();
	int bit_offset = param.msg.bit_offset();
	log << "job " << id << " instr " << instr_offset << " bit " << bit_offset << endl;

	// FIXME could be improved (especially for backends supporting
	// breakpoints natively) by utilizing a previously recorded instruction
	// trace
	bp.setWatchInstructionPointer(ANY_ADDR);
	for (int count = 0; count < instr_offset; ++count) {
		simulator.addListenerAndResume(&bp);
	}

	// inject
	guest_address_t inject_addr = COOL_ECC_OBJUNDERTEST + bit_offset / 8;
	MemoryManager& mm = simulator.getMemoryManager();
	byte_t data = mm.getByte(inject_addr);
	byte_t newdata = data ^ (1 << (bit_offset % 8));
	mm.setByte(inject_addr, newdata);
	// note at what IP we did it
	int32_t injection_ip = simulator.getRegisterManager().getInstructionPointer();
	param.msg.set_injection_ip(injection_ip);
	log << "inject @ ip " << injection_ip
	    << " (offset " << dec << instr_offset << ")"
	    << " bit " << bit_offset << ": 0x"
	    << hex << ((int)data) << " -> 0x" << ((int)newdata) << endl;
	// sanity check (only works if we're working with an instruction trace)
	if (param.msg.has_instr_address() &&
	    injection_ip != param.msg.instr_address()) {
		stringstream ss;
		ss << "SANITY CHECK FAILED: " << injection_ip
		   << " != " << param.msg.instr_address() << endl;
		log << ss.str();
		param.msg.set_resulttype(param.msg.UNKNOWN);
		param.msg.set_resultdata(injection_ip);
		param.msg.set_details(ss.str());

		simulator.clearListeners();
		m_jc.sendResult(param);
		continue;
	}

	// aftermath
	BPSingleListener ev_done(COOL_ECC_CALCDONE);
	simulator.addListener(&ev_done);
	BPSingleListener ev_timeout(ANY_ADDR);
	ev_timeout.setCounter(COOL_ECC_NUMINSTR + 3000);
	simulator.addListener(&ev_timeout);
	TrapListener ev_trap(ANY_TRAP);
	simulator.addListener(&ev_trap);

	BaseListener* ev = simulator.resume();
	if (ev == &ev_done) {
		Register* pRegRes = simulator.getRegisterManager().getRegister(RID_CDX);
		int32_t data = pRegRes->getData();
		log << dec << "Result " << pRegRes->getName() << " = " << data << endl;
		param.msg.set_resulttype(param.msg.CALCDONE);
		param.msg.set_resultdata(data);
	} else if (ev == &ev_timeout) {
		log << dec << "Result TIMEOUT" << endl;
		param.msg.set_resulttype(param.msg.TIMEOUT);
		param.msg.set_resultdata(simulator.getRegisterManager().getInstructionPointer());
	} else if (ev == &ev_trap) {
		log << dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
		param.msg.set_resulttype(param.msg.TRAP);
		param.msg.set_resultdata(simulator.getRegisterManager().getInstructionPointer());
	} else {
		log << dec << "Result WTF?" << endl;
		param.msg.set_resulttype(param.msg.UNKNOWN);
		param.msg.set_resultdata(simulator.getRegisterManager().getInstructionPointer());

		stringstream ss;
		ss << "event addr " << ev << " EIP " << simulator.getRegisterManager().getInstructionPointer();
		param.msg.set_details(ss.str());
	}
	simulator.clearListeners();
	int32_t error_corrected = simulator.getMemoryManager().getByte(COOL_ECC_ERROR_CORRECTED);
	param.msg.set_error_corrected(error_corrected);
	m_jc.sendResult(param);
	}

	// we do not want the simulator to continue running, especially for
	// headless and distributed experiments
	simulator.terminate();
#endif
	// simulator continues to run
	simulator.clearListeners(this);
	return true;
}
