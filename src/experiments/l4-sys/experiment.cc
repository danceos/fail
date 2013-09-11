#include <iostream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "UDIS86.hpp"
#include "InstructionFilter.hpp"
#include "aluinstr.hpp"
#include "campaign.hpp"
#include "conversion.hpp"

#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include "config/FailConfig.hpp"
#include "util/ProtoStream.hpp"
#include "TracePlugin.pb.h"
#include "util/gzstream/gzstream.h"

#include "l4sys.pb.h"

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
    !defined(CONFIG_EVENT_MEMREAD) || !defined(CONFIG_EVENT_MEMWRITE) || \
    !defined(CONFIG_SR_SAVE) || \
    !defined(CONFIG_EVENT_IOPORT)
#error This experiment needs: breakpoints, memory accesses, I/O port events, \
  save, and restore. Enable these in the configuration.
#endif

//string golden_run;
extern L4SysConversion l4sysRegisterConversion;

string L4SysExperiment::sanitised(const string &in_str) {
	string result;
	int in_str_size = in_str.size();
	result.reserve(in_str_size);
	for (int idx = 0; idx < in_str_size; idx++) {
		char cur_char = in_str[idx];
		unsigned cur_char_value = static_cast<unsigned>(cur_char);
		// also exclude the delimiter (',')
		if (cur_char_value < 0x20 || cur_char_value > 0x7E || cur_char_value == ',') {
			char str_nr[5];
			sprintf(str_nr, "\\%03o", cur_char_value);
			result += str_nr;
		} else {
			result += cur_char;
		}
	}
	return result;
}

BaseListener* L4SysExperiment::waitIOOrOther(bool clear_output) {
	IOPortListener ev_ioport(0x3F8, true);
	BaseListener* ev = NULL;
	if (clear_output)
		currentOutput.clear();
	while (true) {
		simulator.addListener(&ev_ioport);
		ev = simulator.resume();
        //log << "hello " << simulator.getListenerCount() << std::endl;
		//simulator.removeListener(&ev_ioport);
		if (ev == &ev_ioport) {
			currentOutput += ev_ioport.getData();
            //log << currentOutput << std::endl;
		} else {
			break;
		}
	}
	return ev;
}

Bit32u L4SysExperiment::eipBiased() {
	BX_CPU_C *cpu_context = simulator.getCPUContext();
	Bit32u EIP = cpu_context->gen_reg[BX_32BIT_REG_EIP].dword.erx;
	return EIP + cpu_context->eipPageBias;
}

const Bit8u *L4SysExperiment::calculateInstructionAddress() {
	// pasted in from various nested Bochs functions and macros - I hope
	// they will not change too soon (as do the Bochs developers, probably)
	BX_CPU_C *cpu_context = simulator.getCPUContext();
	const Bit8u *result = cpu_context->eipFetchPtr + eipBiased();
	return result;
}

bx_bool L4SysExperiment::fetchInstruction(BX_CPU_C *instance,
		const Bit8u *instr, bxInstruction_c *iStorage) {
	unsigned remainingInPage = instance->eipPageWindowSize - eipBiased();
	int ret;

#if BX_SUPPORT_X86_64
	if (BX_CPU_THIS_PTR cpu_mode == BX_MODE_LONG_64)
	ret = instance->fetchDecode64(instr, iStorage, remainingInPage);
	else
#endif
	ret = instance->fetchDecode32(instr, iStorage, remainingInPage);

	if (ret < 0) {
		// handle instrumentation callback inside boundaryFetch
		instance->boundaryFetch(instr, remainingInPage, iStorage);
		return 0;
	}

	return 1;
}

void L4SysExperiment::logInjection() {
// XXX fixme
#if 0
	// explicit type assignment necessary before sending over output stream
	int id = param->getWorkloadID();
	int instr_offset = param->msg.instr_offset();
	int bit_offset = param->msg.bit_offset();
	int exp_type = param->msg.exp_type();
	address_t injection_ip = param->msg.injection_ip();

	log << "job " << id << " exp_type " << exp_type << endl;
	log << "inject @ ip " << hex << injection_ip << " (offset " << dec << instr_offset
	    << ")" << " bit " << bit_offset << endl;
#endif
}

