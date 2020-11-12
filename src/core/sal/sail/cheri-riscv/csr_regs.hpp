#include "../riscv/csr_regs.hpp"
#include "enums.hpp"

#define CSR_CAP(CLS,ID) \
    DECLARE_CAP_WRAP(CLS); \
    SAIL_DECLARE_FN_REG_ID(CLS,ID,read_##CLS,write_##CLS,RT_CONTROL)

namespace fail {
    namespace ids = sail_arch::registers;
    CSR_CAP(ddc,ids::ddc);
    CSR_CAP(utcc, ids::utcc);
    CSR_CAP(utdc, ids::utdc);
    CSR_CAP(uscratchc, ids::uscratchc);
    CSR_CAP(uepcc, ids::uepcc);
    CSR_CAP(stcc, ids::stcc);
    CSR_CAP(stdc, ids::stdc);
    CSR_CAP(sscratchc, ids::sscratchc);
    CSR_CAP(sepcc, ids::sepcc);
    CSR_CAP(mtcc, ids::mtcc);
    CSR_CAP(mtdc, ids::mtdc);
    CSR_CAP(mscratchc, ids::mscratchc);
    CSR_CAP(mepcc, ids::mepcc);
}
