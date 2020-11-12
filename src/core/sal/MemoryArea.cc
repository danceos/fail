#include "MemoryArea.hpp"
#include "Memory.hpp"
#include <sstream>

namespace fail {
    using namespace fail::util::fsp;

    std::unique_ptr<element> memory_area::decode(address_t addr) {
        auto e = std::make_unique<memory_element>(this, addr);
        return std::move(e);
    }

    memory_element::memory_element(memory_area* area, address_t addr)
                :element(static_cast<class area*>(area), addr) { }

    injector_result memory_element::inject(injector_fn injector) {
        memory_area* ma = dynamic_cast<memory_area*>(get_area());
        assert(ma != nullptr);

        auto& mm = ma->get_manager();
        guest_address_t addr = static_cast<guest_address_t>(get_offset());
        byte_t byte = mm->getByte(addr);
        injector_value injected = injector(byte);
        mm->setByte(addr, injected);
        return { .original = byte, .injected = injected };
    }

    std::string memory_element::get_description() const {
            std::ostringstream os;
            os << "{ mem_elem for addr "
               << std::hex << std::showbase
               << get_offset()
               << " (mapped at="
               << (get_offset() + get_area()->get_offset())
               << ") } ";
            return os.str();
    }
}