BaseListener *L4SysExperiment::singleStep(bool preserveAddressSpace) {
	// XXX: fixme
	return 0;
#if 0
	address_t aspace = (preserveAddressSpace ? L4SYS_ADDRESS_SPACE : ANY_ADDR);
	BPSingleListener singlestepping_event(ANY_ADDR, aspace);
	simulator.addListener(&singlestepping_event);
	/* prepare for the case that the kernel panics and never
	   switches back to this thread by introducing a scheduling timeout
	   of 10 seconds */
	TimerListener schedTimeout(10000000);
	simulator.addListener(&schedTimeout);
	BaseListener *ev = waitIOOrOther(false);
	simulator.removeListener(&singlestepping_event);
	simulator.removeListener(&schedTimeout);

	if (ev == &schedTimeout) {
		// otherwise we just assume this thread is never scheduled again
		log << "Result TIMEOUT" << endl;
		param->msg.set_resulttype(param->msg.TIMEOUT);
		param->msg.set_resultdata(
				simulator.getCPU(0).getInstructionPointer());
		param->msg.set_output(sanitised(currentOutput.c_str()));
		param->msg.set_details("Timed out immediately after injecting");

		m_jc.sendResult(*param);
		terminate(0);
	}
	return ev;
#endif
}

void L4SysExperiment::injectInstruction(
        bxInstruction_c *oldInstr, bxInstruction_c *newInstr) {
	// backup the current and insert the faulty instruction
	bxInstruction_c backupInstr;
	memcpy(&backupInstr, oldInstr, sizeof(bxInstruction_c));
	memcpy(oldInstr, newInstr, sizeof(bxInstruction_c));

	// execute the faulty instruction, then return
	singleStep(false);

	//restore the old instruction
	memcpy(oldInstr, &backupInstr, sizeof(bxInstruction_c));
}

unsigned L4SysExperiment::calculateTimeout(unsigned instr_left) {
	// the timeout in seconds, plus one backup second (avoids rounding overhead)
	// [instr] / [instr / s] = [s]
	unsigned seconds = instr_left / L4SYS_BOCHS_IPS + 1;
	// 1.1 (+10 percent) * 1000000 mus/s * [s]
	return 1100000 * seconds;
}

L4SysExperiment::L4SysExperiment()
 : m_jc("localhost"), log("L4Sys", false)
{
	 param = new L4SysExperimentData;
}

L4SysExperiment::~L4SysExperiment() {
	destroy();
}

void L4SysExperiment::destroy() {
	delete param;
	param = NULL;
}

void L4SysExperiment::terminate(int reason) {
	destroy();
	simulator.terminate(reason);
}

void L4SysExperiment::terminateWithError(string details, int reason) {
	L4SysProtoMsg_Result *result = param->msg.add_result();
	result->set_resulttype(param->msg.UNKNOWN);
	result->set_resultdata(simulator.getCPU(0).getInstructionPointer());
	result->set_output(sanitised(currentOutput.c_str()));
	result->set_details(details);

	m_jc.sendResult(*param);
	terminate(reason);
}


void L4SysExperiment::startAndSaveInitState(fail::BPSingleListener* bp)
{
	bp->setWatchInstructionPointer(L4SYS_FUNC_ENTRY);
	simulator.addListenerAndResume(bp);

	log << "test function entry reached, saving state" << endl;
	log << "EIP: expected " << hex << bp->getTriggerInstructionPointer()
			<< " and actually got "
			<< simulator.getCPU(0).getInstructionPointer()
			<< endl;
	log << "check the source code if the two instruction pointers are not equal" << endl;
	simulator.save(L4SYS_STATE_FOLDER);
    delete bp;
}
    
