
#ifndef riscv_arch_cpu_h
#define riscv_arch_cpu_h

#include "arch_reg.hpp"
#include "../cpu.hpp"

namespace fail {
    SAIL_SET_PC(pc);
    SAIL_SET_SP(sp);
    SAIL_SET_INSTCOUNT(minstret);

    SAIL_DECLARE_CPU(0x8000000, sp, pc, minstret, ra,gp,tp,t0,t1,t2,s0,s1,a0,a1,a2,a3,a4,a5,a6,a7,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,t3,t4,t5,t6,misa ,mstatus,mip,mie,mideleg,medeleg,mtvec,mepc,mtval,mscratch,mcycle,mtime,mvendorid,mimpid,marchid,mhartid,sscratch,sepc,stval,tselect,pmpaddr0,pmpaddr1,pmpaddr2,pmpaddr3,pmpaddr4,pmpaddr5,pmpaddr6,pmpaddr7,pmpaddr8,pmpaddr9,pmpaddr10,pmpaddr11,pmpaddr12,pmpaddr13,pmpaddr14,pmpaddr15,uscratch,uepc,utval,mtimecmp,satp,mcounteren,scounteren,mcause,sedeleg,sideleg,stvec,scause,pmp0cfg,pmp1cfg,pmp2cfg,pmp3cfg,pmp4cfg,pmp5cfg,pmp6cfg,pmp7cfg,pmp8cfg,pmp9cfg,pmp10cfg,pmp11cfg,pmp12cfg,pmp13cfg,pmp14cfg,pmp15cfg,utvec,ucause,htif_tohost,htif_exit_code,next_pc,instbits,cur_inst,minstret_written);
}
#endif
