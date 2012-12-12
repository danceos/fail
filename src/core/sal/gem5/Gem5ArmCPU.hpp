#ifndef __GEM5_ARM_CPU_HPP__
  #define __GEM5_ARM_CPU_HPP__

#include "../arm/Architecture.hpp"
#include "../arm/CPUState.hpp"

#include "sim/system.hh"

namespace fail {

class Gem5ArmCPU : public ArmArchitecture, public ArmCPUState {
public:
	// TODO: comments
	Gem5ArmCPU(unsigned int id, System* system) : m_Id(id), m_System(system) { }
	regdata_t getRegisterContent(Register* reg);
	void setRegisterContent(Register* reg, regdata_t value);

	address_t getInstructionPointer();
	address_t getStackPointer();
	address_t getLinkRegister();

	unsigned int getId() { return m_Id; }
private:
	unsigned int m_Id;
	System* m_System;
};

typedef Gem5ArmCPU ConcreteCPU;

} // end-of-namespace: fail

#endif // __GEM5_ARM_CPU_HPP__