void L4SysExperiment::collectInstructionTrace(fail::BPSingleListener* bp)
{
	log << "restoring state" << endl;
	simulator.restore(L4SYS_STATE_FOLDER);
	log << "EIP = " << hex
			<< simulator.getCPU(0).getInstructionPointer()
			<< endl;

#ifdef L4SYS_FILTER_INSTRUCTIONS
	ofstream instr_list_file(L4SYS_INSTRUCTION_LIST, ios::binary);
    RangeSetInstructionFilter filtering(L4SYS_FILTER);
	bp->setWatchInstructionPointer(ANY_ADDR);

    fail::MemAccessListener ML(ANY_ADDR, MemAccessEvent::MEM_READWRITE);
    if (!simulator.addListener(&ML)) {
        log << "did not add memory listener..." << std::endl;
        exit(1);
    }
    if (!simulator.addListener(bp)) {
        log << "did not add breakpoint listener..." << std::endl;
        exit(1);
    }

	size_t count = 0, inst_accepted = 0, mem = 0, mem_valid = 0;
	map<address_t, unsigned> times_called_map;
    bool injecting = false;
    
    ogzstream out("trace.pb");
    ProtoOStream *os = new ProtoOStream(&out);
    
	while (bp->getTriggerInstructionPointer() != L4SYS_FUNC_EXIT) {
		fail::BaseListener *res = simulator.resume();
        address_t curr_addr = 0;
        
        // XXX: See the API problem below!
        if (res == &ML) {
            curr_addr = ML.getTriggerInstructionPointer();
            simulator.addListener(&ML);
            ++mem;
        } else if (res == bp) {
            curr_addr = bp->getTriggerInstructionPointer();
            assert(curr_addr == simulator.getCPU(0).getInstructionPointer());
            simulator.addListener(bp);
            ++count;
        }
        
        simtime_t prevtime = 0, currtime;
        simtime_diff_t deltatime;
        currtime = simulator.getTimerTicks();
        deltatime = currtime - prevtime;
        
        if (curr_addr == L4SYS_FILTER_ENTRY) {
            injecting = true;
        }

        if (curr_addr == L4SYS_FILTER_EXIT) {
            injecting = false;
        }

        if (!injecting or
            !filtering.isValidInstr(curr_addr, reinterpret_cast<char const*>(calculateInstructionAddress()))) {
            //log << "connt..." << std::endl;
            continue;
        }

        if (res == &ML) {
#if 0
            log << "Memory event IP " << std::hex << ML.getTriggerInstructionPointer()
                << " @ " << ML.getTriggerAddress() << "("
                << ML.getTriggerAccessType() << "," << ML.getTriggerWidth()
                << ")" << std::endl;
#endif
            ++mem_valid;

            Trace_Event te;
            if (deltatime != 0) { te.set_time_delta(deltatime); };
            te.set_ip(curr_addr);
            te.set_memaddr(ML.getTriggerAddress());
            te.set_accesstype( (ML.getTriggerAccessType() & MemAccessEvent::MEM_READ) ? te.READ : te.WRITE );
            te.set_width(ML.getTriggerWidth());
            os->writeMessage(&te);
        } else if (res == bp) {
            unsigned times_called = times_called_map[curr_addr];
            ++times_called;
            times_called_map[curr_addr] = times_called;
        
            //log << "breakpoint event" << std::endl;
            // now check if we want to add the instruction for fault injection
            ++inst_accepted;

            // 1) The 'old' way of logging instructions -> DEPRECATE soon
            //    BUT: we are currently using the bp_counter stored in this
            //    file!
            TraceInstr new_instr;
            //log << "writing IP " << hex << curr_addr << " counter "
            //    << dec << times_called << "(" << hex << BX_CPU(0)->cr3 << ")"
            //    << endl;
            new_instr.trigger_addr = curr_addr;
            new_instr.bp_counter = times_called;

            instr_list_file.write(reinterpret_cast<char*>(&new_instr), sizeof(TraceInstr));

            // 2) The 'new' way -> generate Events that can be processed by
            // the generic *-trace tools
            // XXX: need to log CR3 if we want multiple binaries here
            Trace_Event e;
            if (deltatime != 0) { e.set_time_delta(deltatime); };
            e.set_ip(curr_addr);
            os->writeMessage(&e);
        } else {
            printf("Unknown res? %p\n", res);
        }
        prevtime = currtime;

		//short sanity check
        //log << "continue..." << std::endl;
	}
	log << "saving instructions triggered during normal execution" << endl;
	instr_list_file.close();
	log << "test function calculation position reached after "
	    << dec << count << " instructions; " << inst_accepted << " accepted" << endl;
    log << "mem accesses: " << mem << ", valid: " << mem_valid << std::endl;
#else
	int count = 0;
	int ul = 0, kernel = 0;
	bp.setWatchInstructionPointer(ANY_ADDR);
	for (; bp.getTriggerInstructionPointer() != L4SYS_FUNC_EXIT; ++count) {
		simulator.addListenerAndResume(&bp);
		if (bp.getTriggerInstructionPointer() < 0xC0000000) {
			ul++;
		} else {
			kernel++;
		}
	}
	log << "EIP = " << hex
			<< simulator.getCPU(0).getInstructionPointer() << endl;
	log << "test function calculation position reached after "
			<< dec << count << " instructions; "
			<< "ul: " << ul << ", kernel: " << kernel << endl;
#endif
    delete bp;
}

