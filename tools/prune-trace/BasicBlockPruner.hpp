#ifndef __BASIC_BLOCK_PRUNER_H__
#define __BASIC_BLOCK_PRUNER_H__

#include "Pruner.hpp"
#include <list>
#include "util/ProtoStream.hpp"
#include "util/gzstream/gzstream.h"
#include "comm/TracePlugin.pb.h"
#include "sal/SALConfig.hpp"
#include <functional>



class BasicBlockPruner : public Pruner {
protected:
    fail::Logger LOG;

public:
    BasicBlockPruner() : LOG("BasicBlockPruner") {}
    virtual ~BasicBlockPruner() {};

    virtual std::string method_name() { return std::string("basic-block"); }
    virtual bool prune_all();
    void getAliases(std::deque<std::string> *aliases) {
        aliases->push_back("BasicBlockPruner");
        aliases->push_back("basic-block");
    }

    virtual bool create_database();
    virtual bool clear_database();

    using variant_t = fail::Database::Variant;
    using dynamic_instr_t = unsigned int;
    using dynamic_time_t  = fail::simtime_t;
    using static_instr_t  = unsigned long;
    using instr_width_t  = unsigned char;

    struct trace_event_t {
        static_instr_t  static_instr;
        dynamic_instr_t dynamic_instr;
        dynamic_time_t  dynamic_time;
    };

    struct region {
        // Init a basic block
        region(unsigned id, const trace_event_t &event)
            : id(id),
              begin(event),
              instruction_count(0) { }


        // Basic block id
        unsigned id;
        trace_event_t begin;  // first one is included
        trace_event_t end;    // last one is included
        unsigned instruction_count = 0;

        /*  Set the end of the basic block */
        void appendInstruction(const trace_event_t &obj){
            end = obj;
            instruction_count++;
        }
    };

    /*  Import objectdump to find jumps */
    virtual bool importObjdump(const variant_t &variant);

    // list of instructions: PC -> width_of_instruction
    std::map<static_instr_t, instr_width_t> instructions;

    // find leader start adresses
    bool findRegionLeaders();

    virtual bool inSameRegion(static_instr_t previous, static_instr_t next) {
        return previous + instructions.at(previous) == next;
    }

    // Instruction addresses of region leaders.
    std::set<static_instr_t> leaders;


    // find and add regions
    bool findRegions(const variant_t& variant);

    // Map: Leader instruction -> static region
    std::list<region> regions;

    // Insert regions into mysql
    bool exportRegions(const variant_t& variant);
};


// Some printers
std::ostream& operator<<(std::ostream& os, const BasicBlockPruner::region &);
std::ostream& operator<<(std::ostream& os, const BasicBlockPruner::trace_event_t &);


#endif
