#include "RegisterArea.hpp"
#include "CPUState.hpp"
#include "CPU.hpp"
#include "Register.hpp"
#include "SALInst.hpp"
#include <cmath> // std::ceil
#include <sstream>

namespace fail {
    using namespace fail::util::fsp;
    using element_ptr = register_area::element_ptr;

        std::map<uint8_t, std::vector<element_ptr>> register_area::encode(size_t register_id)
        {
            // query the correct register from CPUArchitecture
            Register* base = get_arch()->getRegister(register_id);
            unsigned width = base->getWidth();
            const unsigned byte_width = std::ceil(base->getWidth()/8.0);

            // create an fsp::element for every byte in the register
            std::map<uint8_t, std::vector<element_ptr>> elems;
            unsigned remaining = width;
            for(unsigned byte = 0; byte < byte_width; ++byte) {
                unsigned nbits = std::min(remaining,8u);
                remaining -= nbits;
                uint8_t mask = (1u << nbits) - 1;
                elems[mask].push_back(encode(register_id,byte));
            }

            assert(remaining == 0);

            return elems;
        }

        element_ptr register_area::encode(size_t register_id, unsigned byte_offset) {
            Register* base = get_arch()->getRegister(register_id);
            address_t offset = m_reg_to_addr[base];
            std::cout << "[RegisterArea] encoding register " << base->getName() << "(id=" << base->getId() << ")"
                      << " and length=" << base->getWidth()
                      << " chunk=" << byte_offset
                      << " @ " << std::hex << std::showbase
                      << offset
                      << std::dec
                      << std::endl;
            return std::make_unique<register_element>(this,base,offset,byte_offset);
        }


        std::unique_ptr<element> register_area::decode(address_t addr)
        {
            // find next Register that is mapped below this address.
            auto upper_bound = m_addr_to_reg.upper_bound(addr);
            if(upper_bound == m_addr_to_reg.begin()) {
                throw std::invalid_argument("invalid address in fault area: too low");
            }
            Register* r = std::prev(upper_bound)->second;
            // if we subtract this registers base address, we get the byte offset
            unsigned byte = addr - m_reg_to_addr[r];

            auto e = std::make_unique<register_element>(this,r,addr,byte);
            return std::move(e);
        }

        register_area::register_area(std::unique_ptr<CPUState> state, std::unique_ptr<CPUArchitecture> arch):
                area("register"), m_state(std::move(state)), m_arch(std::move(arch)) {
                    address_t offset = 0;
                    for(size_t i = 0; i < get_arch()->registerCount(); ++i) {
                        Register* r = get_arch()->getRegister(i);
                        m_addr_to_reg[offset] = r;
                        m_reg_to_addr[r] = offset;
                        offset += std::ceil(r->getWidth()/8.0);
                    }
                    m_size = offset;
                }

        register_element::register_element(area* a, Register *b, address_t addr, unsigned byte_offset):
            element(a,addr+byte_offset), m_base_register(b), m_byte(byte_offset)
        {
            assert(byte_offset < b->getWidth()/8);
        }

        injector_result register_element::inject(injector_fn injector)
        {
            register_area *area = dynamic_cast<register_area*>(get_area());
            assert(area != nullptr);
            auto& cpu = area->get_state();
            Register *base = get_base();

            regdata_t value = cpu->getRegisterContent(base);

            assert(sizeof(injector_value) == 1);

            injector_value original_byte = static_cast<injector_value>((value >> m_byte*8) & 0xFF);
            injector_value injected_byte = injector(original_byte);
            // mask out original byte
            regdata_t mask = ~(static_cast<regdata_t>(0xFF) << m_byte*8);
            regdata_t injected_value = value & mask;
            // and update with injected version
            injected_value |= static_cast<regdata_t>(injected_byte) << m_byte*8;

            cpu->setRegisterContent(base, injected_value);

            std::cout << "[register_element] injecting register " << base->getName() << "at offset " << std::dec << m_byte << std::endl
                << std::hex << std::showbase
                << '\t' << "previous value: " << value << std::endl
                << '\t' << "injected value: " << injected_value << std::endl
                << std::dec << std::noshowbase;

            return { .original = original_byte, .injected = injected_byte };
        }

        std::string register_element::get_description() const {
            std::ostringstream os;
            os << "{ reg_elem '"
               << get_base()->getName()
               << "' at byte "
               << std::hex << std::showbase
               << m_byte
               << " (mapped @ " << get_area()->get_offset() << "+"<< get_offset()
               << " ->" << (get_offset() + get_area()->get_offset())
               << ") } ";
            return os.str();
        }

}