void L4SysExperiment::goldenRun(fail::BPSingleListener* bp)
{
	log << "restoring state" << endl;
	simulator.restore(L4SYS_STATE_FOLDER);
	log << "EIP = " << hex
			<< simulator.getCPU(0).getInstructionPointer()
			<< endl;

	std::string golden_run;
	ofstream golden_run_file(L4SYS_CORRECT_OUTPUT);
	bp->setWatchInstructionPointer(L4SYS_FUNC_EXIT);
	simulator.addListener(bp);
	BaseListener* ev = waitIOOrOther(true);
	if (ev == bp) {
		golden_run.assign(currentOutput.c_str());
		golden_run_file << currentOutput.c_str();
		log << "Output successfully logged!" << endl;
	} else {
		log
				<< "Obviously, there is some trouble with"
				<< " the events registered - aborting simulation!"
				<< endl;
		golden_run_file.close();
		terminate(10);
	}

	log << "saving output generated during normal execution" << endl;
	golden_run_file.close();
    delete bp;
}


void L4SysExperiment::getJobParameters()
{
	// get the experiment parameters
	log << "asking job server for experiment parameters" << endl;
	if (!m_jc.getParam(*param)) {
		log << "Dying." << endl;
		// communicate that we were told to die
		terminate(1);
	}
}


void L4SysExperiment::validatePrerequisites()
{
	struct stat teststruct;
	if (stat(L4SYS_STATE_FOLDER, &teststruct) == -1 ||
		stat(L4SYS_CORRECT_OUTPUT, &teststruct) == -1) {
		log << "Important data missing - call \"prepare\" first." << endl;
		terminate(10);
	}
}


void L4SysExperiment::readGoldenRun(std::string& target)
{
	ifstream golden_run_file(L4SYS_CORRECT_OUTPUT);

	if (!golden_run_file.good()) {
		log << "Could not open file " << L4SYS_CORRECT_OUTPUT << endl;
		terminate(20);
	}

	target.assign((istreambuf_iterator<char>(golden_run_file)),
			       istreambuf_iterator<char>());

	golden_run_file.close();
}


void L4SysExperiment::setupFilteredBreakpoint(fail::BPSingleListener* bp, int instOffset)
{
    /*
     * The L4Sys experiment uses instruction filtering to restrict the range
     * of fault injection to only e.g., kernel instructions.
     *
     * To speed up injection, L4Sys furthermore does not use per-instruction
     * breakpoints but only places a breakpoint on the actually interesting
     * instruction (e.g., the injection EIP). Hence, we also do not count
     * instructions from the beginning of the experiment, but we count how
     * often a certain EIP was hit before the injection.
     *
     * To achieve these properties, we use an additional trace file that
     * provides us with a 'hit counter' of each injection candidate. We use
     * the global instruction ID (DataBaseCampaign: instruction_offset) to
     * index into this trace file and determine the value for the breakpoint
     * counter.
     */
    ifstream instr_list_file(L4SYS_INSTRUCTION_LIST, ios::binary);

    if (!instr_list_file.good()) {
        log << "Missing instruction trace" << endl;
        terminate(21);
    }

    TraceInstr curr_instr;
    instr_list_file.seekg(instOffset * sizeof(TraceInstr));
    log << instr_list_file.eof() << " " << instr_list_file.bad() << " "
        << instr_list_file.fail() << endl;
    if (instr_list_file.eof()) {
        log << "Job parameters indicate position outside the traced instruction list." << endl;
        terminate(1);
    }
    instr_list_file.read(reinterpret_cast<char*>(&curr_instr), sizeof(TraceInstr));
    instr_list_file.close();

    log << "setting watchpoint at " << hex << curr_instr.trigger_addr << endl;
    bp->setWatchInstructionPointer(curr_instr.trigger_addr);
    log << "setting bp counter " << hex << curr_instr.bp_counter << endl;
    bp->setCounter(curr_instr.bp_counter);
}


