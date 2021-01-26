#pragma once

#include "../SailSimulator.hpp"
#include "SailTagMemoryManager.hpp"

namespace fail {
class SailSimulator : public SailBaseSimulator {
	SailTagMemoryManager m_tag_mem_manager;

public:
	SailSimulator() : SailBaseSimulator() {
		setMemoryManager(&m_tag_mem_manager, MEMTYPE_TAGS);
	}
	bool isJump(uint64_t instrPtr, uint64_t instr);

	virtual void do_save(const std::string& base) override;
	virtual void do_restore(const std::string& base) override;
};
}
