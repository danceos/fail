#pragma once

#include "../SailSimulator.hpp"

namespace fail {
class SailSimulator : public SailBaseSimulator {
	bool isJump(uint64_t instrPtr, uint64_t instr);
};
}
