#include "simulator.hpp"
#include "../Listener.hpp"
#include <fstream>
#include "cpu.hpp"

namespace fail {
    SailSimulator::SailSimulator(): SimulatorController(), m_mem_manager(), m_tag_manager() {
        setMemoryManager(&m_mem_manager);
        addCPU(new ConcreteCPU(0));
    }

    SailSimulator::~SailSimulator() {
        auto it = m_CPUs.begin();
        while (it != m_CPUs.end()) {
            delete *it;
            it = m_CPUs.erase(it);
        }
    }

    void SailSimulator::do_save(const std::string& base) {
            // create base directory if it does not exist
            // XXX: c++17 use <fs>
            assert(base.size() > 0 && "FATAL ERROR: tried to save state without valid path");
            std::string mkdirState("mkdir -p ");
            mkdirState.append(base);
            int status = system(mkdirState.c_str());
            if (status == -1) {
                throw std::runtime_error("[FAIL] Error: do_save() directory creation failed");
            }
            // serialize registers
            std::string reg_base = base + "/registers";
            for(const auto c: m_CPUs) {
                std::ofstream f { reg_base + std::to_string(c->getId())};
                if (!f.is_open()) throw std::runtime_error("[FAIL] couldnt open register file for writing");
                c->serialize(f);
            }
            // serialize memory
            std::ofstream fm { base + "/memory", std::ios::binary };
            if (!fm.is_open()) throw std::runtime_error("[FAIL] couldnt open memory file for writing");
            m_mem_manager.serialize(fm);
            std::ofstream ft { base + "/tags", std::ios::binary };
            if (!ft.is_open()) throw std::runtime_error("[FAIL] couldnt open tag file for writing");
            m_tag_manager.serialize(ft);
    }
    void SailSimulator::do_restore(const std::string& base) {
            // unserialize registers
            std::string reg_base = base + "/registers";
            for( const auto c: m_CPUs ) {
                std::ifstream f { reg_base + std::to_string(c->getId()) };
                if (!f.is_open()) throw std::runtime_error("[FAIL] couldnt open register file for reading");
                c->unserialize(f);
            }
            // unserialize memory
            std::ifstream fm { base + "/memory", std::ios::binary };
            if (!fm.is_open()) throw std::runtime_error("[FAIL] couldnt open memory file for reading");
            m_mem_manager.unserialize(fm);

            std::ifstream ft { base + "/tags", std::ios::binary };
            if (!ft.is_open()) throw std::runtime_error("[FAIL] couldnt open tag file for reading");
            m_tag_manager.unserialize(ft);

    }


    bool SailSimulator::save(const std::string& path) {
#if defined(CONFIG_SR_SAVE)
            queueRequest(req_type::save, path);
            return true;
#else
            return false;
#endif
    }

    void SailSimulator::restore(const std::string& path) {
#if defined(CONFIG_SR_RESTORE)
            clearListeners();
            queueRequest(req_type::restore, path);
#endif
    }
        void SailSimulator::reboot() {
            dynamic_cast<SailMemoryManager*>(m_Mem)->reset();
            for(const auto c: m_CPUs)
                c->reset();
        }

        void SailSimulator::executeRequests() {
            while(!requests.empty()) {
                auto e = requests.front();
                requests.pop_front();
                if(e.t == req_type::save)
                    do_save(e.path);
                else if(e.t == req_type::restore)
                    do_restore(e.path);
                m_Flows.toggle(e.parent);
            }
        }

        void SailSimulator::checkTimers() {
            // XXX: see getTimerTicks()
            // for now just use inst count of CPU0
            const auto ticks = getCPU(0).getInstructionCounter();

            ListenerManager::iterator it = m_LstList.begin();
            while(it != m_LstList.end()) {
                BaseListener* pev = *it;
                if(auto t = dynamic_cast<TimerListener*>(pev)) {
                    if(t->getTimeout() <= ticks) {
                        std::cout << "timer has gone off" << std::endl;
                        it = m_LstList.makeActive(it);
                        continue;
                    }
                }
                it++;
            }
            m_LstList.triggerActiveListeners();
        }

        simtime_t SailSimulator::getTimerTicks() { return getCPU(0).getInstructionCounter(); }
        simtime_t SailSimulator::getTimerTicksPerSecond() { return getCPU(0).getFrequency(); }
}
