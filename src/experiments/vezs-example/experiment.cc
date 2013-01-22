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

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
  !defined(CONFIG_SR_SAVE)
#error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

#define SAVESTATE (0)

void VEZSExperiment::printEIP() {
  m_log << "EIP = 0x" << hex << simulator.getCPU(0).getInstructionPointer() <<" "<< m_elf.getNameByAddress(simulator.getCPU(0).getInstructionPointer()) << endl; 
}

std::vector<string> v;

bool VEZSExperiment::run()
{
  m_log << "STARTING EXPERIMENT" << endl;
  printEIP();

#if(SAVESTATE)
  m_log << "Booting, and saving state at ";
  BPSingleListener bp;
  // STEP 1: run until interesting function starts, and save state
  bp.setWatchInstructionPointer(m_elf.getAddressByName("main"));
  if(simulator.addListenerAndResume(&bp) == &bp){
    m_log << "test function entry reached, saving state" << endl;
  }
  printEIP();
  //simulator.terminate();
  simulator.save("vezs.state");
  simulator.terminate();
#else
  simulator.restore("vezs.state");
  //m_elf.printDemangled();
  BPSingleListener bpt0;
  BPSingleListener bpt1;
  BPSingleListener bpt2;
  //BPSingleListener inst(ANY_ADDR);
  //bpt0.setWatchInstructionPointer(m_elf.getAddressByName("c17_Main_m4_dumpResults_console"));
  //bpt0.setWatchInstructionPointer(m_elf.getAddressByName("keso_throw_error"));
  //bpt1.setWatchInstructionPointer(m_elf.getAddressByName("c17_Main_m3_run"));
  bpt2.setWatchInstructionPointer(m_elf.getAddressByName("os::krn::OSControl::shutdownOS"));
  //simulator.addListener(&bpt0);
  //simulator.addListener(&bpt1);
  //simulator.addListener(&bpt2);
  simulator.addListener(&bpt2);
  fail::BaseListener* l = simulator.resume();
  printEIP();
  simulator.terminate();
  while(1){
    if(simulator.getCPU(0).getInstructionPointer() == m_elf.getAddressByName("os::krn::OSControl::shutdownOS")) {
      printEIP();
      break;
    }else{
      //std::string name = m_elf.getNameByAddress(simulator.getCPU(0).getInstructionPointer());
      //if(name != ElfReader::NOTFOUND){
      //  v.push_back(name);
      //}
      printEIP();
      l = simulator.addListenerAndResume(l);
    }
  }

  //  simulator.clearListeners();
//  bpt1.setWatchInstructionPointer(m_elf.getAddressByName("os::krn::SchedImpl::superDispatch_impl"));
//  for(;;){
//    simulator.addListenerAndResume(&bpt1); 
//    printEIP();
//  }

// Explicitly terminate, or the simulator will continue to run.
#endif
simulator.terminate();
}
