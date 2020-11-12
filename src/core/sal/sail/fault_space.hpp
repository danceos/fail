#pragma once

#include <map>

#include "util/faultspace/fs.hpp"
#include "../MemoryArea.hpp"
#include "../RegisterArea.hpp"
#include "memory.hpp"
#include "tag_memory.hpp"
#include "arch_config.hpp"

namespace fail {
    using namespace util::fsp;

    class sail_fault_space: public space<sail_fault_space> {
        public:
            static std::vector<std::unique_ptr<area>> create_areas() {
                std::vector<std::unique_ptr<area>> a;
                auto mm = std::make_unique<SailMemoryManager>();
                auto tags = std::make_unique<SailTagMemoryManager>();
                auto state = std::make_unique<ConcreteCPU>();
                auto arch = std::make_unique<ConcreteCPU>();

                a.push_back(std::make_unique<memory_area>(std::move(mm), "memory0"));
                a.push_back(std::make_unique<memory_area>(std::move(tags), "memory1"));

                a.push_back(std::make_unique<register_area>(
                            std::move(state),std::move(arch)));

                return a;
            }
    };
}
