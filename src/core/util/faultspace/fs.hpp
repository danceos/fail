#pragma once

#include <map>
#include <algorithm>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>

#include "fsa.hpp"
#include "util.hpp"

namespace fail {
    namespace util {
        namespace fsp {
            using std::vector;
            using std::map;
            using std::unique_ptr;

            /**
             * This class is a fault space abstraction.
             * It contains one or more areas (filled by CRTP),
             * who themselves can encode or decode elements into or from
             * this fault space.
             * When subclassing, provide one static method in the subclass,
             * create_areas(), which returns a vector filled with initialized
             * areas.
             *
             * To generate an element, i.e. when importing a trace, subclass
             * fsp::area for your use case and provide a generation method and
             * a size for your fault space area. Finally pass the generated element
             * to this classes encode function to get an absolute fault space address.
             *
             * To decode a fault space address an element, i.e. when executing an
             * experiment, use the provided decode method, which automatically returns
             * an element which can be injected.
             *
             * Usually their should only be one, static instance of this class for
             * your architecture, so that consistent fault space addresses are used
             * across any import and/or experiment.
             *
             * NOTE: make sure all your areas mapped with this abstraction fit in
             * sizeof(address_t) (currently 64-Bit). Otherwise the generated map might
             * not work correctly and this abstraction will yield weird results.
             */
            template<class Derived>
                class space {
                    private:
                        map<address_t, unique_ptr<area>> m_areas;
                    public:
                        space() {
                            auto vec = Derived::create_areas();
                            address_t last = 0;
                            for(auto it = vec.begin(); it != vec.end(); ++it) {
                                address_t size = (*it)->get_size();
                                (*it)->set_offset(last);
                                std::cout << "[fsp] mapping " << (*it)->get_name() << " @ 0x" << std::hex << last << std::endl;
                                m_areas.emplace(last, std::move(*it));
                                last += size;
                            }
                        }
                        space(space& other) = delete;
                        space(const space& other) = delete;
                        space(space&& other) = delete;
                        space(const space&& other) = delete;
                        virtual ~space() { }
                        // return an area matching the provided name
                        // used to create elements in this area
                        unique_ptr<area>& get_area(const std::string& name) {
                            for(auto& kv: m_areas) {
                               if(kv.second->get_name() == name) { return kv.second; }
                            }
                            throw std::invalid_argument("couldn't find area with name: " + name);
                        }
                        // encode a newly created element to a unique fault space address
                        address_t encode(unique_ptr<element> e) {
                            const area* as = e->get_area();
                            address_t addr = as->encode(std::move(e));
                            return addr;

                        }
                        // recreate an element from a previously generated fault space address
                        unique_ptr<element> decode(address_t addr) {
                            auto upper_bound = m_areas.upper_bound(addr);
                            if(upper_bound == m_areas.begin()) {
                                throw std::invalid_argument("invalid address in fault space: too low");
                            }
                            auto& area = std::prev(upper_bound)->second;
                            std::cout << "decoding 0x" << std::hex << addr << " with area: " << area->get_name() << std::endl;
                            address_t relative = addr-area->get_offset();
                            return area->decode(relative);
                        }

                };
        }
    }
}
