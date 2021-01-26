#pragma once

#include <stdint.h>
#include <stddef.h>

extern "C" {

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

__attribute__((weak)) uint64_t zPC;
__attribute__((weak)) uint64_t znextPC;
__attribute__((weak)) uint64_t zinstbits;
__attribute__((weak)) zvectorz8deczCz0fbitsz832zCz0decz9z9 zXs;
__attribute__((weak)) uint64_t zx1;
__attribute__((weak)) uint64_t zx2;
__attribute__((weak)) uint64_t zx3;
__attribute__((weak)) uint64_t zx4;
__attribute__((weak)) uint64_t zx5;
__attribute__((weak)) uint64_t zx6;
__attribute__((weak)) uint64_t zx7;
__attribute__((weak)) uint64_t zx8;
__attribute__((weak)) uint64_t zx9;
__attribute__((weak)) uint64_t zx10;
__attribute__((weak)) uint64_t zx11;
__attribute__((weak)) uint64_t zx12;
__attribute__((weak)) uint64_t zx13;
__attribute__((weak)) uint64_t zx14;
__attribute__((weak)) uint64_t zx15;
__attribute__((weak)) uint64_t zx16;
__attribute__((weak)) uint64_t zx17;
__attribute__((weak)) uint64_t zx18;
__attribute__((weak)) uint64_t zx19;
__attribute__((weak)) uint64_t zx20;
__attribute__((weak)) uint64_t zx21;
__attribute__((weak)) uint64_t zx22;
__attribute__((weak)) uint64_t zx23;
__attribute__((weak)) uint64_t zx24;
__attribute__((weak)) uint64_t zx25;
__attribute__((weak)) uint64_t zx26;
__attribute__((weak)) uint64_t zx27;
__attribute__((weak)) uint64_t zx28;
__attribute__((weak)) uint64_t zx29;
__attribute__((weak)) uint64_t zx30;
__attribute__((weak)) uint64_t zx31;
__attribute__((weak)) enum zPrivilege zcur_privilege;
__attribute__((weak)) uint64_t zcur_inst;
__attribute__((weak)) struct zMisa zmisa;
__attribute__((weak)) struct zMstatus zmstatus;
__attribute__((weak)) struct zMinterrupts zmip;
__attribute__((weak)) struct zMinterrupts zmie;
__attribute__((weak)) struct zMinterrupts zmideleg;
__attribute__((weak)) struct zMedeleg zmedeleg;
__attribute__((weak)) struct zMtvec zmtvec;
__attribute__((weak)) struct zMcause zmcause;
__attribute__((weak)) uint64_t zmepc;
__attribute__((weak)) uint64_t zmtval;
__attribute__((weak)) uint64_t zmscratch;
__attribute__((weak)) struct zCounteren zmcounteren;
__attribute__((weak)) struct zCounteren zscounteren;
__attribute__((weak)) uint64_t zmcycle;
__attribute__((weak)) uint64_t zmtime;
__attribute__((weak)) uint64_t zminstret;
__attribute__((weak)) bool zminstret_written;
__attribute__((weak)) uint64_t zmvendorid;
__attribute__((weak)) uint64_t zmimpid;
__attribute__((weak)) uint64_t zmarchid;
__attribute__((weak)) uint64_t zmhartid;
__attribute__((weak)) struct zSedeleg zsedeleg;
__attribute__((weak)) struct zSinterrupts zsideleg;
__attribute__((weak)) struct zMtvec zstvec;
__attribute__((weak)) uint64_t zsscratch;
__attribute__((weak)) uint64_t zsepc;
__attribute__((weak)) struct zMcause zscause;
__attribute__((weak)) uint64_t zstval;
__attribute__((weak)) uint64_t ztselect;
__attribute__((weak)) struct zPmpcfg_ent zpmp0cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp1cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp2cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp3cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp4cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp5cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp6cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp7cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp8cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp9cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp10cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp11cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp12cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp13cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp14cfg;
__attribute__((weak)) struct zPmpcfg_ent zpmp15cfg;
__attribute__((weak)) uint64_t zpmpaddr0;
__attribute__((weak)) uint64_t zpmpaddr1;
__attribute__((weak)) uint64_t zpmpaddr2;
__attribute__((weak)) uint64_t zpmpaddr3;
__attribute__((weak)) uint64_t zpmpaddr4;
__attribute__((weak)) uint64_t zpmpaddr5;
__attribute__((weak)) uint64_t zpmpaddr6;
__attribute__((weak)) uint64_t zpmpaddr7;
__attribute__((weak)) uint64_t zpmpaddr8;
__attribute__((weak)) uint64_t zpmpaddr9;
__attribute__((weak)) uint64_t zpmpaddr10;
__attribute__((weak)) uint64_t zpmpaddr11;
__attribute__((weak)) uint64_t zpmpaddr12;
__attribute__((weak)) uint64_t zpmpaddr13;
__attribute__((weak)) uint64_t zpmpaddr14;
__attribute__((weak)) uint64_t zpmpaddr15;
__attribute__((weak)) struct zMtvec zutvec;
__attribute__((weak)) uint64_t zuscratch;
__attribute__((weak)) uint64_t zuepc;
__attribute__((weak)) struct zMcause zucause;
__attribute__((weak)) uint64_t zutval;
__attribute__((weak)) uint64_t zmtimecmp;
__attribute__((weak)) uint64_t zhtif_tohost;
__attribute__((weak)) bool zhtif_done;
__attribute__((weak)) uint64_t zhtif_exit_code;
// __attribute__((weak)) struct zoption ztlb32; //TODO: shouldnt be modified after init, otherwise should be saved by riscv
__attribute__((weak)) uint64_t zsatp;

__attribute__((weak)) uint64_t globalEntry;
__attribute__((weak)) void reinit_sail(uint64_t elf_entry);

}
