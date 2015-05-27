#include <iostream>
#include <sstream>
#include <fstream>

#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include "experiment.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"

#include "sal/bochs/BochsListener.hpp"
#include "sal/bochs/BochsMemory.hpp"
#include "sal/bochs/BochsCPU.hpp"
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

#include "util/llvmdisassembler/LLVMtoFailTranslator.hpp"
#include "util/llvmdisassembler/LLVMtoFailBochs.hpp"

#include "campaign.hpp"
#include "erika-tester.pb.h"

#include "../plugins/randomgenerator/RandomGenerator.hpp"
#include "../plugins/checkpoint/Checkpoint.hpp"

using namespace std;
using namespace fail;

#define SAFESTATE (1)

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
	!defined(CONFIG_SR_SAVE)
#error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif
#if  !defined(CONFIG_EVENT_MEMREAD) || !defined(CONFIG_EVENT_MEMWRITE)
#error This experiment needs: MemRead and MemWrite. Enable these in the configuration.
#endif

void ErikaTester::redecodeCurrentInstruction() {
	/* Flush Instruction Caches and Prefetch queue */
	BX_CPU_C *cpu_context = simulator.getCPUContext();
	cpu_context->invalidate_prefetch_q();
	cpu_context->iCache.flushICacheEntries();


	guest_address_t pc = simulator.getCPU(0).getInstructionPointer();
	bxInstruction_c *currInstr = simulator.getCurrentInstruction();
	//unsigned length_in_bytes = currInstr->ilen();

	m_log << "REDECODE INSTRUCTION @ IP 0x" << std::hex << pc << endl;

	Bit32u eipBiased = pc + cpu_context->eipPageBias;
	Bit8u  instr_plain[15];

	MemoryManager& mm = simulator.getMemoryManager();
	for(unsigned i=0; i<sizeof(instr_plain); i++) {
		if(!mm.isMapped(pc+i)) {
			m_log << "REDECODE: 0x" << std::hex << pc+i << "UNMAPPED" << endl;
			// TODO: error?
			return;
		}
	}

	mm.getBytes(pc, sizeof(instr_plain), instr_plain);

	unsigned remainingInPage = cpu_context->eipPageWindowSize - eipBiased;
	int ret;
#if BX_SUPPORT_X86_64
	if (cpu_context->cpu_mode == BX_MODE_LONG_64)
		ret = cpu_context->fetchDecode64(instr_plain, currInstr, remainingInPage);
	else
#endif
		ret = cpu_context->fetchDecode32(instr_plain, currInstr, remainingInPage);
	if (ret < 0) {
		// handle instrumentation callback inside boundaryFetch
		cpu_context->boundaryFetch(instr_plain, remainingInPage, currInstr);
	}
}


unsigned ErikaTester::injectBitFlip(address_t data_address, unsigned data_width, unsigned bitpos) {
	/* First 32 Registers, this might neeed adaption */
	if (data_address < (32 << 8)) {
		LLVMtoFailTranslator::reginfo_t reginfo = LLVMtoFailTranslator::reginfo_t::fromDataAddress(data_address, data_width);

		unsigned int value, injectedval;

		value = m_ltof->getRegisterContent(simulator.getCPU(0), reginfo);
		injectedval = value ^ (1 << bitpos);
		m_ltof->setRegisterContent(simulator.getCPU(0), reginfo, injectedval);

		m_log << "INJECTING register (" << dec << reginfo.id
			<< " offset " << (int) reginfo.offset
			<< ") bitpos: " << bitpos
			<< " value: 0x" << hex << setw(2) << setfill('0') << value << " -> 0x" << setw(2) << setfill('0') << injectedval 
			<< dec << endl;
		if (reginfo.id == RID_PC)
			redecodeCurrentInstruction();

		return value;
	} else {
		unsigned int value, injectedval;

		MemoryManager& mm = simulator.getMemoryManager();
		if(!mm.isMapped(data_address)) {
			m_log << "DATA_ADDRESS NOT MAPPED" << endl;
			return 0;
		}

		value = mm.getByte(data_address);
		injectedval = value ^ (1 << bitpos);
		mm.setByte(data_address, injectedval);

		m_log << "INJECTION at: 0x" << hex	<< setw(2) << setfill('0') << data_address
			<< " value: 0x" << setw(2) << setfill('0') << value << " -> 0x" << setw(2) << setfill('0') << injectedval << endl;

		/* If it is the current instruction redecode it */
		guest_address_t pc = simulator.getCPU(0).getInstructionPointer();
		m_log << "IP: 0x" << std::hex << pc << std::endl;
		bxInstruction_c *currInstr = simulator.getCurrentInstruction();
		assert(currInstr != NULL && "FATAL ERROR: current instruction was NULL (not expected)!");
		unsigned length_in_bytes = currInstr->ilen();

		if (currInstr == NULL || (pc <= data_address && data_address <= (pc + currInstr->ilen()))) {
			redecodeCurrentInstruction();
		}

		return value;
	}
}


