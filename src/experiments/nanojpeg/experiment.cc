#include <iostream>

// getpid
#include <sys/types.h>
#include <unistd.h>

#include "util/Logger.hpp"
#include "util/ElfReader.hpp"

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"
#include "psnr.hpp"

#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include "sal/Register.hpp"

// you need to have the tracing plugin enabled for this
#include "../plugins/tracing/TracingPlugin.hpp"

#define PREPARATION 1
#define LOCAL 1

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
//#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
//    !defined(CONFIG_SR_SAVE) || !defined(CONFIG_EVENT_TRAP)
//  #error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
//#endif

bool NanoJPEGExperiment::run()
{
	Logger log("nJPEG", true);

	log << "startup" << endl;

#if PREPARATION == 1
	// STEP 1: run until main starts, save state, record trace
	// TODO: store golden run output
	IOPortListener io(0x3f8, true);
	simulator.addListenerAndResume(&io);

	log << "main() reached, saving state" << endl;
	simulator.save(NANOJPEG_STATE);

	// record trace
	log << "restoring state" << endl;
	simulator.restore(NANOJPEG_STATE);
	log << "EIP = " << hex << simulator.getCPU(0).getInstructionPointer() << endl;
	log << "enabling tracing" << endl;
	TracingPlugin tp;
	tp.setLogIPOnly(true);
	ofstream of(NANOJPEG_TRACE);
	tp.setTraceFile(&of);
	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

	// count instructions
	simulator.addListener(&io);
	BPSingleListener step(ANY_ADDR);
	long counter = 0;
	while (true) {
		BaseListener *l = simulator.addListenerAndResume(&step);
		if (l == &io) {
			break;
		}
		counter++;
	}
	log << "golden run took " << dec << counter << " instructions" << endl;
	simulator.removeFlow(&tp);
	of.close();
#else
	// STEP 2: the actual experiment
	PSNR psnr(NANOJPEG_GOLDEN_PPM);

	ElfReader elfreader(NANOJPEG_ELF);
	guest_address_t addr_text_start =
		elfreader.getSymbol("___TEXT_START__").getAddress();
	guest_address_t addr_text_end =
		elfreader.getSymbol("___TEXT_END__").getAddress();
	guest_address_t addr_rodata_start =
		elfreader.getSymbol("___RODATA_START__").getAddress();
	guest_address_t addr_bss_end =
		elfreader.getSymbol("___BSS_END__").getAddress();
	guest_address_t addr_output_image_ptr =
		elfreader.getSymbol("output_image").getAddress();
	guest_address_t addr_output_image_size =
		elfreader.getSymbol("output_image_size").getAddress();
	log << "ELF symbols: text " << hex << addr_text_start << "-" << addr_text_end
	    << " rodata/data/bss " << addr_rodata_start << "-" << addr_bss_end
	    << " output_image ptr @ " << addr_output_image_ptr << ", size @ " << addr_output_image_size << endl;
	elfreader.~ElfReader();

#if !LOCAL
	for (int experiment_count = 0; experiment_count < 200 || (m_jc.getNumberOfUndoneJobs() != 0) ; ) { // only do 200 sequential experiments, to prevent swapping
#endif

	// get an experiment parameter set
	log << "asking job server for experiment parameters" << endl;
	NanoJPEGExperimentData param;
#if !LOCAL
	if (!m_jc.getParam(param)) {
		log << "Dying." << endl;
		simulator.terminate(1);
	}
#else
	// XXX debug
	param.msg.set_instr_offset(2000);
	//param.msg.set_instr_address(12345);
	param.msg.set_register_id(0); // EAX
	//param.msg.set_bitmask(0xffffffffULL);
	param.msg.set_bitmask(0x2);
	param.msg.set_timeout(NANOJPEG_TIMEOUT);
#endif

	int id = param.getWorkloadID();
	int instr_offset = param.msg.instr_offset();
	Register *reg = simulator.getCPU(0).getRegister(param.msg.register_id());
	uint64_t bitmask = param.msg.bitmask();
	int timeout = param.msg.timeout();

	for (int bitnr = 0; bitnr < 32; ++bitnr) {
		if (!((1 << bitnr) & bitmask)) {
			continue;
		}
#if !LOCAL
		experiment_count++;
#endif
		log << "ID=" << id << " instr=0x" << hex << instr_offset << " reg=" << reg->getName()
		    << " bitnr=" << dec << bitnr << " timeout=" << timeout << endl;
		NanoJPEGProtoMsg_Result *result = param.msg.add_result();
		result->set_bitnr(bitnr);

		log << "restoring state" << endl;
		simulator.restore(NANOJPEG_STATE);

		// no need to wait if offset is 0
		if (instr_offset > 0) {
			// XXX could be improved with intermediate states (reducing runtime until injection)
			BPSingleListener bp_fi;
			bp_fi.setWatchInstructionPointer(ANY_ADDR);
			bp_fi.setCounter(instr_offset);
			if (simulator.addListenerAndResume(&bp_fi) != &bp_fi) {
				log << "WTF?" << endl;
				simulator.terminate(1);
			}
		}

		// --- fault injection ---
		uint32_t data = reg->getData();
		uint32_t newdata = data ^ (1 << bitnr);
		reg->setData(newdata);
		// note at what IP we did it
		uint32_t injection_ip = simulator.getCPU(0).getInstructionPointer();
		param.msg.set_injection_ip(injection_ip);
		log << "fault injected @ ip " << injection_ip << " reg " << reg->getName()
			<< " 0x" << hex << ((int)data) << " -> 0x" << ((int)newdata) << endl;
		// sanity check
		if (param.msg.has_instr_address() &&
			injection_ip != param.msg.instr_address()) {
			stringstream ss;
			ss << "SANITY CHECK FAILED: " << injection_ip
			   << " != " << param.msg.instr_address();
			log << ss.str() << endl;
			result->set_resulttype(result->UNKNOWN);
			result->set_latest_ip(injection_ip);
			result->set_details(ss.str());

			simulator.clearListeners();
			continue;
		}

		// --- aftermath ---
		// possible outcomes:
		// - trap, "crash"
		// - jump outside text segment
		// - memory access outside of DATA/BSS
		// - (XXX unaligned jump inside text segment)
		// - (XXX weird instructions?)
		// - reaches the end, PSNR can be calculated
		// - reaches the end, output image broken
		// - global timeout
		// additional info:
		// - PSNR

		// normal experiment end: guest system tells us
		IOPortListener ev_io(0x3f8, true);
		simulator.addListener(&ev_io);
		// global timeout
		TimerListener ev_timeout(timeout);
		simulator.addListener(&ev_timeout);
		// catch traps as "extraordinary" ending
		TrapListener ev_trap(ANY_TRAP);
		simulator.addListener(&ev_trap);
		// jump outside text segment
		BPRangeListener ev_below_text(ANY_ADDR, addr_text_start - 1);
		BPRangeListener ev_beyond_text(addr_text_end + 1, ANY_ADDR);
		simulator.addListener(&ev_below_text);
		simulator.addListener(&ev_beyond_text);
		// memory access outside of data/bss segment
		MemAccessListener ev_mem_low(0x0, MemAccessEvent::MEM_READWRITE);
		ev_mem_low.setWatchWidth(addr_rodata_start);
		MemAccessListener ev_mem_high(addr_bss_end + 1, MemAccessEvent::MEM_READWRITE);
		ev_mem_high.setWatchWidth(0xFFFFFFFFU - (addr_bss_end + 1));
		simulator.addListener(&ev_mem_low);
		simulator.addListener(&ev_mem_high);

		BaseListener *ev = simulator.resume();
		// record latest IP regardless of result
		result->set_latest_ip(simulator.getCPU(0).getInstructionPointer());

		if (ev == &ev_io) {
			log << "Result FINISHED/BROKEN (" << ev_io.getData() << ")" << endl;
			// retrieve image
			MemoryManager& mm = simulator.getMemoryManager();
			uint32_t output_image_addr;
			mm.getBytes(addr_output_image_ptr, 4, &output_image_addr);
			uint32_t output_image_size;
			mm.getBytes(addr_output_image_size, 4, &output_image_size);
			log << "image address " << hex << output_image_addr << " size " << dec << output_image_size << endl;
			if (output_image_size != 3 * psnr.getWidth() * psnr.getHeight()) {
				log << "odd image size" << endl;
				result->set_details("odd image size");
				result->set_resulttype(result->BROKEN);
			} else if (output_image_addr < addr_rodata_start || output_image_addr >= addr_bss_end) {
				log << "odd image address" << endl;
				result->set_details("odd image address");
				result->set_resulttype(result->BROKEN);
			} else {
				result->set_resulttype(result->FINISHED);
				string image;
				image.resize(output_image_size);
				mm.getBytes(output_image_addr, output_image_size, &image[0]);
				// calculate PSNR
				double psnr_value = psnr.calculate(image);
				result->set_psnr(psnr_value);
				log << "PSNR: " << psnr_value << endl;
			}
		} else if (ev == &ev_timeout) {
			log << "Result TIMEOUT" << endl;
			result->set_resulttype(result->TIMEOUT);
		} else if (ev == &ev_below_text || ev == &ev_beyond_text) {
			log << "Result OUTSIDE" << endl;
			result->set_resulttype(result->OUTSIDE);
		} else if (ev == &ev_mem_low || ev == &ev_mem_high) {
			log << "Result OUTSIDEMEM (EIP " << result->latest_ip() << ")" << endl;
			result->set_resulttype(result->OUTSIDEMEM);
		} else if (ev == &ev_trap) {
			log << dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
			result->set_resulttype(result->TRAP);

			stringstream ss;
			ss << ev_trap.getTriggerNumber();
			result->set_details(ss.str());
		} else {
			log << "Result WTF?" << endl;
			result->set_resulttype(result->UNKNOWN);

			stringstream ss;
			ss << "eventid " << hex << ((unsigned long) ev) << " EIP " << simulator.getCPU(0).getInstructionPointer();
			result->set_details(ss.str());
		}
	}

#if !LOCAL
	m_jc.sendResult(param);
	}
#endif

#endif
	// Explicitly terminate, or the simulator will continue to run.
	simulator.terminate();
}
