#include "../registers.hpp"

#define IMPL(CLS) SAIL_DECLARE_IMPL_REG(CLS)

namespace fail {
    IMPL(htif_tohost);
    IMPL(htif_exit_code);
    IMPL(next_pc);
    IMPL(instbits);
    IMPL(cur_inst);
    extern regdata_t read_minstret_written();
    extern void write_minstret_written(regdata_t value);
    SAIL_DECLARE_FN_REG(minstret_written,read_minstret_written,write_minstret_written, RT_ARCH_SPECIFIC);
}