fail::BPSingleListener*
L4SysExperiment::prepareMemoryExperiment(int ip, int offset, int dataAddress)
{
    fail::BPSingleListener *bp = new BPSingleListener(0, L4SYS_ADDRESS_SPACE);
    log << "\033[34;1mMemory fault injection\033[0m at instruction " << std::hex << offset
        << ", ip " << ip << ", address " << dataAddress << std::endl;

#ifdef L4SYS_FILTER_INSTRUCTIONS
    setupFilteredBreakpoint(bp, offset);
    assert(bp->getWatchInstructionPointer() == (address_t)(ip & 0xFFFFFFFF));
#else
    bp->setWatchInstructionPointer(ANY_ADDR);
    bp->setCounter(instr_offset);
#endif
    return bp;
}


fail::BPSingleListener*
L4SysExperiment::prepareRegisterExperiment(int ip, int offset, int dataAddress)
{
    fail::BPSingleListener *bp = new BPSingleListener(0, L4SYS_ADDRESS_SPACE);
    
	int reg, regOffset;
	reg    = ((dataAddress >> 4) & 0xF) + 1; // regs start at 1
	regOffset = dataAddress & 0xF;

	log << "\033[32;1mGPR bitflip\033[0m at instr. offset " << offset
	    << " reg data (" << reg << ", " 
        << regOffset << ")" << std::endl;

#ifdef L4SYS_FILTER_INSTRUCTIONS
    setupFilteredBreakpoint(bp, offset);
    assert(bp->getWatchInstructionPointer() == (address_t)(ip & 0xFFFFFFFF));
    log << bp->getCounter() << std::endl;
#else
    bp->setWatchInstructionPointer(ANY_ADDR);
    bp->setCounter(instr_offset);
#endif
    return bp;
}

    
void L4SysExperiment::doMemoryInjection(int address, int bit)
{
    MemoryManager& mm = simulator.getMemoryManager();
    byte_t data = mm.getByte(address);
    byte_t newdata = data ^ (1 << bit);
    mm.setByte(address, newdata);
    log << "[" << std::hex << address << "] " << (int)data
        << " -> " << (int)newdata << std::endl;
}


void L4SysExperiment::doRegisterInjection(int regDesc, int bit)
{
	int reg, offset;
	reg    = (regDesc >> 4) + 1; // regs start at 1
	offset = regDesc & 0xF;
    
    ConcreteCPU& cpu = simulator.getCPU(0);
    Register *reg_target = cpu.getRegister(reg - 1);
    regdata_t data = cpu.getRegisterContent(reg_target);
    regdata_t newdata = data ^ (1 << (bit + 8 * offset));
    cpu.setRegisterContent(reg_target, newdata);
    log << "Reg[" << reg << "]: " << std::hex << data << " -> "
        << newdata << std::endl;
}


