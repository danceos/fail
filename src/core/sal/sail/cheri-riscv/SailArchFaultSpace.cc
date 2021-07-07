#include <memory>
#include "SailArchFaultSpace.hpp"
#include "SailArchRegisters.hpp"
#include "sal/sail/SailArchitecture.hpp"
#include "sal/faultspace/MemoryArea.hpp"
#include "sal/faultspace/RegisterArea.hpp"

using namespace fail;

std::vector<std::pair<RegisterElement, unsigned>>
SailCheriRegisterArea::translate(const RegisterView &view) {
	auto ret = RegisterArea::translate(view);
	Register*    base = m_arch.getRegister(view.id);
	if (auto reg = dynamic_cast<SailCapabilityRegister*>(base)) {
		SailRegisterCheriTags* regtags = dynamic_cast<SailRegisterCheriTags *>(
			m_arch.getRegister(RID_REGTAGS));
		unsigned offset = regtags->getTagIndex(reg);
		RegisterView view_tag(RID_REGTAGS, 1, offset);

		for (auto pair : RegisterArea::translate(view_tag)) {
			ret.push_back(pair);
		}
	}
	return ret;
}

SailFaultSpace::SailFaultSpace() {
	// Order Matters!
	// 4GiB of RAM
	add_area(std::make_unique<MemoryArea>("ram", 0, 0xffffffff));
	set_point(1L<<32);
	// The Tag memory
	add_area(std::make_unique<MemoryArea>("tags", 0, 0xffffffff));
	set_point(2L<<32);
	// The Registers
	add_area(std::make_unique<SailCheriRegisterArea>());
}

