#pragma once

#include "util/faultspace/fsa.hpp"
#include <iostream>
#include <stdint.h>

namespace fail {
    using namespace fail::util::fsp;
    using std::vector;
    using std::unique_ptr;

    class memory_area;
    class memory_element: public element {
        public:
            virtual std::string get_description() const override;
            virtual injector_result inject(injector_fn injector) override;
            memory_element(memory_area* area, address_t addr);
    };

    class MemoryManager;
    class memory_area: public area {
        private:
            unique_ptr<MemoryManager> m_mm;
        public:
            unique_ptr<MemoryManager>& get_manager() { return m_mm; }

            virtual unique_ptr<element> decode(address_t addr) override;
            // for now allow a maximum of 4GB memory.
            virtual const address_t get_size() const override { return 0xFFFFFFFF; }

            template<typename Address>
            unique_ptr<element> encode(Address where) {
                address_t absaddr = static_cast<address_t>(where);
                auto e = std::make_unique<memory_element>(this, absaddr);
                return std::move(e);
            }

            // TODO: dynamic masks, which can change for each of the addresses between (from, to)
            template<typename Address>
            vector<unique_ptr<element>> encode(Address from, Address to, std::function<bool(Address)> filter) {
                std::vector<unique_ptr<element>> elems;
                std::cout << "[memarea] encode: from=" << std::hex << std::showbase << from << ", to=" << to << std::endl;
                for(auto it = from; it < to; ++it) {
                    if(filter(it)) {
                        std::cout << "[memarea] creating it=" << std::hex << std::showbase << it << std::endl;
                        elems.push_back(encode(it));
                    }
                }
                std::cout << "[memarea] created " << elems.size() <<" elements" << std::endl;
                return elems;
            }

            memory_area(unique_ptr<MemoryManager> mm, std::string name): area(name), m_mm(std::move(mm)) { }
    };

}

