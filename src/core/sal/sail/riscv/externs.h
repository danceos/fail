#ifndef RISCVEXTERNS_H
#define RISCVEXTERNS_H

#include <stdint.h>
#include <stddef.h>

/**
 * Register used in sail-riscv.
 */
struct zvectorz8deczCz0fbitsz832zCz0decz9z9 {
  size_t len;
  uint64_t *data;
};
typedef struct zvectorz8deczCz0fbitsz832zCz0decz9z9
    zvectorz8deczCz0fbitsz832zCz0decz9z9;
enum zPrivilege { 
    zUser,
    zSupervisor,
    zMachine
};
struct zMisa {
    uint64_t zMisa_chunk_0;
};
struct zMstatus {
    uint64_t zMstatus_chunk_0;
};
struct zMinterrupts {
    uint64_t zMinterrupts_chunk_0;
};
struct zMedeleg {
    uint64_t zMedeleg_chunk_0;
};
struct zMtvec {
    uint64_t zMtvec_chunk_0;
};
struct zMcause {
    uint64_t zMcause_chunk_0;
};
struct zCounteren {
    uint64_t zCounteren_chunk_0;
};
struct zSedeleg {
    uint64_t zSedeleg_chunk_0;
};
struct zSinterrupts {
    uint64_t zSinterrupts_chunk_0;
};
struct zPmpcfg_ent {
    uint64_t zPmpcfg_ent_chunk_0;
};

