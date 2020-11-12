#include "../registers.hpp"
#include "externs.h"
#include "enums.hpp"

#define IMPL(CLS) SAIL_DECLARE_IMPL_REG(CLS)
#define IMPL_CAP(CLS, ID) \
    DECLARE_CAP_WRAP(CLS); \
    SAIL_DECLARE_FN_REG_ID(CLS,ID,read_##CLS,write_##CLS,RT_ARCH_SPECIFIC)

namespace fail {
    IMPL(htif_tohost);
    IMPL(htif_exit_code);
    // these could be moved to gp_regs.hpp
    IMPL(raw_pc);
    IMPL(raw_next_pc);
    //
    namespace ids = sail_arch::registers;
    IMPL_CAP(next_pc, ids::next_pc);
    IMPL(instbits);
    IMPL(cur_inst);
    extern regdata_t read_minstret_written();
    extern void write_minstret_written(regdata_t value);
    SAIL_DECLARE_FN_REG(minstret_written,read_minstret_written,write_minstret_written, RT_ARCH_SPECIFIC);
    extern regdata_t read_parity_mismatch();
    extern void write_parity_mismatch(regdata_t value);
    SAIL_DECLARE_FN_REG(parity_mismatch,read_parity_mismatch,write_parity_mismatch, RT_ARCH_SPECIFIC);
}
