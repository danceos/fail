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
  m_log << "STARTING EXPERIMENT" << endl;
  m_log << "Instruction Pointer: 0x" << hex << simulator.getCPU(0).getInstructionPointer() << endl;

  Register* reg = simulator.getCPU(0).getRegister(RI_R1);
  m_log << "Register R2: 0x" << hex << simulator.getCPU(0).getRegisterContent(reg) << endl;

  reg = simulator.getCPU(0).getRegister(RI_R2);
  m_log << "Register R1: 0x" << hex << simulator.getCPU(0).getRegisterContent(reg) << endl;

  simulator.getCPU(0).setRegisterContent(reg, 0x23);

  address_t targetaddress = 0x12345678;
  MemoryManager& mm = simulator.getMemoryManager();
  mm.setByte(targetaddress, 0x42);
  mm.getByte(targetaddress);

  uint8_t tb[] = {0xaa, 0xbb, 0xcc, 0xdd};
  mm.setBytes(targetaddress, 4, tb);
  *((uint32_t*)(tb)) = 0; // clear array.
  // read back bytes
  mm.getBytes(targetaddress, 4, tb);

  // Explicitly terminate, or the simulator will continue to run.
  simulator.terminate();
}