extern "C" __attribute__((weak)) uint64_t zPC;
extern "C" __attribute__((weak)) uint64_t znextPC;
extern "C" __attribute__((weak)) uint64_t zinstbits;
extern "C" __attribute__((weak)) zvectorz8deczCz0fbitsz832zCz0decz9z9 zXs;
extern "C" __attribute__((weak)) uint64_t zx1;
extern "C" __attribute__((weak)) uint64_t zx2;
extern "C" __attribute__((weak)) uint64_t zx3;
extern "C" __attribute__((weak)) uint64_t zx4;
extern "C" __attribute__((weak)) uint64_t zx5;
extern "C" __attribute__((weak)) uint64_t zx6;
extern "C" __attribute__((weak)) uint64_t zx7;
extern "C" __attribute__((weak)) uint64_t zx8;
extern "C" __attribute__((weak)) uint64_t zx9;
extern "C" __attribute__((weak)) uint64_t zx10;
extern "C" __attribute__((weak)) uint64_t zx11;
extern "C" __attribute__((weak)) uint64_t zx12;
extern "C" __attribute__((weak)) uint64_t zx13;
extern "C" __attribute__((weak)) uint64_t zx14;
extern "C" __attribute__((weak)) uint64_t zx15;
extern "C" __attribute__((weak)) uint64_t zx16;
extern "C" __attribute__((weak)) uint64_t zx17;
extern "C" __attribute__((weak)) uint64_t zx18;
extern "C" __attribute__((weak)) uint64_t zx19;
extern "C" __attribute__((weak)) uint64_t zx20;
extern "C" __attribute__((weak)) uint64_t zx21;
extern "C" __attribute__((weak)) uint64_t zx22;
extern "C" __attribute__((weak)) uint64_t zx23;
extern "C" __attribute__((weak)) uint64_t zx24;
extern "C" __attribute__((weak)) uint64_t zx25;
extern "C" __attribute__((weak)) uint64_t zx26;
extern "C" __attribute__((weak)) uint64_t zx27;
extern "C" __attribute__((weak)) uint64_t zx28;
extern "C" __attribute__((weak)) uint64_t zx29;
extern "C" __attribute__((weak)) uint64_t zx30;
extern "C" __attribute__((weak)) uint64_t zx31;
extern "C" __attribute__((weak)) enum zPrivilege zcur_privilege;
extern "C" __attribute__((weak)) uint64_t zcur_inst;
extern "C" __attribute__((weak)) struct zMisa zmisa;
extern "C" __attribute__((weak)) struct zMstatus zmstatus;
extern "C" __attribute__((weak)) struct zMinterrupts zmip;
extern "C" __attribute__((weak)) struct zMinterrupts zmie;
extern "C" __attribute__((weak)) struct zMinterrupts zmideleg;
extern "C" __attribute__((weak)) struct zMedeleg zmedeleg;
extern "C" __attribute__((weak)) struct zMtvec zmtvec;
extern "C" __attribute__((weak)) struct zMcause zmcause;
extern "C" __attribute__((weak)) uint64_t zmepc;
extern "C" __attribute__((weak)) uint64_t zmtval;
extern "C" __attribute__((weak)) uint64_t zmscratch;
extern "C" __attribute__((weak)) struct zCounteren zmcounteren;
extern "C" __attribute__((weak)) struct zCounteren zscounteren;
extern "C" __attribute__((weak)) uint64_t zmcycle;
extern "C" __attribute__((weak)) uint64_t zmtime;
extern "C" __attribute__((weak)) uint64_t zminstret;
extern "C" __attribute__((weak)) bool zminstret_written;
extern "C" __attribute__((weak)) uint64_t zmvendorid;
extern "C" __attribute__((weak)) uint64_t zmimpid;
extern "C" __attribute__((weak)) uint64_t zmarchid;
extern "C" __attribute__((weak)) uint64_t zmhartid;
extern "C" __attribute__((weak)) struct zSedeleg zsedeleg;
extern "C" __attribute__((weak)) struct zSinterrupts zsideleg;
extern "C" __attribute__((weak)) struct zMtvec zstvec;
extern "C" __attribute__((weak)) uint64_t zsscratch;
extern "C" __attribute__((weak)) uint64_t zsepc;
extern "C" __attribute__((weak)) struct zMcause zscause;
extern "C" __attribute__((weak)) uint64_t zstval;
extern "C" __attribute__((weak)) uint64_t ztselect;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp0cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp1cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp2cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp3cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp4cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp5cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp6cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp7cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp8cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp9cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp10cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp11cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp12cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp13cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp14cfg;
extern "C" __attribute__((weak)) struct zPmpcfg_ent zpmp15cfg;
extern "C" __attribute__((weak)) uint64_t zpmpaddr0;
extern "C" __attribute__((weak)) uint64_t zpmpaddr1;
extern "C" __attribute__((weak)) uint64_t zpmpaddr2;
extern "C" __attribute__((weak)) uint64_t zpmpaddr3;
extern "C" __attribute__((weak)) uint64_t zpmpaddr4;
extern "C" __attribute__((weak)) uint64_t zpmpaddr5;
extern "C" __attribute__((weak)) uint64_t zpmpaddr6;
extern "C" __attribute__((weak)) uint64_t zpmpaddr7;
extern "C" __attribute__((weak)) uint64_t zpmpaddr8;
extern "C" __attribute__((weak)) uint64_t zpmpaddr9;
extern "C" __attribute__((weak)) uint64_t zpmpaddr10;
extern "C" __attribute__((weak)) uint64_t zpmpaddr11;
extern "C" __attribute__((weak)) uint64_t zpmpaddr12;
extern "C" __attribute__((weak)) uint64_t zpmpaddr13;
extern "C" __attribute__((weak)) uint64_t zpmpaddr14;
extern "C" __attribute__((weak)) uint64_t zpmpaddr15;
extern "C" __attribute__((weak)) struct zMtvec zutvec;
extern "C" __attribute__((weak)) uint64_t zuscratch;
extern "C" __attribute__((weak)) uint64_t zuepc;
extern "C" __attribute__((weak)) struct zMcause zucause;
extern "C" __attribute__((weak)) uint64_t zutval;
extern "C" __attribute__((weak)) uint64_t zmtimecmp;
extern "C" __attribute__((weak)) uint64_t zhtif_tohost;
extern "C" __attribute__((weak)) bool zhtif_done;
extern "C" __attribute__((weak)) uint64_t zhtif_exit_code;
extern "C" __attribute__((weak)) struct zoption ztlb32; //TODO: shouldnt be modified after init, otherwise should be saved by riscv
extern "C" __attribute__((weak)) uint64_t zsatp;

extern "C" __attribute__((weak)) uint64_t globalEntry;
extern "C" __attribute__((weak)) void reinit_sail(uint64_t elf_entry);

#endif /* RISCVEXTERNS_H */