bool L4SysExperiment::run()
{
	BPSingleListener *bp = 0;
	srand(time(NULL));

	log << "Starting L4Sys Experiment, phase " << PREPARATION_STEP << endl;

#if PREPARATION_STEP == 1
	// STEP 1: run until interesting function starts, and save state
    startAndSaveInitState(new BPSingleListener(0, L4SYS_ADDRESS_SPACE));
#elif PREPARATION_STEP == 2
	// STEP 2: determine instructions executed
    collectInstructionTrace(new BPSingleListener(0, L4SYS_ADDRESS_SPACE));

#elif PREPARATION_STEP == 3
	// STEP 3: determine the output of a "golden run"
    goldenRun(new BPSingleListener(0, L4SYS_ADDRESS_SPACE));

#elif PREPARATION_STEP == 0
	// LAST STEP: The actual experiment.
    validatePrerequisites();

	// Read the golden run output for validation purposes
	std::string golden_run;
    readGoldenRun(golden_run);
    
    getJobParameters();

	int exp_type     = param->msg.exp_type();
	int instr_offset = param->msg.fsppilot().injection_instr();
	int regData      = param->msg.fsppilot().data_address();

    if (exp_type == param->msg.MEM) {
        bp = prepareMemoryExperiment(param->msg.fsppilot().injection_instr_absolute(),
                                     param->msg.fsppilot().injection_instr(),
                                     param->msg.fsppilot().data_address());
    } else if (exp_type == param->msg.GPRFLIP) {
        bp = prepareRegisterExperiment(param->msg.fsppilot().injection_instr_absolute(),
                                       param->msg.fsppilot().injection_instr(),
                                       param->msg.fsppilot().data_address());
    } else {
        log << "Unsupported experiment type: " << exp_type << std::endl;
        terminate(1);
    }

    assert(bp);

    for (unsigned bit = 0; bit < 8; ++bit) {

		L4SysProtoMsg_Result *result = param->msg.add_result();
		result->set_instr_offset(instr_offset);

        simulator.clearListeners();

        log << "Bit " << bit << ", restoring state." << endl;
        simulator.restore(L4SYS_STATE_FOLDER);
        log << " ... EIP = " << std::hex << simulator.getCPU(0).getInstructionPointer() << std::endl;
        
        simulator.addListener(bp);
        
        simtime_t now = simulator.getTimerTicks();
        fail::BaseListener *go = waitIOOrOther(true);
        assert(go == bp);
        
        log << "Hit BP. Start time " << now << ", new time " << simulator.getTimerTicks()
            << ", diff = " << simulator.getTimerTicks() - now << std::endl;

        assert(bp->getTriggerInstructionPointer() == bp->getWatchInstructionPointer());
        result->set_injection_ip(bp->getTriggerInstructionPointer());
        
        if (exp_type == param->msg.MEM) {
            result->set_bit_offset(bit);
            doMemoryInjection(param->msg.fsppilot().data_address(), bit);
        } else if (exp_type == param->msg.GPRFLIP) {
            result->set_bit_offset(bit + 8 * (param->msg.fsppilot().data_address() & 0xF));
            doRegisterInjection(param->msg.fsppilot().data_address(), bit);
        } else {
          log << "doing nothing for experiment type " << exp_type << std::endl;
        }
            
        BPSingleListener ev_done(L4SYS_FUNC_EXIT, L4SYS_ADDRESS_SPACE);
        simulator.addListener(&ev_done);

        unsigned instr_left = L4SYS_TOTINSTR - instr_offset; // XXX offset is in NUMINSTR, TOTINSTR is higher
        BPSingleListener ev_incomplete(ANY_ADDR, L4SYS_ADDRESS_SPACE);
        ev_incomplete.setCounter(instr_left);
        simulator.addListener(&ev_incomplete);

        TimerListener ev_timeout(calculateTimeout(instr_left));
        simulator.addListener(&ev_timeout);
        log << "continue... (" <<  simulator.getListenerCount()
            << " breakpoints, timeout @ " << ev_timeout.getTimeout()
            << std::endl;

        //do not discard output recorded so far
        BaseListener *ev = waitIOOrOther(false);

        /* copying a string object that contains control sequences
         * unfortunately does not work with the library I am using,
         * which is why output is passed on as C string and
         * the string compare is done on C strings
         */
        if (ev == &ev_done) {
            if (strcmp(currentOutput.c_str(), golden_run.c_str()) == 0) {
                log << "Result DONE" << endl;
                result->set_resulttype(param->msg.DONE);
            } else {
                log << "Result WRONG" << endl;
                result->set_resulttype(param->msg.WRONG);
                result->set_output(sanitised(currentOutput.c_str()));
            }
        } else if (ev == &ev_incomplete) {
            log << "Result INCOMPLETE" << endl;
            result->set_resulttype(param->msg.INCOMPLETE);
            result->set_resultdata(simulator.getCPU(0).getInstructionPointer());
            result->set_output(sanitised(currentOutput.c_str()));
        } else if (ev == &ev_timeout) {
            log << "Result TIMEOUT" << endl;
            result->set_resulttype(param->msg.TIMEOUT);
            result->set_resultdata(simulator.getCPU(0).getInstructionPointer());
            result->set_output(sanitised(currentOutput.c_str()));
        } else {
            log << "Result WTF?" << endl;
            stringstream ss;
            ss << "eventid " << ev;
            terminateWithError(ss.str(), 50);
        }
    }
        
    m_jc.sendResult(*param);

// XXX: Fixme to work with database campaign!
#if 0
		else if (exp_type == param->msg.IDCFLIP) {
			// this is a twisted one

			// initial definitions
			bxInstruction_c *currInstr = simulator.getCurrentInstruction();
			unsigned length_in_bits = currInstr->ilen() << 3;

			// get the instruction in plain text and inject the error there
			// Note: we need to fetch some extra bytes into the array
			// in case the faulty instruction is interpreted to be longer
			// than the original one
			Bit8u curr_instr_plain[MAX_INSTR_BYTES];
			const Bit8u *addr = calculateInstructionAddress();
			memcpy(curr_instr_plain, addr, MAX_INSTR_BYTES);

			// CampaignManager has no idea of the instruction length
			// (neither do we), therefore this small adaption
			bit_offset %= length_in_bits;
			param->msg.set_bit_offset(bit_offset);

			// do some access calculation
			int byte_index = bit_offset >> 3;
			Bit8u bit_index = bit_offset & 7;

			// apply the fault
			curr_instr_plain[byte_index] ^= 0x80 >> bit_index;

			// decode the instruction
			bxInstruction_c bochs_instr;
			memset(&bochs_instr, 0, sizeof(bxInstruction_c));
			fetchInstruction(simulator.getCPUContext(), curr_instr_plain,
							 &bochs_instr);

			// inject it
			injectInstruction(currInstr, &bochs_instr);

			// do the logging
			logInjection();
		} else if (exp_type == param->msg.RATFLIP) {
			ud_type_t which = UD_NONE;
			unsigned rnd = 0;
			Udis86 udis(injection_ip);
			do {
				bxInstruction_c *currInstr = simulator.getCurrentInstruction();
				udis.setInputBuffer(calculateInstructionAddress(), currInstr->ilen());
				if (!udis.fetchNextInstruction()) {
					terminateWithError(
									   "Could not decode instruction using UDIS86", 32);
				}
				ud_t _ud = udis.getCurrentState();

				/* start Bjoern Doebel's code (slightly modified) */
				/* ============================================== */
				unsigned opcount     = 0;
				unsigned operands[4] = { ~0U, ~0U, ~0U, ~0U };
				enum {
					RAT_IDX_MASK   = 0x0FF,
					RAT_IDX_OFFSET = 0x100
				};

				for (unsigned i = 0; i < 3; ++i) {
					/*
					 * Case 1: operand is a register
					 */
					if (_ud.operand[i].type == UD_OP_REG) {
						operands[opcount++] = i;
					} else if (_ud.operand[i].type == UD_OP_MEM) {
						/*
						 * Case 2: operand is memory op.
						 *
						 * In this case, we may have 2 registers involved for the
						 * index-scale address calculation.
						 */
						if (_ud.operand[i].base != 0)  // 0 if hard-wired mem operand
							operands[opcount++] = i;
						if (_ud.operand[i].index != 0)
							operands[opcount++] = i + RAT_IDX_OFFSET;
					}
				}

				if (opcount == 0) {
					// try the next instruction
					singleStep(true);
				} else {
					// assign the necessary variables
					rnd = rand() % opcount;

					if (operands[rnd] > RAT_IDX_OFFSET) {
						which = _ud.operand[operands[rnd] - RAT_IDX_OFFSET].index;
					} else {
						which = _ud.operand[operands[rnd]].base;
					}
				}
				/* ============================================ */
				/* end Bjoern Doebel's code (slightly modified) */

			} while (which == UD_NONE &&
					 simulator.getCPU(0).getInstructionPointer() != L4SYS_FUNC_EXIT);

			if (simulator.getCPU(0).getInstructionPointer() == L4SYS_FUNC_EXIT) {
				stringstream ss;
				ss << "Reached the end of the experiment ";
				ss << "without finding an appropriate instruction";

				terminateWithError(ss.str(), 33);
			}

			// store the real injection point
			param->msg.set_injection_ip(simulator.getCPU(0).getInstructionPointer());

			// so we are able to flip the associated registers
			// for details on the algorithm, see Bjoern Doebel's SWIFI/RATFlip class

			// some declarations
			GPRegisterId bochs_reg = Udis86::udisGPRToFailBochsGPR(which);
			param->msg.set_register_offset(static_cast<L4SysProtoMsg_RegisterType>(bochs_reg + 1));
			ConcreteCPU &cpu = simulator.getCPU(0);
			Register *bochsRegister = cpu.getRegister(bochs_reg);
			Register *exchangeRegister = NULL;

			// first, decide if the fault hits a register bound to this thread
			// (ten percent chance)
			if (rand() % 10 == 0) {
				// assure exchange of registers
				unsigned int exchg_reg = rand() % 7;
				if (exchg_reg == bochs_reg)
					exchg_reg++;
				exchangeRegister = cpu.getRegister(exchg_reg);
				param->msg.set_details(l4sysRegisterConversion.output(exchg_reg + 1));
			}

			// prepare the fault
			regdata_t data = cpu.getRegisterContent(bochsRegister);
			if (rnd > 0) {
				//input register - do the fault injection here
				regdata_t newdata = 0;
				if (exchangeRegister != NULL) {
					// the data is taken from a process register chosen before
					newdata = cpu.getRegisterContent(exchangeRegister);
				} else {
					// the data comes from an uninitialised register
					newdata = rand();
					stringstream ss;
					ss << "0x" << hex << newdata;
					param->msg.set_details(ss.str());
				}
				cpu.setRegisterContent(bochsRegister, newdata);
			}

			// execute the instruction
			singleStep(true);

			// restore the register if we are still in the thread
			if (rnd == 0) {
				// output register - do the fault injection here
				if (exchangeRegister != NULL) {
					// write the result into the wrong local register
					regdata_t newdata = cpu.getRegisterContent(bochsRegister);
					cpu.setRegisterContent(exchangeRegister, newdata);
				}
				// otherwise, just assume it is stored in an unused register
			}
			// restore the actual value of the register
			// in reality, it would never have been overwritten
			cpu.setRegisterContent(bochsRegister, data);

			// log the injection
			logInjection();

		} else if (exp_type == param->msg.ALUINSTR) {
			static BochsALUInstructions aluInstrObject(aluInstructions, aluInstructionsSize);
			// find the closest ALU instruction after the current IP

			bxInstruction_c *currInstr;
			while (!aluInstrObject.isALUInstruction(
													currInstr = simulator.getCurrentInstruction()) &&
				   simulator.getCPU(0).getInstructionPointer() != L4SYS_FUNC_EXIT) {
				singleStep(true);
			}

			if (simulator.getCPU(0).getInstructionPointer() == L4SYS_FUNC_EXIT) {
				stringstream ss;
				ss << "Reached the end of the experiment ";
				ss << "without finding an appropriate instruction";

				terminateWithError(ss.str(), 34);
			}

			// store the real injection point
			param->msg.set_injection_ip(simulator.getCPU(0).getInstructionPointer());

			// now exchange it with a random equivalent
			bxInstruction_c newInstr;
			string details;
			aluInstrObject.randomEquivalent(newInstr, details);
			if (memcmp(&newInstr, currInstr, sizeof(bxInstruction_c)) == 0) {
				// something went wrong - exit experiment
				terminateWithError(
								   "Did not hit an ALU instruction - correct the source code please!",
								   40);
			}
			// record information on the new instruction
			param->msg.set_details(details);

			// inject it
			injectInstruction(currInstr, &newInstr);

			// do the logging
			logInjection();
		}
#endif

#endif

    terminate(0);
	// experiment successfully conducted
	return true;
}
