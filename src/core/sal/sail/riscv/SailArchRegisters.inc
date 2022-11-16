// See https://stackoverflow.com/questions/1643226/c-array-initialization-via-macro/1645599#1645599
// for passing initializer lists in macro argument
#define SAIL_CLASS_GP {RT_GP, RT_TRACE}

// 32 General-Purpose Registers
#define GP(id, ptr)  SAIL_REG_PTR(id, SAIL_XLEN, SAIL_CLASS_GP, ptr)
SAIL_REG_PTR(PC,  SAIL_XLEN, RT_IP, zPC)
GP(SP,  zx1)
GP(RA,  zx2)
GP(GP,  zx3)
GP(TP,  zx4)
GP(T0,  zx5)
GP(T1,  zx6)
GP(T2,  zx7)
GP(S0,  zx8)
GP(S1,  zx9)
GP(A0,  zx10)
GP(A1,  zx11)
GP(A2,  zx12)
GP(A3,  zx13)
GP(A4,  zx14)
GP(A5,  zx15)
GP(A6,  zx16)
GP(A7,  zx17)
GP(S2,  zx18)
GP(S3,  zx19)
GP(S4,  zx20)
GP(S5,  zx21)
GP(S6,  zx22)
GP(S7,  zx23)
GP(S8,  zx24)
GP(S9,  zx25)
GP(S10, zx26)
GP(S11, zx27)
GP(T3,  zx28)
GP(T4,  zx29)
GP(T5,  zx30)
GP(T6,  zx31)
#undef GP

// CPU Specific Registers
#define CSR(id, ptr) SAIL_REG_PTR(id, SAIL_XLEN, RT_CONTROL, ptr)
CSR(MINSTRET, zminstret)
CSR(MISA, zmisa.zMisa_chunk_0)
CSR(MSTATUS, zmstatus.zMstatus_chunk_0)
CSR(MIP, zmip.zMinterrupts_chunk_0)
CSR(MIE, zmie.zMinterrupts_chunk_0)
CSR(MIDELEG, zmideleg.zMinterrupts_chunk_0)
CSR(MEDELEG, zmedeleg.zMedeleg_chunk_0)
CSR(MTVEC, zmtvec.zMtvec_chunk_0)
CSR(MEPC, zmepc)
CSR(MTVAL, zmtval)
CSR(MSCRATCH, zmscratch)
CSR(MCYCLE, zmcycle)
CSR(MTIME, zmtime)
CSR(MVENDORID, zmvendorid)
CSR(MIMPID, zmimpid)
CSR(MARCHID, zmarchid)
CSR(MHARTID, zmhartid)
CSR(SSCRATCH, zsscratch)
CSR(SEPC, zsepc)
CSR(STVAL, zstval)
CSR(TSELECT, ztselect)
CSR(PMPADDR0, zpmpaddr0)
CSR(PMPADDR1, zpmpaddr1)
CSR(PMPADDR2, zpmpaddr2)
CSR(PMPADDR3, zpmpaddr3)
CSR(PMPADDR4, zpmpaddr4)
CSR(PMPADDR5, zpmpaddr5)
CSR(PMPADDR6, zpmpaddr6)
CSR(PMPADDR7, zpmpaddr7)
CSR(PMPADDR8, zpmpaddr8)
CSR(PMPADDR9, zpmpaddr9)
CSR(PMPADDR10, zpmpaddr10)
CSR(PMPADDR11, zpmpaddr11)
CSR(PMPADDR12, zpmpaddr12)
CSR(PMPADDR13, zpmpaddr13)
CSR(PMPADDR14, zpmpaddr14)
CSR(PMPADDR15, zpmpaddr15)
CSR(USCRATCH, zuscratch)
CSR(UEPC, zuepc)
CSR(UTVAL, zutval)
CSR(MTIMECMP, zmtimecmp)
CSR(SATP, zsatp)
CSR(MCOUNTEREN, zmcounteren.zCounteren_chunk_0)
CSR(SCOUNTEREN, zscounteren.zCounteren_chunk_0)
CSR(MCAUSE, zmcause.zMcause_chunk_0)
CSR(SEDELEG, zsedeleg.zSedeleg_chunk_0)
CSR(SIDELEG, zsideleg.zSinterrupts_chunk_0)
CSR(STVEC, zstvec.zMtvec_chunk_0)
CSR(SCAUSE, zscause.zMcause_chunk_0)
CSR(PMP0CFG, zpmp0cfg.zPmpcfg_ent_chunk_0)
CSR(PMP1CFG, zpmp1cfg.zPmpcfg_ent_chunk_0)
CSR(PMP2CFG, zpmp2cfg.zPmpcfg_ent_chunk_0)
CSR(PMP3CFG, zpmp3cfg.zPmpcfg_ent_chunk_0)
CSR(PMP4CFG, zpmp4cfg.zPmpcfg_ent_chunk_0)
CSR(PMP5CFG, zpmp5cfg.zPmpcfg_ent_chunk_0)
CSR(PMP6CFG, zpmp6cfg.zPmpcfg_ent_chunk_0)
CSR(PMP7CFG, zpmp7cfg.zPmpcfg_ent_chunk_0)
CSR(PMP8CFG, zpmp8cfg.zPmpcfg_ent_chunk_0)
CSR(PMP9CFG, zpmp9cfg.zPmpcfg_ent_chunk_0)
CSR(PMP10CFG, zpmp10cfg.zPmpcfg_ent_chunk_0)
CSR(PMP11CFG, zpmp11cfg.zPmpcfg_ent_chunk_0)
CSR(PMP12CFG, zpmp12cfg.zPmpcfg_ent_chunk_0)
CSR(PMP13CFG, zpmp13cfg.zPmpcfg_ent_chunk_0)
CSR(PMP14CFG, zpmp14cfg.zPmpcfg_ent_chunk_0)
CSR(PMP15CFG, zpmp15cfg.zPmpcfg_ent_chunk_0)
CSR(UTVEC, zutvec.zMtvec_chunk_0)
CSR(UCAUSE, zucause.zMcause_chunk_0)
#undef CSR

// Implementation Specific Registers
#define MODEL_SPECIFIC(id, ptr) SAIL_REG_PTR(id, SAIL_XLEN, RT_IMPL_SPECIFIC, ptr)
MODEL_SPECIFIC(HTIF_TOHOST,    zhtif_tohost)
MODEL_SPECIFIC(HTIF_EXIT_CODE, zhtif_exit_code)
MODEL_SPECIFIC(NEXT_PC,        znextPC)
MODEL_SPECIFIC(INSTBITS,       zinstbits)
MODEL_SPECIFIC(CUR_INST,       zcur_inst)
SAIL_REG_FN(MINSTRET_WRITTEN, SAIL_XLEN, RT_IMPL_SPECIFIC, minstret_written)
#undef MODEL_SPECIFIC
