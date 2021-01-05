#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>

#include <stdlib.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "sal/SALConfig.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include "sal/faultspace/FaultSpace.hpp"
#include "sal/faultspace/RegisterArea.hpp"
#include "sal/faultspace/MemoryArea.hpp"

#include "efw/DatabaseExperiment.hpp"
#include "comm/DatabaseCampaignMessage.pb.h"


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

unsigned DatabaseExperiment::injectFault(DatabaseCampaignMessage * fsppilot, unsigned bitpos) {
	fsp_address_t data_address = fsppilot->data_address();

	// Random Jump Injection
	if (fsppilot->injection_mode() == fsppilot->RANDOMJUMP) {
		// interpret data_address as new value for the IP, i.e. jump there
		ConcreteCPU &cpu = simulator.getCPU(0);
		address_t current_PC = cpu.getInstructionPointer();
		address_t new_PC     = data_address;
		m_log << "INJECT: jump from 0x" << hex << current_PC << " to 0x" << new_PC << std::endl;

		cpu.setRegisterContent(cpu.getRegister(RID_PC), new_PC);
		simulator.redecodeCurrentInstruction(&cpu);
		return current_PC;
	}

	// Regular Injections
	std::unique_ptr<FaultSpaceElement> target = m_fsp.decode(data_address);
	FaultSpaceElement::injector_result result;
	if(fsppilot->injection_mode() == fsppilot->BURST) {
		m_log << "INJECTING BURST " << endl;
		uint8_t mask = static_cast<uint8_t>(fsppilot->data_mask());
		result = target->inject([&] (unsigned val) -> unsigned {
			return val ^ (mask & 0xFF);
		});
	} else {
		m_log << "INJECTING BITFLIP (bitpos = " << bitpos << ") " << endl;
		result = target->inject([&] (unsigned val) -> unsigned {
			return val ^ (1 << bitpos);
		});
	}
	// Redecode Instruction on RID_PC Change
	// FIXME: After AspectC++ is removed, we can push a simulator
	// instance down into the fault-space hierarchy and redecode in
	// RegisterElement::inject(). At the moment, this is impossible as
	// AspectC++ introduced linking dependencies on Bochs symbols.
	if (auto reg = dynamic_cast<RegisterElement*>(target.get())) {
		ConcreteCPU & cpu = simulator.getCPU(0);
		if (reg->get_base()->getId() == RID_PC) {
			simulator.redecodeCurrentInstruction(&cpu);
		}
	}


	m_log << hex << setw(2) << setfill('0')
		<< std::showbase
		<< "\tIP: " << fsppilot->injection_instr_absolute() << endl
		<< "\tFAULT SITE: FSP " << data_address
		<< " -> AREA " << target->get_area()->get_name() << " @ " << target->get_offset() << endl
		<< "\telement: " << *target << endl
		<< "\tvalue: 0x" << (int)result.original
		<<     " -> 0x" << (int)result.injected << endl;
	return result.original;
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

	// Initialize the faultspace abstraction for injection mode
	RegisterArea &reg_area = (RegisterArea&) m_fsp.get_area("register");
	reg_area.set_state(&simulator.getCPU(0));
	MemoryArea   &mem_area = (MemoryArea&) m_fsp.get_area("ram");
	mem_area.set_manager(&simulator.getMemoryManager());



	if (!this->cb_start_experiment()) {
		m_log << "Initialization failed. Exiting." << endl;
		simulator.terminate(1);
	}

#if defined(BUILD_BOCHS)
	#define MAX_EXECUTED_JOBS 25
#else
	#define MAX_EXECUTED_JOBS UINT_MAX
#endif
	unsigned executed_jobs = 0;
	while (executed_jobs < MAX_EXECUTED_JOBS || m_jc->getNumberOfUndoneJobs() > 0) {
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
		fsppilot->set_data_mask(0xff);
		fsppilot->set_inject_bursts(true);
#endif

		unsigned  injection_instr = fsppilot->injection_instr();

		assert(fsppilot->data_mask() <=255 && "mask covers more than 8 bit, this is unsupported!");
		uint8_t mask = static_cast<uint8_t>(fsppilot->data_mask());

		bool single_injection = false;
		if (   fsppilot->injection_mode() == fsppilot->BURST
			|| fsppilot->injection_mode() == fsppilot->RANDOMJUMP) {
			single_injection = true;
		}

		for (unsigned bit_offset = 0; bit_offset < 8; bit_offset += 1) {
			// if the mask is zero at this bit offset, this bit shall not be injected.
			bool allowed_mask = mask & (1 << bit_offset);
			// additionally, always inject once for bursts.
			// this first bit might be unset otherwise and thus,
			// this address will never be injected otherwise.
			if(!allowed_mask && !single_injection) {
				continue;
			}

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
			BaseListener *listener = 0;
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
			result->set_original_value(injectFault(fsppilot, bit_offset));
			result->set_injection_width(single_injection ? 8 : 1);

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

			// Break bit_offset loop, if we only perform a single injection
			if (single_injection)
				break;
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


