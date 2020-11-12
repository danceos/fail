#pragma once

#include "util/faultspace/fsa.hpp"
#include "Register.hpp"
#include <map>

namespace fail {
    using namespace fail::util::fsp;
    class CPUState;
    class CPUArchitecture;

    class register_area: public area {
        private:
            std::unique_ptr<CPUState> m_state;
            std::unique_ptr<CPUArchitecture>  m_arch;
            std::map<address_t, Register*> m_addr_to_reg;
            std::map<Register*, address_t> m_reg_to_addr;
            address_t m_size;
        public:
            std::unique_ptr<CPUState>& get_state() { return m_state; }
            std::unique_ptr<CPUArchitecture>& get_arch() { return m_arch; }

            using element_ptr = std::unique_ptr<element>;
            std::map<uint8_t, std::vector<element_ptr>> encode(size_t register_id);
            element_ptr encode(size_t register_id, unsigned byte_offset);
            virtual std::unique_ptr<element> decode(address_t addr) override;
            virtual const address_t get_size() const override  { return m_size; }

            register_area(std::unique_ptr<CPUState> state, std::unique_ptr<CPUArchitecture> arch);
    };

    class Register;
    class register_element: public element {
        private:
           Register* m_base_register;
           unsigned m_byte;

           Register* get_base() const { return m_base_register; }
        public:
           virtual injector_result inject(injector_fn injector) override;
           virtual std::string get_description() const override;
           register_element(area* a, Register* b, address_t addr, unsigned byte_offset);
    };
}


