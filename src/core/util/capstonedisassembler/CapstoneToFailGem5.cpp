#include <capstone/arm.h>
#include "CapstoneToFailGem5.hpp"
#include "sal/arm/ArmArchitecture.hpp"

using namespace fail;

CapstoneToFailGem5::CapstoneToFailGem5() {
	capstone_to_fail_map[ARM_REG_R0] = reginfo_t(RI_R0);
	capstone_to_fail_map[ARM_REG_R1] = reginfo_t(RI_R1);
	capstone_to_fail_map[ARM_REG_R2] = reginfo_t(RI_R2);
	capstone_to_fail_map[ARM_REG_R3] = reginfo_t(RI_R3);
	capstone_to_fail_map[ARM_REG_R4] = reginfo_t(RI_R4);
	capstone_to_fail_map[ARM_REG_R5] = reginfo_t(RI_R5);
	capstone_to_fail_map[ARM_REG_R6] = reginfo_t(RI_R6);
	capstone_to_fail_map[ARM_REG_R7] = reginfo_t(RI_R7);
	capstone_to_fail_map[ARM_REG_R8] = reginfo_t(RI_R8);
	capstone_to_fail_map[ARM_REG_R9] = reginfo_t(RI_R9);
	capstone_to_fail_map[ARM_REG_R10] = reginfo_t(RI_R10);
	capstone_to_fail_map[ARM_REG_R11] = reginfo_t(RI_R11);
	capstone_to_fail_map[ARM_REG_R12] = reginfo_t(RI_R12);
	capstone_to_fail_map[ARM_REG_SP] = reginfo_t(RI_SP);
	capstone_to_fail_map[ARM_REG_LR] = reginfo_t(RI_LR);
	capstone_to_fail_map[ARM_REG_PC] = reginfo_t(RI_IP);
}
