#define GP_IMPL(CLS,REF) \
    IMPL_CAP_WRAP(CLS,REF) \
    SAIL_FN_REG(CLS);

GP_IMPL(pc, zPCC);
GP_IMPL(sp, zx2);
GP_IMPL(ra, zx1);
GP_IMPL(gp ,zx3);
GP_IMPL(tp, zx4);
GP_IMPL(t0, zx5);
GP_IMPL(t1, zx6);
GP_IMPL(t2, zx7);
GP_IMPL(s0, zx8);
GP_IMPL(s1, zx9);
GP_IMPL(a0, zx10);
GP_IMPL(a1, zx11);
GP_IMPL(a2, zx12);
GP_IMPL(a3, zx13);
GP_IMPL(a4, zx14);
GP_IMPL(a5, zx15);
GP_IMPL(a6, zx16);
GP_IMPL(a7, zx17);
GP_IMPL(s2, zx18);
GP_IMPL(s3, zx19);
GP_IMPL(s4, zx20);
GP_IMPL(s5, zx21);
GP_IMPL(s6, zx22);
GP_IMPL(s7, zx23);
GP_IMPL(s8, zx24);
GP_IMPL(s9, zx25);
GP_IMPL(s10, zx26);
GP_IMPL(s11, zx27);
GP_IMPL(t3, zx28);
GP_IMPL(t4, zx29);
GP_IMPL(t5, zx30);
GP_IMPL(t6, zx31);
// special care must be taken, that these ids match the order in which the capability structures have been defined in gp_regs.hpp
namespace ids = sail_arch::registers;
SAIL_TAG_REG(regtags, ids::ra,
ids::sp,
ids::gp,
ids::tp,
ids::t0,
ids::t1,
ids::t2,
ids::s0,
ids::s1,
ids::a0,
ids::a1,
ids::a2,
ids::a3,
ids::a4,
ids::a5,
ids::a6,
ids::a7,
ids::s2,
ids::s3,
ids::s4,
ids::s5,
ids::s6,
ids::s7,
ids::s8,
ids::s9,
ids::s10,
ids::s11,
ids::t3,
ids::t4,
ids::t5,
ids::t6,
ids::pc,
ids::next_pc,
ids::ddc,
ids::utcc,
ids::utdc,
ids::uscratchc,
ids::uepcc,
ids::stcc,
ids::stdc,
ids::sscratchc,
ids::sepcc,
ids::mtcc,
ids::mtdc,
ids::mscratchc,
ids::mepcc);



