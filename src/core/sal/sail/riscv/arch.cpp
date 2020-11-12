#include "externs.h"
#include "arch_reg.hpp"
#include "../registers.hpp"

namespace fail {
SAIL_REG(pc, zPC);
SAIL_REG(sp, zx2);
SAIL_REG(ra, zx1);
SAIL_REG(gp ,zx3);
SAIL_REG(tp, zx4);
SAIL_REG(t0, zx5);
SAIL_REG(t1, zx6);
SAIL_REG(t2, zx7);
SAIL_REG(s0, zx8);
SAIL_REG(s1, zx9);
SAIL_REG(a0, zx10);
SAIL_REG(a1, zx11);
SAIL_REG(a2, zx12);
SAIL_REG(a3, zx13);
SAIL_REG(a4, zx14);
SAIL_REG(a5, zx15);
SAIL_REG(a6, zx16);
SAIL_REG(a7, zx17);
SAIL_REG(s2, zx18);
SAIL_REG(s3, zx19);
SAIL_REG(s4, zx20);
SAIL_REG(s5, zx21);
SAIL_REG(s6, zx22);
SAIL_REG(s7, zx23);
SAIL_REG(s8, zx24);
SAIL_REG(s9, zx25);
SAIL_REG(s10, zx26);
SAIL_REG(s11, zx27);
SAIL_REG(t3, zx28);
SAIL_REG(t4, zx29);
SAIL_REG(t5, zx30);
SAIL_REG(t6, zx31);

SAIL_REG(minstret, zminstret);
SAIL_REG(misa, zmisa.zMisa_chunk_0);
SAIL_REG(mstatus, zmstatus.zMstatus_chunk_0);
SAIL_REG(mip, zmip.zMinterrupts_chunk_0);
SAIL_REG(mie, zmie.zMinterrupts_chunk_0);
SAIL_REG(mideleg, zmideleg.zMinterrupts_chunk_0);
SAIL_REG(medeleg, zmedeleg.zMedeleg_chunk_0);
SAIL_REG(mtvec, zmtvec.zMtvec_chunk_0);
SAIL_REG(mepc, zmepc);
SAIL_REG(mtval, zmtval);
SAIL_REG(mscratch, zmscratch);
SAIL_REG(mcycle, zmcycle);
SAIL_REG(mtime, zmtime);
SAIL_REG(mvendorid, zmvendorid);
SAIL_REG(mimpid, zmimpid);
SAIL_REG(marchid, zmarchid);
SAIL_REG(mhartid, zmhartid);
SAIL_REG(sscratch, zsscratch);
SAIL_REG(sepc, zsepc);
SAIL_REG(stval, zstval);
SAIL_REG(tselect, ztselect);
SAIL_REG(pmpaddr0, zpmpaddr0);
SAIL_REG(pmpaddr1, zpmpaddr1);
SAIL_REG(pmpaddr2, zpmpaddr2);
SAIL_REG(pmpaddr3, zpmpaddr3);
SAIL_REG(pmpaddr4, zpmpaddr4);
SAIL_REG(pmpaddr5, zpmpaddr5);
SAIL_REG(pmpaddr6, zpmpaddr6);
SAIL_REG(pmpaddr7, zpmpaddr7);
SAIL_REG(pmpaddr8, zpmpaddr8);
SAIL_REG(pmpaddr9, zpmpaddr9);
SAIL_REG(pmpaddr10, zpmpaddr10);
SAIL_REG(pmpaddr11, zpmpaddr11);
SAIL_REG(pmpaddr12, zpmpaddr12);
SAIL_REG(pmpaddr13, zpmpaddr13);
SAIL_REG(pmpaddr14, zpmpaddr14);
SAIL_REG(pmpaddr15, zpmpaddr15);
SAIL_REG(uscratch, zuscratch);
SAIL_REG(uepc, zuepc);
SAIL_REG(utval, zutval);
SAIL_REG(mtimecmp, zmtimecmp);
SAIL_REG(satp, zsatp);
SAIL_REG(mcounteren, zmcounteren.zCounteren_chunk_0);
SAIL_REG(scounteren, zscounteren.zCounteren_chunk_0);
SAIL_REG(mcause, zmcause.zMcause_chunk_0);
SAIL_REG(sedeleg, zsedeleg.zSedeleg_chunk_0);
SAIL_REG(sideleg, zsideleg.zSinterrupts_chunk_0);
SAIL_REG(stvec, zstvec.zMtvec_chunk_0);
SAIL_REG(scause, zscause.zMcause_chunk_0);
SAIL_REG(pmp0cfg, zpmp0cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp1cfg, zpmp1cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp2cfg, zpmp2cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp3cfg, zpmp3cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp4cfg, zpmp4cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp5cfg, zpmp5cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp6cfg, zpmp6cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp7cfg, zpmp7cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp8cfg, zpmp8cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp9cfg, zpmp9cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp10cfg, zpmp10cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp11cfg, zpmp11cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp12cfg, zpmp12cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp13cfg, zpmp13cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp14cfg, zpmp14cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(pmp15cfg, zpmp15cfg.zPmpcfg_ent_chunk_0);
SAIL_REG(utvec, zutvec.zMtvec_chunk_0);
SAIL_REG(ucause, zucause.zMcause_chunk_0);

SAIL_REG(htif_tohost, zhtif_tohost);
SAIL_REG(htif_exit_code, zhtif_exit_code);
SAIL_REG(next_pc, znextPC);
SAIL_REG(instbits, zinstbits);
SAIL_REG(cur_inst, zcur_inst);
regdata_t read_minstret_written() {
    return static_cast<regdata_t>(zminstret_written);
}

void write_minstret_written(regdata_t value) {
   zminstret_written = static_cast<bool>(value);
}
SAIL_FN_REG(minstret_written);
}
