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

#include <string>
using namespace std;
using namespace fail;

bool VEZSExperiment::run()
{
  //MemoryManager& mm = simulator.getMemoryManager();

  //m_elf.printDemangled();
  m_log << "STARTING EXPERIMENT" << endl;
  m_log << "Instruction Pointer: 0x" << hex << simulator.getCPU(0).getInstructionPointer() << endl;
// Test register access
  Register* reg = simulator.getCPU(0).getRegister(RI_R1);
  m_log << "Register R1: 0x" << hex << simulator.getCPU(0).getRegisterContent(reg) << endl;

  reg = simulator.getCPU(0).getRegister(RI_R2);
  m_log << "Register R2: 0x" << hex << simulator.getCPU(0).getRegisterContent(reg) << endl;


// Test Listeners
  address_t address = m_elf.getSymbol("incfoo").getAddress();
  address &= ~1; // Cortex M3 Thumb Mode has the first bit set..
  m_log << "incfoo() @ 0x" << std::hex << address << std::endl;

  address_t pfoo = m_elf.getSymbol("foo").getAddress();
  //BPSingleListener bp(address);
  //BPRangeListener bp(address-32, address + 32);
  //MemWriteListener l_foo( pfoo );
  MemAccessListener l_foo( 0x20002018 ); l_foo.setWatchWidth(0x20);
  reg = simulator.getCPU(0).getRegister(RI_R4);

  unsigned foo = 23;
  for(int i = 0; i < 15; i++){
    simulator.addListenerAndResume(&l_foo);
    //if(i == 0) mm.setBytes(pfoo, 4, (void*)&foo);
    m_log << " Breakpoint hit! @ 0x" << std::hex << simulator.getCPU(0).getInstructionPointer() << std::endl;
    //m_log << " Register R3: 0x" << hex << simulator.getCPU(0).getRegisterContent(reg) << endl;
    //mm.getBytes(pfoo, 4, (void*)&foo);
    m_log << " foo @ 0x"<< std::hex << pfoo << " = " << foo << std::endl;
  }

/*
  BPRangeListener rbp(0xef, 0xff);
  simulator.addListener(&rbp);

  MemAccessListener l_mem_w(0x1111, MemAccessEvent::MEM_WRITE);
  l_mem_w.setWatchWidth(16);
  simulator.addListener(&l_mem_w);

  MemAccessListener l_mem_r(0x2222, MemAccessEvent::MEM_READ);
  l_mem_r.setWatchWidth(16);
  simulator.addListener(&l_mem_r);

  MemAccessListener l_mem_rw(0x3333, MemAccessEvent::MEM_READWRITE);
  l_mem_rw.setWatchWidth(16);
  simulator.addListener(&l_mem_rw);

  simulator.clearListeners();
// resume backend.
//  simulator.resume();

// Test Memory access
  address_t targetaddress = 0x12345678;
  MemoryManager& mm = simulator.getMemoryManager();
  mm.setByte(targetaddress, 0x42);
  mm.getByte(targetaddress);

  uint8_t tb[] = {0xab, 0xbb, 0xcc, 0xdd};
  mm.setBytes(targetaddress, 4, tb);
  *((uint32_t*)(tb)) = 0; // clear array.
  // read back bytes
  mm.getBytes(targetaddress, 4, tb);

*/
  // Explicitly terminate, or the simulator will continue to run.
  simulator.terminate();
}
