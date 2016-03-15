#include <iostream>
#include <iomanip>
#include <fstream>

#include <stdlib.h>
#include "sal/SALConfig.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include "efw/DatabaseExperiment.hpp"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "comm/DatabaseCampaignMessage.pb.h"
#include <string>
#include <vector>

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

unsigned DatabaseExperiment::injectBurst(address_t data_address) {
	unsigned value, burst_value;
	value = m_mm.getByte(data_address);
	burst_value = value ^ 0xff;
	m_mm.setByte(data_address, burst_value);

	m_log << "INJECTED BURST at: 0x" << hex<< setw(2) << setfill('0') << data_address
		  << " value: 0x" << setw(2) << setfill('0') << value << " -> 0x"
		  << setw(2) << setfill('0') << burst_value << endl;
	return value;
}

unsigned DatabaseExperiment::injectBitFlip(address_t data_address, unsigned bitpos){
	unsigned int value, injectedval;

	value = m_mm.getByte(data_address);
	injectedval = value ^ (1 << bitpos);
	m_mm.setByte(data_address, injectedval);

	m_log << "INJECTION at: 0x" << hex<< setw(2) << setfill('0') << data_address
		  << " value: 0x" << setw(2) << setfill('0') << value << " -> 0x"
		  << setw(2) << setfill('0') << (unsigned) m_mm.getByte(data_address) << endl;

	return value;
}

template<class T>
T * protobufFindSubmessageByTypename(Message *msg, const std::string &name) {
	T * submessage = 0;
	const Descriptor *msg_type = msg->GetDescriptor();
	const Message::Reflection *ref = msg->GetReflection();
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
		unsigned injection_width = fsppilot->inject_bursts() ? 8 : 1;

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

			if (fsppilot->inject_bursts()) {
				/// INJECT BURST:
				result->set_original_value(injectBurst((data_address+bit_offset/8)));
			} else {
				/// INJECT BITFLIP:
				result->set_original_value(injectBitFlip(data_address, bit_offset));
			}
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


