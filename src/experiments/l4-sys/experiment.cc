#include <iostream>
#include <sstream>
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

#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include <sal/bochs/BochsMemory.hpp>
#include "config/FailConfig.hpp"
#include "TracePlugin.pb.h"
#include "util/ProtoStream.hpp"
#include "util/gzstream/gzstream.h"
#include "util/CommandLine.hpp"

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

unsigned L4SysExperiment::calculateTimeout(unsigned instr_left, unsigned ips) {
	// the timeout in seconds, plus one backup second (avoids rounding overhead)
	// [instr] / [instr / s] = [s]
	unsigned seconds = instr_left / ips + 1;
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

void L4SysExperiment::terminateWithError(string details, int reason,
                                         L4SysProtoMsg_Result *r = 0) {
    L4SysProtoMsg_Result *result;
    
	if (r)
		result = r;
	else
		result = param->msg.add_result();

	result->set_resulttype(param->msg.UNKNOWN);
	result->set_resultdata(simulator.getCPU(0).getInstructionPointer());
	result->set_output(sanitised(currentOutput.c_str()));
	result->set_details(details);

	m_jc.sendResult(*param);
	terminate(reason);
}


void L4SysExperiment::runToStart(fail::BPSingleListener *bp)
{
	bp->setWatchInstructionPointer(conf.func_entry);

	log << "run until ip reaches 0x" << hex << conf.func_entry << endl;

	simulator.addListenerAndResume(bp);

	log << "test function entry reached, saving state" << endl;
	log << "EIP: expected " << hex << bp->getTriggerInstructionPointer()
			<< " and actually got "
			<< simulator.getCPU(0).getInstructionPointer()
			<< endl;
	log << "check the source code if the two instruction pointers are not equal" << endl;

	if(conf.address_space == conf.address_space_trace) {
		conf.address_space_trace = BX_CPU(0)->cr3;
	} 

		conf.address_space = BX_CPU(0)->cr3;
}



BaseListener* L4SysExperiment::afterInjection(L4SysProtoMsg_Result* res)
{
	BaseListener *bl = 0;

	simtime_t t_inject = simulator.getTimerTicks();
	simtime_t t_bailout;

    ifstream instr_list_file(conf.instruction_list.c_str(), ios::binary);
	instr_list_file.seekg((1 + res->instr_offset()) * sizeof(TraceInstr));

    RangeSetInstructionFilter filtering(conf.filter.c_str());

	for (;;) {
		// Step over _all_ instructions in the trace AS
		BPSingleListener stepListener(ANY_ADDR, conf.address_space_trace);

		TraceInstr curr_instr;
		instr_list_file.read(reinterpret_cast<char*>(&curr_instr),
		                     sizeof(TraceInstr));

		t_bailout = simulator.getTimerTicks();

		// step until next traced instruction
		simulator.addListener(&stepListener);
		bl = waitIOOrOther(false);
		
		// bail out if we hit a listener other than the single step
		// one -> in this case the experiment is over prematurely
		if (bl != &stepListener) {
			// Note, the difference in this case is the diff between the
			// last correct instruction and the starting point -> this is
			// useful for TIMEOUT events where the actual time now would be
			// the complete TIMEOUT whereas we are interested in the time
			// until execution deviates from the original trace
			res->set_deviate_steps(t_bailout - t_inject);
			res->set_deviate_eip(-1);
			log << "bailing out of single-stepping mode" << endl;
			break;
		}

		address_t eip = stepListener.getTriggerInstructionPointer();

		if (!filtering.isValidInstr(eip))
			continue;

		if (eip != curr_instr.trigger_addr) {
			// In the case where we see an actual instruction stream deviation, we
			// want the real diff between NOW and the injection start point
			t_bailout = simulator.getTimerTicks();
			log << "got " << hex << eip << " expected "
				<< curr_instr.trigger_addr << endl;

			log << "mismatch found after " << (t_bailout - t_inject) << " instructions." << endl;
			res->set_deviate_steps(t_bailout - t_inject);
			res->set_deviate_eip(eip);
			
			return waitIOOrOther(false);
		}
	}
    
	log << "left single-stepping mode after " << (t_bailout - t_inject)
		<< " instructions." << endl;
	return bl;
}
    
void L4SysExperiment::collectInstructionTrace(fail::BPSingleListener* bp)
{
	fail::MemAccessListener ML(ANY_ADDR, MemAccessEvent::MEM_READWRITE);
	ogzstream out(conf.trace.c_str());
	ProtoOStream *os = new ProtoOStream(&out);

	size_t count = 0, inst_accepted = 0, mem = 0, mem_valid = 0;
	simtime_t prevtime = 0, currtime;
	simtime_diff_t deltatime;

	log << "restoring state" << endl;
	simulator.restore(conf.state_folder.c_str());
	currtime = simulator.getTimerTicks();

	log << "EIP = " << hex
			<< simulator.getCPU(0).getInstructionPointer()
			<< endl;

    if (!simulator.addListener(&ML)) {
        log << "did not add memory listener..." << std::endl;
        exit(1);
    }
    if (!simulator.addListener(bp)) {
        log << "did not add breakpoint listener..." << std::endl;
        exit(1);
    }

#if L4SYS_FILTER_INSTRUCTIONS
	ofstream instr_list_file(conf.instruction_list.c_str(), ios::binary);
    RangeSetInstructionFilter filtering(conf.filter.c_str());
	bp->setWatchInstructionPointer(ANY_ADDR);

	map<address_t, unsigned> times_called_map;
    bool injecting = false;

	while (bp->getTriggerInstructionPointer() != conf.func_exit) {
		fail::BaseListener *res = simulator.resume();
        address_t curr_addr = 0;
        
        // XXX: See the API problem below!
        if (res == &ML) {
            curr_addr = ML.getTriggerInstructionPointer();
            simulator.addListener(&ML);
            if ((conf.address_space_trace != ANY_ADDR) && (BX_CPU(0)->cr3 != conf.address_space_trace)) {
                continue;
            }
            ++mem;
        } else if (res == bp) {
            curr_addr = bp->getTriggerInstructionPointer();
            assert(curr_addr == simulator.getCPU(0).getInstructionPointer());
            simulator.addListener(bp);
            ++count;
        }
        
        currtime = simulator.getTimerTicks();
        deltatime = currtime - prevtime;
        
        if (curr_addr == conf.filter_entry) {
            injecting = true;
        }

        if (curr_addr == conf.filter_exit) {
            injecting = false;
        }

        // Only trace if:
        // 1) we are between FILTER_ENTRY and FILTER_EXIT, and
        // 2) we have a valid instruction according to filter rules, and
        // 3) we are in the TRACE address space
        if (!injecting or
            !filtering.isValidInstr(curr_addr, reinterpret_cast<char const*>(calculateInstructionAddress()))
                       or
            (BX_CPU(0)->cr3 != conf.address_space_trace)
            ) {
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
            if (deltatime != 0) { te.set_time_delta(1); };
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
            if (deltatime != 0) { e.set_time_delta(1); };
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
	bp->setWatchInstructionPointer(ANY_ADDR);
	while (bp->getTriggerInstructionPointer() != conf.func_exit)
	{
		fail::BaseListener *res = simulator.resume();
        address_t curr_addr = 0;
        
        // XXX: See the API problem below!
        if (res == &ML) {
            curr_addr = ML.getTriggerInstructionPointer();
            simulator.addListener(&ML);
            if ((func.address_space_trace != ANY_ADDR) && (BX_CPU(0)->cr3 != func.address_space_trace)) {
                continue;
            }
            ++mem;
        } else if (res == bp) {
            curr_addr = bp->getTriggerInstructionPointer();
            assert(curr_addr == simulator.getCPU(0).getInstructionPointer());
            simulator.addListener(bp);
            ++count;
        }
#if 0
		if (curr_addr < 0xC0000000) // XXX filter for kernel-only experiment
			continue;
#endif        
        currtime = simulator.getTimerTicks();
        deltatime = currtime - prevtime;

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
            Trace_Event e;
            if (deltatime != 0) { e.set_time_delta(deltatime); };
            e.set_ip(curr_addr);
            os->writeMessage(&e);
        } else {
            printf("Unknown res? %p\n", res);
        }
        prevtime = currtime;
	}
	log << "test function calculation position reached after "
	    << dec << count << " instructions; " << count << " accepted" << endl;
    log << "mem accesses: " << mem << ", valid: " << mem_valid << std::endl;
#endif
		conf.numinstr = inst_accepted;
		conf.totinstr = count;
    delete bp;
}

void L4SysExperiment::goldenRun(fail::BPSingleListener* bp)
{
	log << "restoring state" << endl;
	simulator.restore(conf.state_folder.c_str());
	log << "EIP = " << hex
			<< simulator.getCPU(0).getInstructionPointer()
			<< endl;

	std::string golden_run;
	ofstream golden_run_file(conf.golden_run.c_str());
	bp->setWatchInstructionPointer(conf.func_exit);
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


void L4SysExperiment::validatePrerequisites(std::string state, std::string output)
{
	struct stat teststruct;
	if (stat(state.c_str(), &teststruct) == -1 ||
		stat(output.c_str(), &teststruct) == -1) {
		log << "Important data missing - call \"prepare\" first." << endl;
		terminate(10);
	}
}


void L4SysExperiment::readGoldenRun(std::string& target, std::string golden_run)
{
	ifstream golden_run_file(golden_run.c_str());

	if (!golden_run_file.good()) {
		log << "Could not open file " << golden_run.c_str() << endl;
		terminate(20);
	}

	target.assign((istreambuf_iterator<char>(golden_run_file)),
			       istreambuf_iterator<char>());

	golden_run_file.close();
}


void L4SysExperiment::setupFilteredBreakpoint(fail::BPSingleListener* bp, int instOffset, std::string instr_list)
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
    ifstream instr_list_file(instr_list.c_str(), ios::binary);

    if (!instr_list_file.good()) {
        log << "Missing instruction trace" << endl;
        terminate(21);
    }

    log << "inst offset " << dec << instOffset << " sizeof(TraceInstr) " << sizeof(TraceInstr) << endl;
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
    fail::BPSingleListener *bp = new BPSingleListener(0, conf.address_space_trace);
    log << "\033[34;1mMemory fault injection\033[0m at instruction " << std::hex << offset
        << ", ip " << ip << ", address " << dataAddress << std::endl;

#if L4SYS_FILTER_INSTRUCTIONS
    setupFilteredBreakpoint(bp, offset, conf.instruction_list);
    assert(bp->getWatchInstructionPointer() == (address_t)(ip & 0xFFFFFFFF));
#else
    bp->setWatchInstructionPointer(ANY_ADDR);
    bp->setCounter(offset);
#endif
    return bp;
}


fail::BPSingleListener*
L4SysExperiment::prepareRegisterExperiment(int ip, int offset, int dataAddress)
{
    fail::BPSingleListener *bp = new BPSingleListener(0, conf.address_space_trace);
    
	int reg, regOffset;
	reg    = ((dataAddress >> 4) & 0xF) + 1; // regs start at 1
	regOffset = dataAddress & 0xF;

	log << "\033[32;1mGPR bitflip\033[0m at instr. offset " << offset
	    << " reg data (" << reg << ", " 
        << regOffset << ")" << std::endl;

#if L4SYS_FILTER_INSTRUCTIONS
    setupFilteredBreakpoint(bp, offset, conf.instruction_list);
    log << bp->getWatchInstructionPointer() << std::endl;
    log << ip << std::endl;
    assert(bp->getWatchInstructionPointer() == (address_t)(ip & 0xFFFFFFFF));
    log << bp->getCounter() << std::endl;
#else
    log << "Exp offset: " << offset << std::endl;
    bp->setWatchInstructionPointer(ANY_ADDR);
    bp->setCounter(offset);
#endif
    return bp;
}

    
bool L4SysExperiment::doMemoryInjection(int address, int bit)
{
    MemoryManager& mm = simulator.getMemoryManager();

    // XXX: evil, but I need to bail out if memory access is invalid
	host_address_t addr = reinterpret_cast<BochsMemoryManager*>(&mm)->guestToHost(address);
	if (addr == (host_address_t)ADDR_INV)
		return false;

    byte_t data = mm.getByte(address);
    byte_t newdata = data ^ (1 << bit);
    mm.setByte(address, newdata);
    log << "[" << std::hex << address << "] " << (int)data
        << " -> " << (int)newdata << std::endl;
    return true;
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

void L4SysExperiment::doExperiments(fail::BPSingleListener* bp) {
	// LAST STEP: The actual experiment.
    validatePrerequisites(conf.state_folder, conf.golden_run);

	// Read the golden run output for validation purposes
	std::string golden_run;
    readGoldenRun(golden_run, conf.golden_run);
    
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
        simulator.restore(conf.state_folder.c_str());
        log << " ... EIP = " << std::hex << simulator.getCPU(0).getInstructionPointer() << std::endl;

        simulator.addListener(bp);

        simtime_t now = simulator.getTimerTicks();
        fail::BaseListener *go = waitIOOrOther(true);
        assert(go == bp);

        log << "Hit BP @ " << hex << bp->getTriggerInstructionPointer() << " " << bp->getWatchInstructionPointer()
	        << " Start time " << now << ", new time " << simulator.getTimerTicks()
            << ", diff = " << simulator.getTimerTicks() - now << std::endl;

#if L4SYS_FILTER_INSTRUCTIONS
        assert(bp->getTriggerInstructionPointer() == bp->getWatchInstructionPointer());
#endif
        result->set_injection_ip(bp->getTriggerInstructionPointer());

        if (exp_type == param->msg.MEM) {
            result->set_bit_offset(bit);
            log << "injection addr: "
	            << std::hex << param->msg.fsppilot().data_address()
	            << std::endl;
            result->set_injection_address(param->msg.fsppilot().data_address());
            if (!doMemoryInjection(param->msg.fsppilot().data_address(), bit))
			{
				terminateWithError("invalid mem access", 51, result);
			}
        } else if (exp_type == param->msg.GPRFLIP) {
            int reg = (param->msg.fsppilot().data_address() >> 4) + 1;
            result->set_register_offset(static_cast<L4SysProtoMsg_RegisterType>(reg));
            result->set_bit_offset(bit + 8 * (param->msg.fsppilot().data_address() & 0xF));
            doRegisterInjection(param->msg.fsppilot().data_address(), bit);
        } else {
          log << "doing nothing for experiment type " << exp_type << std::endl;
        }

        BPSingleListener ev_done(conf.func_exit, conf.address_space);
        simulator.addListener(&ev_done);

		// Well-known bailout point -- if we hit L4SYS_BREAK_BLINK, which
		// is the entry of Vga::blink_cursor(), we know that we are in some
		// kind of error handler
		BPSingleListener ev_blink(conf.break_blink);
		simulator.addListener(&ev_blink);
		BPSingleListener ev_longjmp(conf.break_longjmp);
		simulator.addListener(&ev_longjmp);

		//If we come to our own exit function, we can stop 
		BPSingleListener ev_exit(conf.break_exit);
		simulator.addListener(&ev_exit);

        unsigned instr_left = conf.totinstr - instr_offset; // XXX offset is in NUMINSTR, TOTINSTR is higher
        BPSingleListener ev_incomplete(ANY_ADDR, conf.address_space);
        /*
         * Use hard-coded value for incomplete counter. We are currently looking at short-running pieces
         * of code. This means that in the error case, where a lot of data is still to be printed to serial
         * line, the benchmark does not complete this within <short-time> * <1.x> cycles. Instead, we use
         * a frame large enough to catch some more output even at the end of a run.
         */
        ev_incomplete.setCounter(2000000);
        simulator.addListener(&ev_incomplete);

		/*
		 * This timeout will always be at least one second - see calculateTimeout()
		 */
        TimerListener ev_timeout(calculateTimeout(instr_left, conf.emul_ips));
        simulator.addListener(&ev_timeout);
        log << "continue... (" <<  simulator.getListenerCount()
            << " breakpoints, timeout @ " << ev_timeout.getTimeout()
            << std::endl;

        log << "TOListener " << (void*)&ev_timeout << " incompListener "
	        << (void*)&ev_incomplete << endl;
        BaseListener *ev = afterInjection(result);
        log << "afterInj: res.devstep = " << result->deviate_steps() << endl;

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
        } else if ((ev == &ev_incomplete) || 
                   (ev == &ev_blink)      ||
                   (ev == &ev_longjmp)) {
            log << "Result INCOMPLETE" << endl;
            result->set_resulttype(param->msg.INCOMPLETE);
            result->set_resultdata(simulator.getCPU(0).getInstructionPointer());
            result->set_output(sanitised(currentOutput.c_str()));
        } else if (ev == &ev_timeout) {
            log << "Result TIMEOUT" << endl;
            result->set_resulttype(param->msg.TIMEOUT);
            result->set_resultdata(simulator.getCPU(0).getInstructionPointer());
            result->set_output(sanitised(currentOutput.c_str()));
        } else if (ev == &ev_exit) {
            log << "Result FAILSTOP" << endl;
            result->set_resulttype(param->msg.FAILSTOP);
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
}



void L4SysExperiment::parseOptions(L4SysConfig &conf) {
	CommandLine &cmd = CommandLine::Inst();

	cmd.addOption("", "", Arg::None, "USAGE: fail-client -Wf,[option] -Wf,[option] ... <BochsOptions...>");
	CommandLine::option_handle HELP =
		cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");
	CommandLine::option_handle STEP =
		cmd.addOption("s", "step", Arg::Optional, "-s,--step \tSpecify preparation step, without this argumnt fail-client start in experiment mode (cr3: get CR3, cc: Create Checkpoint, it: collect instruction trace, gr: golden run, all: do the whole preparation)");


	CommandLine::option_handle OPT_MAX_INSTR_BYTES = 
		cmd.addOption("", "max_instr_bytes", Arg::Optional, "--max_instr_bytes \t define MAX_INSTR_BYTES");
	CommandLine::option_handle OPT_ADDRESS_SPACE = 
		cmd.addOption("", "address_space", Arg::Optional, "--address_space \t define L4SYS_ADDRESS_SPACE");
	CommandLine::option_handle OPT_ADDRESS_SPACE_TRACE = 
		cmd.addOption("", "address_space_trace", Arg::Optional, "--address_space_trace \t define L4SYS_ADDRESS_SPACE_TRACE");
	CommandLine::option_handle OPT_FUNC_ENTRY = 
		cmd.addOption("", "func_entry", Arg::Optional, "--func_entry \t define L4SYS_FUNC_ENTRY");
	CommandLine::option_handle OPT_FUNC_EXIT = 
		cmd.addOption("", "func_exit", Arg::Optional, "--func_exit \t define L4SYS_FUNC_EXIT");
	CommandLine::option_handle OPT_FILTER_ENTRY = 
		cmd.addOption("", "filter_entry", Arg::Optional, "--filter_entry \t define L4SYS_FILTER_ENTRY");
	CommandLine::option_handle OPT_FILTER_EXIT = 
		cmd.addOption("", "filter_exit", Arg::Optional, "--filter_exit \t define L4SYS_FILTER_EXIT");
	CommandLine::option_handle OPT_BREAK_BLINK = 
		cmd.addOption("", "break_blink", Arg::Optional, "--break_blink \t define L4SYS_BREAK_BLINK");
	CommandLine::option_handle OPT_BREAK_LONGJMP = 
		cmd.addOption("", "break_longjmp", Arg::Optional, "--break_longjmp \t define L4SYS_BREAK_LONGJMP");
	CommandLine::option_handle OPT_BREAK_EXIT = 
		cmd.addOption("", "break_exit", Arg::Optional, "--break_exit \t define L4SYS_BREAK_EXIT");
	CommandLine::option_handle OPT_FILTER_INSTRUCTIONS = 
		cmd.addOption("", "filter_instructions", Arg::Optional, "--filter_instructions \t define L4SYS_FILTER_INSTRUCTIONS");
	CommandLine::option_handle OPT_NUMINSTR = 
		cmd.addOption("", "numinstr", Arg::Optional, "--numinstr \t define L4SYS_NUMINSTR");
	CommandLine::option_handle OPT_TOTINSTR = 
		cmd.addOption("", "totinstr", Arg::Optional, "--totinstr \t define L4SYS_TOTINSTR");
	CommandLine::option_handle OPT_EMUL_IPS = 
		cmd.addOption("", "bochs_ips", Arg::Optional, "--bochs_ips \t define L4SYS_BOCHS_IPS");
	CommandLine::option_handle OPT_STATE_FOLDER = 
		cmd.addOption("", "state_folder", Arg::Optional, "--state_folder \t define L4SYS_STATE_FOLDER");
	CommandLine::option_handle OPT_INSTRUCTION_LIST = 
		cmd.addOption("", "instruction_list", Arg::Optional, "--instruction_list \t define L4SYS_INSTRUCTION_LIST");
	CommandLine::option_handle OPT_ALU_INSTRUCTIONS = 
		cmd.addOption("", "alu_instructions", Arg::Optional, "--alu_instructions \t define L4SYS_ALU_INSTRUCTIONS");
	CommandLine::option_handle OPT_CORRECT_OUTPUT = 
		cmd.addOption("", "golden_run", Arg::Optional, "--correct_output \t define L4SYS_CORRECT_OUTPUT");
	CommandLine::option_handle OPT_FILTER = 
		cmd.addOption("", "filter", Arg::Optional, "--filter \t define L4SYS_FILTER");
	CommandLine::option_handle OPT_TRACE = 
		cmd.addOption("", "trace", Arg::Optional, "--trace \t define outputfile for trace (default trace.pb)");
	
	if (!cmd.parse()) { 
		cerr << "Error parsing arguments." << endl;
		simulator.terminate(1);
	} else if (cmd[HELP]) {
		cmd.printUsage();
		simulator.terminate(0);
	}

	if (cmd[OPT_MAX_INSTR_BYTES]) {
		 conf.max_instr_bytes = strtol(cmd[OPT_MAX_INSTR_BYTES].arg, NULL, 16);
	} else {
		 conf.max_instr_bytes = MAX_INSTR_BYTES;
	}

	if (cmd[OPT_ADDRESS_SPACE]) {
		 conf.address_space = strtol(cmd[OPT_ADDRESS_SPACE].arg, NULL, 16);
	} else {
		 conf.address_space = L4SYS_ADDRESS_SPACE;
	}

	if (cmd[OPT_ADDRESS_SPACE_TRACE]) {
		 conf.address_space_trace = strtol(cmd[OPT_ADDRESS_SPACE_TRACE].arg, NULL, 16);
	} else {
		 conf.address_space_trace = L4SYS_ADDRESS_SPACE_TRACE;
	}

	if (cmd[OPT_FUNC_ENTRY]) {
		 conf.func_entry = strtol(cmd[OPT_FUNC_ENTRY].arg, NULL, 16);
	} else {
		 conf.func_entry = L4SYS_FUNC_ENTRY;
	}

	if (cmd[OPT_FUNC_EXIT]) {
		 conf.func_exit = strtol(cmd[OPT_FUNC_EXIT].arg, NULL, 16);
	} else {
		 conf.func_exit = L4SYS_FUNC_EXIT;
	}

	if (cmd[OPT_FILTER_ENTRY]) {
		 conf.filter_entry = strtol(cmd[OPT_FILTER_ENTRY].arg, NULL, 16);
	} else {
		 conf.filter_entry = L4SYS_FILTER_ENTRY;
	}

	if (cmd[OPT_FILTER_EXIT]) {
	 conf.filter_exit = strtol(cmd[OPT_FILTER_EXIT].arg, NULL, 16);
	} else {
	 conf.filter_exit = L4SYS_FILTER_EXIT;
	}

	if (cmd[OPT_BREAK_BLINK]) {
	 conf.break_blink = strtol(cmd[OPT_BREAK_BLINK].arg, NULL, 16);
	} else {
	 conf.break_blink = L4SYS_BREAK_BLINK;
	}

	if (cmd[OPT_BREAK_LONGJMP]) {
	 conf.break_longjmp = strtol(cmd[OPT_BREAK_LONGJMP].arg, NULL, 16);
	} else {
	 conf.break_longjmp = L4SYS_BREAK_LONGJMP;
	}

	if (cmd[OPT_BREAK_EXIT]) {
	 conf.break_exit = strtol(cmd[OPT_BREAK_EXIT].arg, NULL, 16);
	} else {
	 conf.break_exit = L4SYS_BREAK_EXIT;
	}

	if (cmd[OPT_FILTER_INSTRUCTIONS]) {
	 conf.filter_instructions = strtol(cmd[OPT_FILTER_INSTRUCTIONS].arg, NULL, 16);
	} else {
	 conf.filter_instructions = L4SYS_FILTER_INSTRUCTIONS;
	}

	if (cmd[OPT_NUMINSTR]) {
	 conf.numinstr = strtol(cmd[OPT_NUMINSTR].arg, NULL, 16);
	} else {
	 conf.numinstr = L4SYS_NUMINSTR;
	}

	if (cmd[OPT_TOTINSTR]) {
	 conf.totinstr = strtol(cmd[OPT_TOTINSTR].arg, NULL, 16);
	} else {
	 conf.totinstr = L4SYS_TOTINSTR;
	}

	if (cmd[OPT_EMUL_IPS]) {
	 conf.emul_ips = strtol(cmd[OPT_EMUL_IPS].arg, NULL, 16);
	} else {
	 conf.emul_ips = L4SYS_BOCHS_IPS;
	}

	if (cmd[OPT_STATE_FOLDER]) {
	 conf.state_folder = std::string(cmd[OPT_STATE_FOLDER].arg);
	} else {
	 conf.state_folder = L4SYS_STATE_FOLDER;
	}

	if (cmd[OPT_INSTRUCTION_LIST]) {
	 conf.instruction_list = std::string(cmd[OPT_INSTRUCTION_LIST].arg);
	} else {
	 conf.instruction_list = L4SYS_INSTRUCTION_LIST;
	}

	if (cmd[OPT_ALU_INSTRUCTIONS]) {
		 conf.alu_instructions = std::string(cmd[OPT_ALU_INSTRUCTIONS].arg);
	} else {
		 conf.alu_instructions = L4SYS_ALU_INSTRUCTIONS;
	}

	if (cmd[OPT_CORRECT_OUTPUT]) {
		 conf.golden_run = std::string(cmd[OPT_CORRECT_OUTPUT].arg);
	} else {
		 conf.golden_run = L4SYS_CORRECT_OUTPUT;
	}

	if (cmd[OPT_FILTER]) {
		 conf.filter = std::string(cmd[OPT_FILTER].arg);
	} else {
		 conf.filter = L4SYS_FILTER;
	}

	if (cmd[OPT_TRACE]) {
		 conf.trace = std::string(cmd[OPT_TRACE].arg);
	} else {
		 conf.trace = std::string("trace.pb");
	}

	if (cmd[STEP]) {
		if (!std::string("cr3").compare(cmd[STEP].arg)  ) {
			log << "calculate cr3" << endl;
			conf.step = L4SysConfig::GET_CR3;
		} else if (!std::string("cc").compare(cmd[STEP].arg) ) { 
			log << "Create Checkpoint" << endl;
			conf.step = L4SysConfig::CREATE_CHECKPOINT;
		} else if (!std::string("it").compare(cmd[STEP].arg) ) { 
			log << "collect instruction trace" << endl;
			conf.step = L4SysConfig::COLLECT_INSTR_TRACE;
		} else if (!std::string("gr").compare(cmd[STEP].arg) ) {  
			log << "golden run" << endl;
			conf.step = L4SysConfig::GOLDEN_RUN;
		} else if (!std::string("all").compare(cmd[STEP].arg) ) {  
			log << "do all preparation steps" << endl;
			conf.step = L4SysConfig::FULL_PREPARATION;
		} else {
			cerr << "Wrong argument for option '--step'" << endl;
			simulator.terminate(1);
		}
	} else {
		conf.step = L4SysConfig::NO_PREP;
	}

}

bool L4SysExperiment::run()
{
	srand(time(NULL));

	log << "Starting L4Sys Experiment, phase " << PREPARATION_STEP << endl;

	parseOptions(conf);

	switch(conf.step) {
		case L4SysConfig::GET_CR3: {
			log << "CR_3Run: Watching for instruction " << hex << conf.func_entry << endl;
    	runToStart(new BPSingleListener(0));
			log << "CR3 = " << hex << conf.address_space << endl;
			break;
		}
		case L4SysConfig::CREATE_CHECKPOINT: {
			// STEP 1: run until interesting function starts, and save state
			// -> needs L4SYS_ADDRESS_SPACE, because it runs until L4SYS_FUNC_ENTRY
    	runToStart(new BPSingleListener(0, conf.address_space));
			simulator.save(conf.state_folder);
			break;
		}
		case L4SysConfig::COLLECT_INSTR_TRACE: {
			// STEP 2: determine instructions executed
	    collectInstructionTrace(new BPSingleListener(0, ANY_ADDR));
			break;
		}
		case L4SysConfig::GOLDEN_RUN: {
			// STEP 3: determine the output of a "golden run"
			// -> golden run needs L4SYS_ADDRESS_SPACE as it breaks on
			//    L4SYS_FUNC_EXIT
   		goldenRun(new BPSingleListener(0, L4SYS_ADDRESS_SPACE));
			break;
		}
		case L4SysConfig::FULL_PREPARATION: {
    	runToStart(new BPSingleListener(0));
			simulator.save(conf.state_folder);
      simulator.clearListeners();
	    collectInstructionTrace(new BPSingleListener(0, ANY_ADDR));
      simulator.clearListeners();
   		goldenRun(new BPSingleListener(0, L4SYS_ADDRESS_SPACE));
			break;
		}
		default: {
			BPSingleListener *bp = 0;
			doExperiments(bp);
		}
	}


  terminate(0);
	// experiment successfully conducted
	return true;
}
