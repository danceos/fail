#include "externs.h"

#define GP(CLS,ID) \
    DECLARE_CAP_WRAP(CLS); \
    SAIL_DECLARE_FN_REG_ID(CLS,ID,read_##CLS,write_##CLS,RT_TRACE,RT_GP)

#define GP_CUSTOM(CLS,ID,...) \
    DECLARE_CAP_WRAP(CLS); \
    SAIL_DECLARE_FN_REG_ID(CLS,ID,read_##CLS,write_##CLS,__VA_ARGS__)
#define CUSTOM_ENUMS

#include "enums.hpp"
#include "../riscv/gp_regs.hpp"

namespace fail {
SAIL_DECLARE_TAG_REG(regtags,struct zCapability*,
        &zx1,
        &zx2,
        &zx3,
        &zx4,
        &zx5,
        &zx6,
        &zx7,
        &zx8,
        &zx9,
        &zx10,
        &zx11,
        &zx12,
        &zx13,
        &zx14,
        &zx15,
        &zx16,
        &zx17,
        &zx18,
        &zx19,
        &zx20,
        &zx21,
        &zx22,
        &zx23,
        &zx24,
        &zx25,
        &zx26,
        &zx27,
        &zx28,
        &zx29,
        &zx30,
        &zx31,
        &zPCC,
        &znextPCC,
        &zDDC,
        &zUTCC,
        &zUTDC,
        &zUScratchC,
        &zUEPCC,
        &zSTCC,
        &zSTDC,
        &zSScratchC,
        &zSEPCC,
        &zMTCC,
        &zMTDC,
        &zMScratchC,
        &zMEPCC);
SAIL_SET_TAG(regtags);
}
