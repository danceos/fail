#include <fstream>
#include <iostream>

#include "experiment.hpp"
#include "InstructionFilter.hpp"
#include "UDIS86.hpp"
#include "aluinstr.hpp"
#include "campaign.hpp"

#include <sal/bochs/BochsMemory.hpp>
#include "sal/SALConfig.hpp"

using namespace std;
using namespace fail;

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

unsigned L4SysExperiment::calculateTimeout(unsigned instr_left, unsigned ips) {
	// the timeout in seconds, plus one backup second (avoids rounding overhead)
	// [instr] / [instr / s] = [s]
	unsigned seconds = instr_left / ips + 1;
	// 1.1 (+10 percent) * 1000000 mus/s * [s]
	return 1100000 * seconds;
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

    setupFilteredBreakpoint(bp, offset, conf.instruction_list);
    assert(bp->getWatchInstructionPointer() == (address_t)(ip & 0xFFFFFFFF));
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

    setupFilteredBreakpoint(bp, offset, conf.instruction_list);
    log << bp->getWatchInstructionPointer() << std::endl;
    log << ip << std::endl;
    assert(bp->getWatchInstructionPointer() == (address_t)(ip & 0xFFFFFFFF));
    log << bp->getCounter() << std::endl;
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

        assert(bp->getTriggerInstructionPointer() == bp->getWatchInstructionPointer());
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

