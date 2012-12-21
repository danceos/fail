#include <iostream>
#include <fstream>

// getpid
#include <sys/types.h>
#include <unistd.h>

#include "util/Logger.hpp"

#include "util/ElfReader.hpp"

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"

#include "sal/bochs/BochsRegister.hpp"
#include "sal/bochs/BochsListener.hpp"

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
  !defined(CONFIG_SR_SAVE)
#error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

bool VEZSExperiment::run()
{
  Logger log("VEZS-Example", false);
  ElfReader elf("./x86_bare_test");
  log << "STARTING EXPERIMENT" << endl;
  log << "main() address: " <<   elf.getAddressByName("main") << endl;
  elf.printMangled();
  elf.printDemangled();

  BPSingleListener bp;
#if 0
  // STEP 1: run until interesting function starts, and save state
  bp.setWatchInstructionPointer(elf.getAddressByName("main"));
  if(simulator.addListenerAndResume(&bp) == &bp){
    log << "test function entry reached, saving state" << endl;
  }
  log << "EIP = " << hex << bp.getTriggerInstructionPointer()   << endl;
  //simulator.terminate();
  simulator.save("vezs.state");
  simulator.terminate();
#endif
#if 1

  //int bit_offset = 2;	
  //for (int instr_offset = 0; instr_offset < OOSTUBS_NUMINSTR; ++instr_offset) {

  // STEP 3: The actual experiment.
  log << "restoring state" << endl;
  simulator.restore("vezs.state");

  log << " current EIP = " << simulator.getCPU(0).getInstructionPointer() << endl;
  BPSingleListener bpt0;
  BPSingleListener bpt1;
  bpt0.setWatchInstructionPointer(elf.getAddressByName("Alpha::functionTaskTask0"));
  bpt1.setWatchInstructionPointer(elf.getAddressByName("_ZN4Beta17functionTaskTask1Ev")); // both mangled and demangled name a working.

  simulator.addListener(&bpt1);
  simulator.addListenerAndResume(&bpt0);
  log << "EIP = " << simulator.getCPU(0).getInstructionPointer() <<" "<<elf.getMangledNameByAddress(simulator.getCPU(0).getInstructionPointer()) << endl;
  simulator.resume();
  log << "EIP = " << simulator.getCPU(0).getInstructionPointer() <<" "<<elf.getNameByAddress(simulator.getCPU(0).getInstructionPointer()) << endl; 

  simulator.clearListeners();
  bpt1.setWatchInstructionPointer(elf.getAddressByName("os::krn::SchedImpl::superDispatch_impl"));
  for(int i = 0; i < 10; i++){
    simulator.addListenerAndResume(&bpt1); 
    log << "EIP = " << simulator.getCPU(0).getInstructionPointer() <<" "<< elf.getNameByAddress(simulator.getCPU(0).getInstructionPointer()) << endl;
  }
#endif
#if 0	
  int32_t data = simulator.getCPU(0).getRegister(RID_CAX)->getData();
  // The INJECTION:
  int32_t newdata = data ^ (1<<bit_offset);
  simulator.getCPU(0).getRegister(RID_CAX)->setData(newdata);

  int32_t injection_ip = simulator.getCPU(0).getInstructionPointer();
  log << "inject @ ip " << injection_ip
    << " (offset " << dec << instr_offset << ")"
    << " bit " << bit_offset << ": 0x"
    << hex << ((int)data) << " -> 0x" << ((int)newdata) << endl;

  // --- aftermath ---
  // possible outcomes:
  // - trap, "crash"
  // - jump outside text segment
  // - reaches THE END
  // - error detected, stop
  // additional info:
  // - #loop iterations before/after FI

  // catch traps as "extraordinary" ending
  TroubleListener ev_trap(ANY_TRAP);
  simulator.addListener(&ev_trap);
  // jump outside text segment
  BPRangeListener ev_below_text(ANY_ADDR, OOSTUBS_TEXT_START - 1);
  BPRangeListener ev_beyond_text(OOSTUBS_TEXT_END + 1, ANY_ADDR);
  simulator.addListener(&ev_below_text);
  simulator.addListener(&ev_beyond_text);
  // timeout (e.g., stuck in a HLT instruction)
  // 10000us = 500000 instructions
  GenericTimerListener ev_timeout(1000000); // 50,000,000 instructions !!
  simulator.addListener(&ev_timeout);

  // remaining instructions until "normal" ending
  BPSingleListener ev_end(ANY_ADDR);
  ev_end.setCounter(OOSTUBS_NUMINSTR - instr_offset);
  simulator.addListener(&ev_end);

  // Start simulator and wait for any result
  BaseListener* ev = simulator.resume();

  // record latest IP regardless of result
  injection_ip =  simulator.getCPU(0).getInstructionPointer();


  if (ev == &ev_end) {
    log << dec << "Result FINISHED" << endl;
  } else if (ev == &ev_timeout) {
    log << "Result TIMEOUT" << endl;
  } else if (ev == &ev_below_text || ev == &ev_beyond_text) {
    log << "Result OUTSIDE" << endl;
  } else if (ev == &ev_trap) {
    log << dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
  } else {
    log << "Result WTF?" << endl;
  }
  log << "@ ip 0x" << hex << injection_ip << endl;
  // explicitly remove all events before we leave their scope
  // FIXME event destructors should remove them from the queues
  simulator.clearListeners();
}

#endif
// Explicitly terminate, or the simulator will continue to run.
simulator.terminate();
}
