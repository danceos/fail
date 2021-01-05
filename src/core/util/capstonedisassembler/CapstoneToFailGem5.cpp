#include <capstone/arm.h>
#include "CapstoneToFailGem5.hpp"
#include "sal/arm/ArmArchitecture.hpp"
#include "CapstoneDisassembler.hpp"

using namespace fail;

CapstoneToFailGem5::CapstoneToFailGem5(CapstoneDisassembler *disas) {
	capstone_to_fail_map[ARM_REG_R0] = RegisterView(RI_R0);
	capstone_to_fail_map[ARM_REG_R1] = RegisterView(RI_R1);
	capstone_to_fail_map[ARM_REG_R2] = RegisterView(RI_R2);
	capstone_to_fail_map[ARM_REG_R3] = RegisterView(RI_R3);
	capstone_to_fail_map[ARM_REG_R4] = RegisterView(RI_R4);
	capstone_to_fail_map[ARM_REG_R5] = RegisterView(RI_R5);
	capstone_to_fail_map[ARM_REG_R6] = RegisterView(RI_R6);
	capstone_to_fail_map[ARM_REG_R7] = RegisterView(RI_R7);
	capstone_to_fail_map[ARM_REG_R8] = RegisterView(RI_R8);
	capstone_to_fail_map[ARM_REG_R9] = RegisterView(RI_R9);
	capstone_to_fail_map[ARM_REG_R10] = RegisterView(RI_R10);
	capstone_to_fail_map[ARM_REG_R11] = RegisterView(RI_R11);
	capstone_to_fail_map[ARM_REG_R12] = RegisterView(RI_R12);
	capstone_to_fail_map[ARM_REG_SP] = RegisterView(RI_SP);
	capstone_to_fail_map[ARM_REG_LR] = RegisterView(RI_LR);
	capstone_to_fail_map[ARM_REG_PC] = RegisterView(RI_IP);
}
