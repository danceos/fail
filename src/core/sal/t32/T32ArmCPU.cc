#include "T32ArmCPU.hpp"
#include <t32.h>
#include <iostream>

namespace fail {

static const uint64_t lower = 0x00000000ffffffff;

regdata_t T32ArmCPU::getRegisterContent(const Register* reg) const
{
	// T32_ReadRegister wants a mask of bits representig the registers to read:
	// e.g., reading R1 and R4 and R63
	//             mask1
	// 0000 0000 0000 0000 0001 0010 ->  R1/R4
	//             mask2
	// 1000 0000 0000 0000 0000 0000 ->  R63
	uint64_t mask = (1 << reg->getIndex());

	if (mask) {
		if ( T32_ReadRegister(static_cast<dword>(mask & lower ), static_cast<dword>(mask >> 32), m_regbuffer) == 0 ) {
			// No error, return value.
			return m_regbuffer[reg->getIndex()];
		} else {
			/// TODO Error handling!
			std::cout << "could not read register :(" << std::endl;
		}
	}
	return 0; // we should not come here.
}

void T32ArmCPU::setRegisterContent(const Register* reg, regdata_t value)
{
	uint64_t mask = (1 << reg->getIndex());

	// set value to be set by T32:
	m_regbuffer[reg->getIndex()] = value;
	if (mask) {
		if ( T32_WriteRegister(static_cast<dword>(mask & lower), static_cast<dword>(mask >> 32), m_regbuffer) == 0 ) {
			// No error, return value.
			return;
		} else {
			/// TODO Error handling!
			std::cout << "could not write register :(" << std::endl;
		}
	}
}


address_t T32ArmCPU::getInstructionPointer() const
{
	// TODO: programpointer is only valid when Emulation is stopped! -> T32_GetState)
	address_t programpointer;
	T32_ReadPP( &programpointer );
	return programpointer;
}

} // end-of-namespace: fail


