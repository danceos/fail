#include "../registers.hpp"

#ifndef GP
#define GP(CLS, ID) SAIL_DECLARE_REG_ID(CLS, ID, RT_GP, RT_TRACE)
#endif
#ifndef GP_CUSTOM
#define GP_CUSTOM(CLS, ID, ...) SAIL_DECLARE_REG_ID(CLS, ID, __VA_ARGS__)
#endif

namespace fail {
    namespace ids = sail_arch::registers;
    GP_CUSTOM(pc, ids::pc, RT_IP);
    GP_CUSTOM(sp, ids::sp, RT_ST, RT_TRACE);
    GP(ra, ids::ra);
    GP(gp ,ids::gp);
    GP(tp, ids::tp);
    GP(t0, ids::t0);
    GP(t1, ids::t1);
    GP(t2, ids::t2);
    GP(s0, ids::s0);
    GP(s1, ids::s1);
    GP(a0, ids::a0);
    GP(a1, ids::a1);
    GP(a2, ids::a2);
    GP(a3, ids::a3);
    GP(a4, ids::a4);
    GP(a5, ids::a5);
    GP(a6, ids::a6);
    GP(a7, ids::a7);
    GP(s2, ids::s2);
    GP(s3, ids::s3);
    GP(s4, ids::s4);
    GP(s5, ids::s5);
    GP(s6, ids::s6);
    GP(s7, ids::s7);
    GP(s8, ids::s8);
    GP(s9, ids::s9);
    GP(s10, ids::s10);
    GP(s11, ids::s11);
    GP(t3, ids::t3);
    GP(t4, ids::t4);
    GP(t5, ids::t5);
    GP(t6, ids::t6);

}
