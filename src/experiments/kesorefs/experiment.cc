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

using namespace std;
using namespace fail;

#define SAFESTATE (0)

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_SR_RESTORE) || \
  !defined(CONFIG_SR_SAVE)
#error This experiment needs: breakpoints, traps, save, and restore. Enable these in the configuration.
#endif

void KESOrefs::printEIP() {
  m_log << "EIP = 0x" << hex << simulator.getCPU(0).getInstructionPointer() <<" "<< m_elf.getNameByAddress(simulator.getCPU(0).getInstructionPointer()) << endl;
}

static vector<BPSingleListener*> mg_exitbps;

void KESOrefs::setupExitBPs(const string& funcname){
  BPSingleListener* bp = new BPSingleListener();
  bp->setWatchInstructionPointer(m_elf.getAddressByName(funcname));

  mg_exitbps.push_back(bp);
}

void KESOrefs::enableBPs(){
  vector<BPSingleListener*>::const_iterator it;
  // add all BPs
  for(it = mg_exitbps.begin(); it != mg_exitbps.end(); ++it){
    simulator.addListener(*it);
  }
}

void KESOrefs::clearExitBPs(){
  for( size_t i = 0; i < mg_exitbps.size(); i++){
    delete mg_exitbps[i];
  }
  mg_exitbps.clear();
}

const unsigned KESO_NUM_STATIC_REFS = 36; // from KESO globals.h

address_t rev_byte(address_t dword){
  return ((dword>>24)&0x000000FF) | ((dword>>8)&0x0000FF00) | ((dword<<8)&0x00FF0000) | ((dword<<24)&0xFF000000);
}

void KESOrefs::showStaticRefs(){
  address_t sref_start = m_elf.getAddressByName("__CIAO_APPDATA_cdx_det__heap"); // guest_address_t -> uint32_t
  MemoryManager& mm = simulator.getMemoryManager();
  address_t value = 0;
  m_log << "__CIAO_APPDATA_cdx_det__heap : 0x" << hex  << setw(8) << setfill('0') << sref_start << endl;

  for(unsigned i = 0; i < KESO_NUM_STATIC_REFS; ++i){
    mm.getBytes(sref_start+(i*4), 4, (void*)&value);
    value = rev_byte(value);
    cout << "0x" << hex << setw(8) << setfill('0')   << value << " | ";
    if ((i+1) % 8 == 0) cout << "" <<  endl;
  }
  cout << "" << endl;
}

void KESOrefs::injectStaticRefs(unsigned referenceoffset, unsigned bitpos){
  address_t sref_start = m_elf.getAddressByName("__CIAO_APPDATA_cdx_det__heap"); // guest_address_t -> uint32_t

  MemoryManager& mm = simulator.getMemoryManager();
  address_t value = 0, injectedval =0;

  sref_start += (referenceoffset*4);

  if(referenceoffset > KESO_NUM_STATIC_REFS){
    m_log << "WARNING: reference offset to large!" << endl;
  }
  mm.getBytes(sref_start, 4, (void*)&value);
  injectedval = value ^ bitpos;
  mm.setBytes(sref_start, 4, (void*)&injectedval);

  m_log << "INJECTION at: __CIAO_APPDATA_cdx_det__heap + " << referenceoffset << " : 0x" << hex  << setw(8) << setfill('0') << sref_start;
  cout << " value: 0x" << setw(8) << setfill('0') << value << " -> 0x" << setw(8) << setfill('0') << injectedval << endl;

}



bool KESOrefs::run()
{
//******* Boot, and store state *******//
  m_log << "STARTING EXPERIMENT" << endl;
  printEIP();

#if SAFESTATE // define SS (SafeState) when building: make -DSS
#warning "Building safe state variant"
  m_log << "Booting, and saving state at ";
  BPSingleListener bp;
  // STEP 1: run until interesting function starts, and save state
  bp.setWatchInstructionPointer(m_elf.getAddressByName("main"));
  if(simulator.addListenerAndResume(&bp) == &bp){
    m_log << "main function entry reached, saving state" << endl;
  }
  printEIP();

  simulator.save("keso.state");
  simulator.terminate();
#else

//******* Fault injection *******//
#warning "Building restore state variant"
  simulator.restore("keso.state");


  // Goto injection point
  BPSingleListener injBP;
  injBP.setWatchInstructionPointer(m_elf.getAddressByName("c23_PersistentDetectorScopeEntry_m5_run"));
  simulator.addListenerAndResume(&injBP);
  printEIP(); m_log << "Lets inject some stuff..." << endl;
  showStaticRefs();
  /// INJECT BITFLIP:
  injectStaticRefs(9, 9);
  showStaticRefs();

  // Setup exit points
  setupExitBPs("keso_throw_error");
  setupExitBPs("keso_throw_parity");
  setupExitBPs("keso_throw_nullpointer");
  setupExitBPs("keso_throw_index_out_of_bounds");
  setupExitBPs("c17_Main_m4_dumpResults_console");
  setupExitBPs("os::krn::OSControl::shutdownOS");

  enableBPs();

  // resume and wait for results
  /* fail::BaseListener* l =*/ simulator.resume();
  printEIP();
  showStaticRefs();
  // cleanup
  clearExitBPs();
// Explicitly terminate, or the simulator will continue to run.
#endif
simulator.terminate();
}
