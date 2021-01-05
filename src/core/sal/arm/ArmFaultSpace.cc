#include <memory>
#include "ArmFaultSpace.hpp"
#include "sal/faultspace/MemoryArea.hpp"
#include "sal/faultspace/RegisterArea.hpp"

fail::ArmFaultSpace::ArmFaultSpace() {
	// Order Matters!
	add_area(std::make_unique<RegisterArea>());

	// This magic register break was used in pre-faultspace version.
	// We stick to this value to remain as compatible as possible with
	// the old data_addresses
	unsigned register_break = (128 << 4);
	set_point(register_break);

	// 512-4GiB of RAM
	add_area(std::make_unique<MemoryArea>("ram", register_break, 0xFFFFFFFF-register_break));
}