void handleEvent(ErikaTesterProtoMsg_Result& result, ErikaTesterProtoMsg_Result_ResultType restype, const std::string &msg) {
	cout << msg << endl;
	result.set_resulttype(restype);
	result.set_details(msg);
}


std::string handleMemoryAccessEvent(fail::MemAccessListener* l_mem) {
	stringstream sstr;

	sstr << "mem access (";
	switch (l_mem->getTriggerAccessType()) {
		case MemAccessEvent::MEM_READ:
			sstr << "r";
			break;
		case MemAccessEvent::MEM_WRITE:
			sstr << "w";
			break;
		default:
			break;
	}
	sstr << ") @ 0x" << hex << l_mem->getTriggerAddress();
	sstr << "; ip @ 0x" << hex << l_mem->getTriggerInstructionPointer();

	return sstr.str();
}


const ElfSymbol& ErikaTester::getELFSymbol(const std::string name) {
	const ElfSymbol &symbol = m_elf.getSymbol(name);
	if (!symbol.isValid()) {
		m_log << "Couldn't find symbol: " << name << std::endl;
		simulator.terminate(1);
	}

	return symbol;
}


bool ErikaTester::run() {
	m_log << "STARTING EXPERIMENT" << endl;

	// seed random number generator
	timeval start;
	gettimeofday(&start, NULL);
	srand(start.tv_usec);

	// get symbols
	const ElfSymbol &s_trace_end_marker = getELFSymbol("test_finish");
	BPSingleListener l_trace_end_marker(s_trace_end_marker.getAddress());

	const ElfSymbol &s_stext_app = getELFSymbol("_stext_application");
	const ElfSymbol &s_etext_app = getELFSymbol("_etext_application");

	const ElfSymbol &s_panic_handler = m_elf.getSymbol("panic_handler");
	assert (s_panic_handler.isValid());

	const ElfSymbol &s_fail_trace = getELFSymbol("fail_trace");
	MemAccessListener l_fail_trace(s_fail_trace.getAddress());

	const ElfSymbol &s_random_source = m_elf.getSymbol("random_source");
	const ElfSymbol &s_tos = getELFSymbol("EE_x86_system_tos");


	// allowed regions
	address_t text_tasks_start = s_stext_app.getAddress();
	address_t text_tasks_end = s_etext_app.getAddress();

	m_log << "not injecting in application: " << std::hex << text_tasks_start << " -- " << text_tasks_end << std::endl;

	// listeners for traps, code region
	//InterruptListener l_interrupt(2);  // NMI interrupt
	//TrapListener l_trap(2);  // NMI trap?
	BPSingleListener* l_panic;
	if(s_panic_handler.isValid()) {
		l_panic = new BPSingleListener(s_panic_handler.getAddress());
		m_log << "PANIC handler @ " << std::hex << s_panic_handler.getAddress() << std::endl;
	}

	unsigned i_timeout = 2000 * 1000;
	TimerListener l_timeout(i_timeout); // 1s in microseconds
	TimerListener l_timeout_hard(2 * 1000 * 1000); // 2s in microseconds


	// initialize LLVM disassembler
	m_ltof = LLVMtoFailTranslator::createFromBinary(m_elf.getFilename());

	// memory manager
	MemoryManager& mm = simulator.getMemoryManager();

	// job client with environment parameters
	char* server = getenv("FAIL_SERVER_HOST");
	if(server == NULL) server = (char*) SERVER_COMM_HOSTNAME;
	char* cport = getenv("FAIL_SERVER_PORT");
	int port = (cport != NULL) ? atoi(cport) : SERVER_COMM_TCP_PORT;
	fail::JobClient jobclient(server, port);

	// execute jobs
	unsigned executed_jobs = 0;
	while (executed_jobs < 25 || jobclient.getNumberOfUndoneJobs() > 0) {
		// get next parameters from jobserver
		m_log << "asking jobserver for parameters" << endl;
		ErikaTesterExperimentData param;
		if(!jobclient.getParam(param)){
			m_log << "Dying." << endl; // We were told to die.
			simulator.terminate(128); // special return code to signal campaign finish
		}

		// extract parameters
		unsigned  injection_instr = param.msg.fsppilot().injection_instr();
		address_t data_address = param.msg.fsppilot().data_address();
		unsigned data_width = param.msg.fsppilot().data_width();

		// detect wheter we should inject the PC
		bool pc_injection = param.msg.fsppilot().benchmark().find("jump") != std::string::npos;

		// setup experiments
		int experiments = pc_injection ? 1 : 8;

		// run experiments
		for (int experiment_id = 0; experiment_id < experiments; ++experiment_id) {
			ErikaTesterProtoMsg_Result *result = 0;

			// restore to the image
			m_log << "restoring state" << endl;
			simulator.clearListeners(this);
			simulator.restore("state");
			m_log << "state restored" << endl;
			executed_jobs++;

			// init plugins
			Checkpoint::range_vector check_ranges;

			// The idle stack
			std::map<fail::address_t, fail::address_t> stackpointers;
			const ElfSymbol &s_os_stack = m_elf.getSymbol("os_stack");
			assert (s_os_stack.isValid());
			m_log << "found task stack: " << "os_stack" << std::endl;
			fail::address_t paddr;
			paddr = s_tos.getAddress() + 0*4;
			Checkpoint::indirectable_address_t start = std::make_pair(paddr, true);
			Checkpoint::indirectable_address_t end = std::make_pair(s_os_stack.getEnd(), false);
			check_ranges.push_back(std::make_pair(start, end));
			stackpointers[paddr+0] = paddr;
			stackpointers[paddr+1] = paddr;
			stackpointers[paddr+2] = paddr;
			stackpointers[paddr+3] = paddr;


			for(int i=1; i<255; i++) {
				stringstream stackname;
				stackname << "EE_x86_stack_";
				stackname << i;
				const ElfSymbol &s_stack = m_elf.getSymbol(stackname.str());
				if(!s_stack.isValid()) {
					m_log << "found " << std::dec << (i-1) << " task stacks" << std::endl;
					break;
				}

				paddr = s_tos.getAddress() + i*4;
				Checkpoint::indirectable_address_t start = std::make_pair(paddr, true);
				Checkpoint::indirectable_address_t end = std::make_pair(s_stack.getEnd(), false);
				check_ranges.push_back(std::make_pair(start, end));

				stackpointers[paddr+0] = paddr;
				stackpointers[paddr+1] = paddr;
				stackpointers[paddr+2] = paddr;
				stackpointers[paddr+3] = paddr;
			}


			Checkpoint cpoint(check_ranges, "checkpoint.trace");

			RandomGenerator* rgen;
    		if (s_random_source.isValid()) {
				const unsigned seed = 12342;
			    rgen = new RandomGenerator(s_random_source, seed);
			    simulator.addFlow(rgen);
		    }

			// fast forward to injection address
			m_log << "Trying to inject @ instr #" << dec << injection_instr << endl;
			simulator.clearListeners(this);
			simulator.addListener(&l_fail_trace);
			if (injection_instr != 0) {

				BPSingleListener bp;
				bp.setWatchInstructionPointer(ANY_ADDR);
				// TODO: why does this need a +1?
				bp.setCounter(injection_instr);
				simulator.addListener(&bp);

				fail::BaseListener * listener = simulator.resume();
				bool ok = true;

				while ( ok && (listener == &l_fail_trace) ) {
					m_log << "CP IP 0x" << std::hex << simulator.getCPU(0).getInstructionPointer() << std::endl;
					ok = cpoint.check(s_fail_trace, l_fail_trace.getTriggerInstructionPointer()) == Checkpoint::IDENTICAL;
					if(ok) listener = simulator.addListenerAndResume(&l_fail_trace);
				}
				if(!ok) {
					result = param.msg.add_result();
					handleEvent(*result, result->NOINJECTION, "CHECKPOINT FAIL BEFORE INJECTION?!");
					break;
				}
				if (listener != &bp) {
					result = param.msg.add_result();
					handleEvent(*result, result->NOINJECTION, "WTF");
					break;
				}
			}

			// program counter sanity check
			if (param.msg.fsppilot().has_injection_instr_absolute()) {
				address_t PC = param.msg.fsppilot().injection_instr_absolute();
				if (simulator.getCPU(0).getInstructionPointer() != PC) {
					m_log << "Invalid injection address EIP=0x"
						<< std::hex << simulator.getCPU(0).getInstructionPointer()
						<< " != injection_instr_absolute=0x" << PC << std::endl;
					simulator.terminate(1);
				}
			}

			unsigned experiment_number = cpoint.getCount();
			m_log << "Trace " << std::dec << experiment_number << std::endl;
			result = param.msg.add_result();
			result->set_experiment_number((unsigned int) experiment_number);
			result->set_bitoffset(experiment_id);
			result->set_original_value(0);

			// abort if outside targeted region
			address_t PC = simulator.getCPU(0).getInstructionPointer();
			//if(PC < min_code || PC > max_code) {
			if(PC < text_tasks_end && PC >= text_tasks_start) {
				std::stringstream ss;
				ss << "0x" << hex << PC;
				ss << " inside task text: ";
				ss << "0x" << hex << text_tasks_start << " - 0x" << hex << text_tasks_end;
				handleEvent(*result, result->NOINJECTION, ss.str());
				continue; // next experiment
			}

			MemWriteListener *stackpointer_event = 0;
			bool stackpointer_event_valid = false;
			bool check_digest = true;

			// perform injection
			if (pc_injection) {
				// jump to "data" address
				address_t current_PC = simulator.getCPU(0).getInstructionPointer();
				address_t new_PC     = param.msg.fsppilot().data_address();
				m_log << "jump from 0x" << hex  << current_PC << " to 0x" << new_PC << std::endl;
				simulator.getCPU(0).setRegisterContent(simulator.getCPU(0).getRegister(RID_PC), new_PC);

				// set program counter
				result->set_bitoffset(0);
				result->set_original_value(current_PC);
				redecodeCurrentInstruction();
			} else {
				// inject random bitflip
				// experiment_id == bitpos
				// We cache the stackpointer value inside the
				// checkpoint plugin, if we inject it. This prevents
				// the Checkpoint plugin to read invalid values.
				if (stackpointers.find(data_address) != stackpointers.end()) {
					guest_address_t paddr = stackpointers[data_address];
					guest_address_t value = 0;
					simulator.getMemoryManager().getBytes(paddr, 4, &value);
					cpoint.cache_stackpointer_variable(paddr, value);
					stackpointer_event = new MemWriteListener(paddr);
					stackpointer_event_valid = true;
					m_log << "Injected Stackpointer; saved old stackpointer value ("
						  << hex << "0x" << paddr << "="<< value << dec << ")"<< std::endl;
				}

				if (cpoint.in_range(data_address)) {
					m_log << "Injected value is in checksummed range, ignore stack digest results" << endl;
					check_digest = false;
				}

				result->set_original_value(injectBitFlip(data_address, data_width, experiment_id));
			}

			// add listeners
			simulator.clearListeners(this);

			if(s_panic_handler.isValid()) {
				simulator.addListener(l_panic);
			}
			i_timeout = 2000 * 1000;
			l_timeout.setTimeout(i_timeout);
			simulator.addListener(&l_timeout);
			simulator.addListener(&l_timeout_hard);
			BPSingleListener l_timeout_instr;
			l_timeout_instr.setWatchInstructionPointer(ANY_ADDR);
			// Simulate only further 500 000 instructions
			l_timeout_instr.setCounter(500000); 
			simulator.addListener(&l_timeout_instr);

			simulator.addListener(&l_fail_trace);
			simulator.addListener(&l_trace_end_marker);

			if (stackpointer_event_valid) {
				m_log << "Injected Stackpointer; add listener 0x"
					  << hex << stackpointer_event->getWatchAddress() << dec
					  << " " << stackpointer_event->getTriggerAccessType() << endl;
				simulator.addListener(stackpointer_event);
			}

			// resume and wait for results
			m_log << "Resuming till the crash" << std::endl;
			bool reached_check_start = false;
			fail::BaseListener* l = simulator.resume();

			int checkpoint_count = 0;

			while(l == &l_fail_trace || l == stackpointer_event) {
				if (l == stackpointer_event) {
					cpoint.uncache_stackpointer_variable(stackpointer_event->getWatchAddress());
					m_log << "Injected Stackpointer; cleared stackpointer cache" << std::endl;
					l = simulator.resume();
					continue;
				}

				Checkpoint::check_result res = cpoint.check(s_fail_trace, l_fail_trace.getTriggerInstructionPointer());
				checkpoint_count ++;
				if(res == Checkpoint::DIFFERENT_IP) {
					std::stringstream ss;
					ss << "different IP";
					ss << "@ IP 0x" << std::hex << l_fail_trace.getTriggerInstructionPointer();
					ss << " (checkpoint " << std::dec << cpoint.getCount() << ")";
					handleEvent(*result, result->ERR_WRONG_RESULT, ss.str());
					break;
				} else if(res == Checkpoint::DIFFERENT_VALUE) {
					std::stringstream ss;
					ss << "different value";
					ss << "@ IP 0x" << std::hex << l_fail_trace.getTriggerInstructionPointer();
					ss << " (checkpoint " << std::dec << cpoint.getCount() << ")";
					handleEvent(*result, result->ERR_WRONG_RESULT, ss.str());
					break;
				} else if(check_digest && res == Checkpoint::DIFFERENT_DIGEST) {
					std::stringstream ss;
					ss << "different digest";
					ss << "@ IP 0x" << std::hex << l_fail_trace.getTriggerInstructionPointer();
					ss << " (checkpoint " << std::dec << cpoint.getCount() << ")";

					// IGNORE for ISORC'14 experiments
					handleEvent(*result, result->ERR_WRONG_RESULT, ss.str());
					break;
				} else if(res == Checkpoint::INVALID) {
					std::stringstream ss;
					ss << "invalid checkpoint";
					ss << "@ IP 0x" << std::hex << l_fail_trace.getTriggerInstructionPointer();
					ss << " (checkpoint " << std::dec << cpoint.getCount() << ")";
					handleEvent(*result, result->ERR_WRONG_RESULT, ss.str());
					break;
				}
				m_log << "i " << checkpoint_count << " " << simulator.getTimerTicks()  << " " << i_timeout << endl;

				// Reset the soft timeout listener
				simulator.removeListener(&l_timeout);
				if (checkpoint_count > 5) {
					i_timeout = 59 * 1000;
				}
				l_timeout.setTimeout(i_timeout);
				simulator.addListener(&l_timeout);
				assert(l_timeout.getTimeout() == i_timeout);

				l = simulator.addListenerAndResume(&l_fail_trace);
			}

			if(l == &l_fail_trace) {
				// already reported invalid checkpoint
			} else if(l == &l_trace_end_marker) {
				// trace ended successfully
				std::stringstream ss;
				ss << "correct end after " << cpoint.getCount() << " checkpoints";
				handleEvent(*result, result->OK, ss.str());
			} else if (l == l_panic) {
				// error detected
				stringstream sstr;
				sstr << "PANIC";

				// ErikaOS specific trap information
				const Register *reg_eax = simulator.getCPU(0).getRegister(RID_CAX);
				uint32_t trap = simulator.getCPU(0).getRegisterContent(reg_eax);
				sstr << " trap " << dec << trap;

				//address_t sp = simulator.getCPU(0).getStackPointer();
				//if(mm.isMapped(sp)) {
				//	uint32_t ip;
				//	mm.getBytes(sp, 4, &ip);
				//	sstr << " @ 0x" << hex << ip;

				const Register *reg_esi = simulator.getCPU(0).getRegister(RID_CSI);
				address_t sp = simulator.getCPU(0).getRegisterContent(reg_esi) + 4;
				if(mm.isMapped(sp) && mm.isMapped(sp+1) && mm.isMapped(sp+2) && mm.isMapped(sp+3)) {
					uint32_t ip;
					mm.getBytes(sp, 4, &ip);
					sstr << " from 0x" << hex << ip;
				}

				handleEvent(*result, result->OK_DETECTED_ERROR, sstr.str());
			} else if ( l == &l_timeout || l == &l_timeout_hard || l == &l_timeout_instr) {
				if (l == &l_timeout) {
					handleEvent(*result, result->ERR_TIMEOUT, "soft timeout");
				} else if (l == &l_timeout_hard) {
					handleEvent(*result, result->ERR_TIMEOUT, "hard timeout");
				} else if (l == &l_timeout_instr) {
					handleEvent(*result, result->ERR_TIMEOUT, "instr timeout");
				}
			} else {
				// what happened?
				handleEvent(*result, result->UNKNOWN, "WTF");
			}

			if (stackpointer_event_valid) {
				delete stackpointer_event;
			}

			// For RandomJump do only one experiment not 8
			if (pc_injection) break;
		}

		// send results
		jobclient.sendResult(param);
	}

	// explicitly terminate, or the simulator will continue to run
	simulator.terminate();
}

