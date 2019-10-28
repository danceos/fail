#ifndef __CALL_REGION_PRUNER_H__
#define __CALL_REGION_PRUNER_H__

#include "BasicBlockPruner.hpp"
#include <list>
#include "util/ProtoStream.hpp"
#include "util/gzstream/gzstream.h"
#include "comm/TracePlugin.pb.h"
#include "sal/SALConfig.hpp"
#include <tuple>
#include <string>

class CallRegionPruner : public BasicBlockPruner {
public:
    CallRegionPruner() : BasicBlockPruner() {
        LOG.setDescription("CallRegionPruner");
    }
    virtual ~CallRegionPruner() {};

    virtual std::string method_name() { return std::string("call-region"); }
    void getAliases(std::deque<std::string> *aliases) {
        aliases->push_back("CallRegionPruner");
        aliases->push_back("call-region");
    }

protected:
    /*  Import objectdump to find jumps */
    virtual bool importObjdump(const variant_t &variant);

    // list of instructions: PC -> is a call or ret instruction
    std::set<static_instr_t> call_or_ret;

    virtual bool inSameRegion(static_instr_t previous, static_instr_t next);
};
#endif
