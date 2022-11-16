#pragma once

#include "../SailCPU.hpp"

namespace fail {
class SailCPU : public SailBaseCPU {
public:
	SailCPU(unsigned int id) : SailBaseCPU(id) {
		pc_reg = getRegister(RID_RAW_PC);
	}
	virtual ~SailCPU() {}
};
}

