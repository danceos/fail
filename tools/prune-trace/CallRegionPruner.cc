#include "CallRegionPruner.hpp"
#include "util/Logger.hpp"
#include <sstream>

static fail::Logger LOG ("CallRegionPruner");
using std::endl;


bool CallRegionPruner::importObjdump(const variant_t &variant){
    std::stringstream ss;
    ss << "SELECT instr_address, (char_length(opcode) DIV 2), disassemble  FROM objdump WHERE variant_id = "
       << variant.id
       << " ORDER BY instr_address";

    MYSQL_RES *res = db->query_stream(ss.str().c_str());
    assert(res && "Reading objdump failed");

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        static_instr_t pc   =  std::strtoul(row[0], 0, 10);
        instr_width_t width =  std::strtoul(row[1], 0, 10);
        std::string disas   = std::string(row[2]);
        this->instructions[pc] = width;

        // Record calls and returns
        if (strncmp(disas.c_str(), "call", 4) == 0
            || strncmp(disas.c_str(), "ret", 3) == 0
            || strncmp(disas.c_str(), "repz ret", 8) == 0) {
            this->call_or_ret.insert(pc);
        }
    }
    LOG << "objdump: " << this->instructions.size() << " instructions" << endl;
    LOG << "objdump: " << this->call_or_ret.size() << " calls/returns" << endl;

    if (this->instructions.size() == 0) {
        LOG << "ERROR: No objdump found" << std::endl;
        return false;
    }

    return true;
}

bool CallRegionPruner::inSameRegion(static_instr_t previous, static_instr_t next) {
    // We are not in the same region, if we have encountered a call or a return
    if (call_or_ret.count(previous) > 0) {
        if (BasicBlockPruner::inSameRegion(previous, next)) {
            LOG << "ERROR: every call-region break must also be a basic-block break" << std::endl;
        }
        return false;
    }
    return true;
}
