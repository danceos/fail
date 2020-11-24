#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

#include <stdlib.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "sal/SALConfig.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include "efw/DatabaseExperiment.hpp"
#include "comm/DatabaseCampaignMessage.pb.h"

#if defined(BUILD_CAPSTONE_DISASSEMBLER)
#  include "util/capstonedisassembler/CapstoneToFailTranslator.hpp"
#elif defined(BUILD_LLVM_DISASSEMBLER)
#  include "util/llvmdisassembler/LLVMtoFailTranslator.hpp"
#endif

//#define LOCAL

using namespace std;
using namespace fail;
using namespace google::protobuf;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE)
     #error This experiment needs: breakpoints, restore. Enable these in the configuration.
#endif

DatabaseExperiment::~DatabaseExperiment()  {
	delete this->m_jc;
}

void DatabaseExperiment::redecodeCurrentInstruction() {
	/* Flush Instruction Caches and Prefetch queue */
	BX_CPU_C *cpu_context = simulator.getCPUContext();
	cpu_context->invalidate_prefetch_q();
	cpu_context->iCache.flushICacheEntries();

	guest_address_t pc = simulator.getCPU(0).getInstructionPointer();
	bxInstruction_c *currInstr = simulator.getCurrentInstruction();

	m_log << "REDECODE INSTRUCTION @ IP 0x" << std::hex << pc << endl;

	guest_address_t eipBiased = pc + cpu_context->eipPageBias;
	Bit8u instr_plain[15];

	MemoryManager& mm = simulator.getMemoryManager();
	for (unsigned i = 0; i < sizeof(instr_plain); ++i) {
		if (!mm.isMapped(pc + i)) {
			m_log << "REDECODE: 0x" << std::hex << pc+i << "UNMAPPED" << endl;
			// TODO: error?
			return;
		}
	}

	mm.getBytes(pc, sizeof(instr_plain), instr_plain);

	guest_address_t remainingInPage = cpu_context->eipPageWindowSize - eipBiased;
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

unsigned DatabaseExperiment::injectFault(
	address_t data_address, unsigned bitpos, bool inject_burst,
	bool inject_registers, bool force_registers, bool randomjump) {
	unsigned value, injected_value;

	if (randomjump) {
		// interpret data_address as new value for the IP, i.e. jump there
		address_t current_PC = simulator.getCPU(0).getInstructionPointer();
		address_t new_PC     = data_address;
		m_log << "jump from 0x" << hex << current_PC << " to 0x" << new_PC << std::endl;
		simulator.getCPU(0).setRegisterContent(simulator.getCPU(0).getRegister(RID_PC), new_PC);
		redecodeCurrentInstruction();

		// set program counter
		value = current_PC;
		injected_value = new_PC;

	/* First 128 registers, TODO use LLVMtoFailTranslator::getMaxDataAddress() */
	} else if (data_address < (128 << 4) && inject_registers) {
#if defined(BUILD_LLVM_DISASSEMBLER) || defined(BUILD_CAPSTONE_DISASSEMBLER)
#if defined(BUILD_LLVM_DISASSEMBLER)
		typedef LLVMtoFailTranslator XtoFailTranslator;
#elif defined(BUILD_CAPSTONE_DISASSEMBLER)
		typedef CapstoneToFailTranslator XtoFailTranslator;
#endif
		// register FI
		XtoFailTranslator::reginfo_t reginfo =
			XtoFailTranslator::reginfo_t::fromDataAddress(data_address, 1);

		value = XtoFailTranslator::getRegisterContent(simulator.getCPU(0), reginfo);
		if (inject_burst) {
			injected_value = value ^ 0xff;
			m_log << "INJECTING BURST at: REGISTER " << dec << reginfo.id
				<< " bitpos " << (reginfo.offset + bitpos) << endl;
		} else {
			injected_value = value ^ (1 << bitpos);
			m_log << "INJECTING BIT-FLIP at: REGISTER " << dec << reginfo.id
				<< " bitpos " << (reginfo.offset + bitpos) << endl;
		}
		XtoFailTranslator::setRegisterContent(simulator.getCPU(0), reginfo, injected_value);
		if (reginfo.id == RID_PC) {
			// FIXME move this into the Bochs backend
			m_log << "Redecode current instruction" << endl;
			redecodeCurrentInstruction();
		}
#else
		m_log << "ERROR: Not compiled with LLVM or Capstone.  Enable BUILD_LLVM_DISASSEMBLER OR BUILD_CAPSTONE_DISASSEMBLER at buildtime." << endl;
		simulator.terminate(1);
#endif
	} else if (!force_registers) {
		// memory FI
		value = m_mm.getByte(data_address);

		if (inject_burst) {
			injected_value = value ^ 0xff;
			m_log << "INJECTING BURST at: MEM 0x"
				<< hex << setw(2) << setfill('0') << data_address << endl;
		} else {
			injected_value = value ^ (1 << bitpos);
			m_log << "INJECTING BIT-FLIP (" << dec << bitpos << ") at: MEM 0x"
				<< hex << setw(2) << setfill('0') << data_address << endl;
		}
		m_mm.setByte(data_address, injected_value);

	} else {
		m_log << "WARNING: Skipping injection, data address 0x"
			<< hex << data_address << " out of range." << endl;
		return 0;
	}
	m_log << hex << setw(2) << setfill('0')
		<< " value: 0x" << value
		<<     " -> 0x" << injected_value << endl;
	return value;
}

template<class T>
T * protobufFindSubmessageByTypename(Message *msg, const std::string &name) {
	T * submessage = 0;
	const Descriptor *msg_type = msg->GetDescriptor();
	const Reflection *ref = msg->GetReflection();
	const Descriptor *database_desc =
		DescriptorPool::generated_pool()->FindMessageTypeByName(name);
	assert(database_desc != 0);

	size_t count = msg_type->field_count();

	for (unsigned i = 0; i < count; i++) {
		const FieldDescriptor *field = msg_type->field(i);
		assert(field != 0);
		if (field->message_type() == database_desc) {
			submessage = dynamic_cast<T*>(ref->MutableMessage(msg, field));
			assert(submessage != 0);
			break;
		}
	}
	return submessage;
}


bool DatabaseExperiment::run()
{
    m_log << "STARTING EXPERIMENT" << endl;

	if (!this->cb_start_experiment()) {
		m_log << "Initialization failed. Exiting." << endl;
		simulator.terminate(1);
	}

	unsigned executed_jobs = 0;

	while (executed_jobs < 25 || m_jc->getNumberOfUndoneJobs() > 0) {
		m_log << "asking jobserver for parameters" << endl;
		ExperimentData * param = this->cb_allocate_experiment_data();
#ifndef LOCAL
		if (!m_jc->getParam(*param)){
			m_log << "Dying." << endl; // We were told to die.
			simulator.terminate(1);
		}
#endif
		m_current_param = param;

		DatabaseCampaignMessage * fsppilot =
			protobufFindSubmessageByTypename<DatabaseCampaignMessage>(&param->getMessage(), "DatabaseCampaignMessage");
		assert (fsppilot != 0);

#ifdef LOCAL
		fsppilot->set_injection_instr(0);
		fsppilot->set_injection_instr_absolute(1048677);
		fsppilot->set_data_address(2101240);
		fsppilot->set_data_width(1);
		fsppilot->set_inject_bursts(true);
#endif

		unsigned  injection_instr = fsppilot->injection_instr();
		address_t data_address = fsppilot->data_address();
		unsigned width = fsppilot->data_width();
		unsigned injection_width =
			(fsppilot->inject_bursts() || fsppilot->register_injection_mode() == fsppilot->RANDOMJUMP) ? 8 : 1;

		for (unsigned bit_offset = 0; bit_offset < width * 8; bit_offset += injection_width) {
			// 8 results in one job
			Message *outer_result = cb_new_result(param);
			m_current_result = outer_result;
			DatabaseExperimentMessage *result =
				protobufFindSubmessageByTypename<DatabaseExperimentMessage>(outer_result, "DatabaseExperimentMessage");
			result->set_bitoffset(bit_offset);
			m_log << "restoring state" << endl;
			// Restore to the image, which starts at address(main)
			simulator.restore(cb_state_directory());
			executed_jobs ++;

			m_log << "Trying to inject @ instr #" << dec << injection_instr << endl;

			simulator.clearListeners(this);

			if (!this->cb_before_fast_forward()) {
				continue;
			}

			// Do we need to fast-forward at all?
			fail::BaseListener *listener = 0;
			if (injection_instr > 0) {
				// Create a listener that matches any IP event. It is used to
				// forward to the injection point.
				BPSingleListener bp;
				bp.setWatchInstructionPointer(ANY_ADDR);
				bp.setCounter(injection_instr);
				simulator.addListener(&bp);

				while (true) {
					listener = simulator.resume();
					if (listener == &bp) {
						break;
					} else {
						bool should_continue = this->cb_during_fast_forward(listener);
						if (!should_continue)
							break; // Stop fast forwarding
					}
				}
			}
			if (!this->cb_after_fast_forward(listener)) {
				continue; // Continue to next injection experiment
			}

			address_t injection_instr_absolute = fsppilot->injection_instr_absolute();
			bool found_eip = false;
			for (size_t i = 0; i < simulator.getCPUCount(); i++) {
				address_t eip = simulator.getCPU(i).getInstructionPointer();
				if (eip == injection_instr_absolute) {
					found_eip = true;
				}
			}
			if (fsppilot->has_injection_instr_absolute() && !found_eip) {
				m_log << "Invalid Injection address  != 0x" << std::hex << injection_instr_absolute<< std::endl;
				for (size_t i = 0; i < simulator.getCPUCount(); i++) {
					address_t eip = simulator.getCPU(i).getInstructionPointer();
					m_log << " CPU " << i << " EIP = 0x" << std::hex << eip << std::dec << std::endl;
				}
				simulator.terminate(1);
			}

			simulator.clearListeners(this);

			// inject fault (single-bit flip or burst)
			result->set_original_value(
				injectFault(data_address + bit_offset / 8, bit_offset % 8,
					fsppilot->inject_bursts(),
					fsppilot->register_injection_mode() != fsppilot->OFF,
					fsppilot->register_injection_mode() == fsppilot->FORCE,
					fsppilot->register_injection_mode() == fsppilot->RANDOMJUMP));
			result->set_injection_width(injection_width);

			if (!this->cb_before_resume()) {
				continue; // Continue to next experiment
			}

			m_log << "Resuming till the crash" << std::endl;
			// resume and wait for results
			while (true) {
				listener = simulator.resume();
				bool should_continue = this->cb_during_resume(listener);
				if (!should_continue)
					break;
			}
			m_log << "Resume done" << std::endl;
			this->cb_after_resume(listener);

			simulator.clearListeners(this);
		}
#ifndef LOCAL
		m_jc->sendResult(*param);
#else
		break;
#endif
		this->cb_free_experiment_data(param);
	}
	// Explicitly terminate, or the simulator will continue to run.
	simulator.terminate();
	return false;
}


