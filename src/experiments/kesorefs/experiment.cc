#include <iostream>
#include <fstream>

// getpid
#include <sys/types.h>
#include <unistd.h>


#include <stdlib.h>
#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"

#include "sal/bochs/BochsListener.hpp"
#include <string>
#include <vector>

#include "campaign.hpp"
#include "kesoref.pb.h"

using namespace std;
using namespace fail;

#define SAFESTATE (0)

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
  !defined(CONFIG_SR_SAVE)
#error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif
//
//void KESOrefs::printEIP() {
//  m_log << "EIP = 0x" << hex << simulator.getCPU(0).getInstructionPointer() <<" "<< m_elf.getNameByAddress(simulator.getCPU(0).getInstructionPointer()) << endl;
//}

unsigned KESOrefs::injectBitFlip(address_t data_address, unsigned bitpos){

  MemoryManager& mm = simulator.getMemoryManager();
  unsigned value, injectedval;

  mm.getBytes(data_address, 4, (void*)&value);
  injectedval = value ^ bitpos;
  mm.setBytes(data_address, 4, (void*)&injectedval);

  m_log << "INJECTION at: 0x" << hex  << setw(8) << setfill('0') << data_address;
  cout << " value: 0x" << setw(8) << setfill('0') << value << " -> 0x" << setw(8) << setfill('0') << injectedval << endl;

  return value;
}


void handleEvent(KesoRefExperimentData& param, KesoRefProtoMsg_ResultType restype, const std::string &msg){
    cout << msg << endl;
    param.msg.set_resulttype(restype);
    param.msg.set_details(msg);
}

void handleMemoryAccessEvent(KesoRefExperimentData& param, const fail::MemAccessListener& l_mem){
    stringstream sstr;
    sstr << "mem access (";
    switch (l_mem.getTriggerAccessType()) {
      case MemAccessEvent::MEM_READ:
          sstr << "r";
        break;
      case MemAccessEvent::MEM_WRITE:
          sstr << "w";
        break;
      default: break;
    }
    sstr << ") @ 0x" << hex << l_mem.getTriggerAddress();

    sstr << " ip @ 0x" << hex << l_mem.getTriggerInstructionPointer();

    handleEvent(param, param.msg.MEMACCESS, sstr.str());
}


bool KESOrefs::run()
{
//******* Boot, and store state *******//
  m_log << "STARTING EXPERIMENT" << endl;
  ElfReader m_elf;
#if SAFESTATE // define SS (SafeState) when building: make -DSS
#warning "Building safe state variant"
  m_log << "Booting, and saving state at main";
  BPSingleListener bp;
  // STEP 1: run until interesting function starts, and save state
  bp.setWatchInstructionPointer(m_elf.getAddressByName("main"));
  if(simulator.addListenerAndResume(&bp) == &bp){
    m_log << "main function entry reached, saving state" << endl;
  }

  simulator.save("keso.state");
  simulator.terminate();
#else

//******* Fault injection *******//
#warning "Building restore state variant"

	for (int experiment_count = 0; experiment_count < 200 || (m_jc.getNumberOfUndoneJobs() != 0) ; ) { // only do 200 sequential experiments, to prevent swapping

  m_log << "asking jobserver for parameters" << endl;
  KesoRefExperimentData param;
  if(!m_jc.getParam(param)){
    m_log << "Dying." << endl; // We were told to die.
    simulator.terminate(1);
  }

  // Get input data from  Jobserver
  address_t injectionPC = param.msg.pc_address();
  address_t data_address = param.msg.ram_address();
  unsigned bitpos = param.msg.bit_offset();

  simulator.restore("keso.state");
  // Goto injection point
  BPSingleListener injBP;
  m_log << "Trying to inject @ " << hex << m_elf.getNameByAddress(injectionPC) << endl;

  injBP.setWatchInstructionPointer(injectionPC);

  simulator.addListenerAndResume(&injBP);
  /// INJECT BITFLIP:
  param.msg.set_original_value(injectBitFlip(data_address, bitpos));

  // Setup exit points
  BPSingleListener l_error(m_elf.getAddressByName("keso_throw_error"));
  BPSingleListener l_nullp(m_elf.getAddressByName("keso_throw_nullpointer"));
  BPSingleListener l_parity(m_elf.getAddressByName("keso_throw_parity"));
  BPSingleListener l_oobounds(m_elf.getAddressByName("keso_throw_index_out_of_bounds"));
  BPSingleListener l_dump(m_elf.getAddressByName("c17_Main_m4_dumpResults_console"));
	MemAccessListener l_mem_text(m_elf.getSectionStart(".text"), MemAccessEvent::MEM_WRITE); l_mem_text.setWatchWidth(m_elf.getSectionSize(".text"));
	MemAccessListener l_mem_textcdx_det( m_elf.getSectionStart(".text.cdx_det"), MemAccessEvent::MEM_WRITE ); l_mem_textcdx_det.setWatchWidth(m_elf.getSectionSize(".text.cdx_det"));
	MemAccessListener l_mem_outerspace( m_elf.getSectionStart(".copy_sec") ); l_mem_outerspace.setWatchWidth(0xfffffff0);
  TrapListener l_trap(ANY_TRAP);
cout << " outerspace : " << l_mem_outerspace.getWatchWidth() << " --- @ :" << l_mem_outerspace.getWatchAddress() << endl;
  simulator.addListener(&l_trap);
  simulator.addListener(&l_error);
  simulator.addListener(&l_nullp);
  simulator.addListener(&l_oobounds);
  simulator.addListener(&l_dump);
  simulator.addListener(&l_parity);
  simulator.addListener(&l_mem_text);
  simulator.addListener(&l_mem_outerspace);
  simulator.addListener(&l_mem_textcdx_det);
  // resume and wait for results
  fail::BaseListener* l = simulator.resume();

  // Evaluate result
  if(l == &l_error) {
    handleEvent(param, param.msg.EXC_ERROR, "exc error");

  } else if ( l == &l_nullp ) {
    handleEvent(param, param.msg.EXC_NULLPOINTER, "exc nullpointer");

  } else if ( l == &l_oobounds ) {
    handleEvent(param, param.msg.EXC_OOBOUNDS, "exc out of bounds");

  } else if (l == &l_dump) {
    handleEvent(param, param.msg.CALCDONE, "calculation done");

  } else if (l == &l_parity) {
    handleEvent(param, param.msg.EXC_PARITY, "exc parity");

  }  else if (l == &l_trap) {
    stringstream sstr;
    sstr << "trap #" << l_trap.getTriggerNumber();
    handleEvent(param, param.msg.TRAP, sstr.str());

  } else if (l == &l_mem_text){
    handleMemoryAccessEvent(param, l_mem_text);

  } else if (l == &l_mem_textcdx_det){
    handleMemoryAccessEvent(param, l_mem_textcdx_det);

  } else if (l == &l_mem_outerspace){
    handleMemoryAccessEvent(param, l_mem_outerspace);

  } else {
    handleEvent(param, param.msg.UNKNOWN, "UNKNOWN event");
  }
  simulator.clearListeners();
  m_jc.sendResult(param);
} // end while (1)
// Explicitly terminate, or the simulator will continue to run.
#endif
simulator.terminate();
}

