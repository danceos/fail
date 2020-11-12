#ifndef SAILCONTROLLER_HPP
#define SAILCONTROLLER_HPP

#include "../SimulatorController.hpp"
#include "../SALConfig.hpp"
#include "memory.hpp"
#include "tag_memory.hpp"


namespace fail {

/**
 * \class SailController
 * Sail-specific implementation of a SimulatorController.
 */
class SailSimulator: public SimulatorController {
    private:
        enum class req_type: short {
            save, restore
        };
        struct request {
            const req_type t;
            const std::string path;
            ExperimentFlow* parent;
            request(const req_type t, const std::string p, ExperimentFlow* f): t(t),path(p),parent(f) {}
        };

        std::deque<request> requests = {};

        void queueRequest(req_type t, const std::string& p) {
            requests.emplace_back(t, p, m_Flows.getCurrent());
            m_Flows.resume();
        }

        void do_save(const std::string& base) ;
        void do_restore(const std::string& base);

        SailMemoryManager m_mem_manager;
        SailTagMemoryManager m_tag_manager;

    public:
        SailSimulator();
        ~SailSimulator();
        bool save(const std::string& path);
        void restore(const std::string& path);

        void reboot();

        // called by Sail wrapper code to execute any
        // pending save/restore requests
        void executeRequests();

        void checkTimers();

        // XXX: find a way to make this configurable
        // to be either a cpu specific value or a
        // device.
        // for now just use inst count of cpu 0
        simtime_t getTimerTicks();
        simtime_t getTimerTicksPerSecond();
};


}  // namespace fail

#endif /* SAILCONTROLLER_HPP */
