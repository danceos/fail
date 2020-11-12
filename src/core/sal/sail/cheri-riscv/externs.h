#ifndef cheri_riscv_externs_h
#define cheri_riscv_externs_h

#include <stdint.h>
#include <stddef.h>
#include <cassert>
#include "config/VariantConfig.hpp"

struct zCapability {
  uint64_t zB;
  uint64_t zE;
  uint64_t zT;
  bool zaccess_system_regs;
  uint64_t zaddress;
  bool zflag_cap_mode;
  bool zglobal;
  bool zinternal_e;
  uint64_t zotype;
  bool zpermit_ccall;
  bool zpermit_execute;
  bool zpermit_load;
  bool zpermit_load_cap;
  bool zpermit_seal;
  bool zpermit_set_CID;
  bool zpermit_store;
  bool zpermit_store_cap;
  bool zpermit_store_local_cap;
  bool zpermit_unseal;
  uint64_t zreserved;
  bool zsealed;
  bool ztag;
  uint64_t zuperms;
};

#define DECLARE_CAP_WRAP(CLS) \
    extern regdata_t read_##CLS(); \
    extern void write_##CLS(regdata_t)

#if defined(BUILD_64BIT)

#include "gmp.h"

extern "C" {
    typedef struct {
    mp_bitcnt_t len;
    mpz_t * bits;
} lbits;
}

extern "C" __attribute__((weak)) struct zCapability zmemBitsToCapability(bool, lbits);
extern "C" __attribute__((weak)) void zcapToMemBits(lbits*, struct zCapability);
extern "C" __attribute__((weak)) void create_lbits(lbits*);

#define IMPL_CAP_WRAP(CLS,REF) \
    regdata_t read_##CLS() { \
        lbits c;\
        create_lbits(&c); \
        zcapToMemBits(&c, REF); \
        unsigned __int128 hi,lo; \
        lo = mpz_get_ui(*c.bits); \
        mpz_cdiv_q_2exp(*c.bits,*c.bits,64); \
        hi = mpz_get_ui(*c.bits); \
        return hi << 64 | lo; \
    } \
    void write_##CLS(regdata_t val) { \
        lbits c;\
        create_lbits(&c); \
        uint64_t hi,lo; \
        hi = (val >> 64) & UINT64_MAX; \
        lo = val & UINT64_MAX; \
        mpz_set_ui(*c.bits, hi); \
        mpz_mul_2exp(*c.bits, *c.bits, 64); \
        mpz_add_ui(*c.bits, *c.bits, lo); \
        c.len = 128; \
        REF = zmemBitsToCapability(REF.ztag, c); \
    }
#else
extern "C" __attribute__((weak)) struct zCapability zmemBitsToCapability(bool, uint64_t);
extern "C" __attribute__((weak)) uint64_t zcapToMemBits(struct zCapability);

#define IMPL_CAP_WRAP(CLS,REF) \
    regdata_t read_##CLS() { \
        return static_cast<regdata_t>(zcapToMemBits(REF)); \
    } \
    void write_##CLS(regdata_t val) { \
        REF = zmemBitsToCapability(REF.ztag, static_cast<uint64_t>(val)); \
    }
#endif


extern "C" __attribute__((weak)) struct zCapability zx1;

// register x2
extern "C" __attribute__((weak)) struct zCapability zx2;

// register x3
extern "C" __attribute__((weak)) struct zCapability zx3;

// register x4
extern "C" __attribute__((weak)) struct zCapability zx4;

// register x5
extern "C" __attribute__((weak)) struct zCapability zx5;

// register x6
extern "C" __attribute__((weak)) struct zCapability zx6;

// register x7
extern "C" __attribute__((weak)) struct zCapability zx7;

// register x8
extern "C" __attribute__((weak)) struct zCapability zx8;

// register x9
extern "C" __attribute__((weak)) struct zCapability zx9;

// register x10
extern "C" __attribute__((weak)) struct zCapability zx10;

// register x11
extern "C" __attribute__((weak)) struct zCapability zx11;

// register x12
extern "C" __attribute__((weak)) struct zCapability zx12;

// register x13
extern "C" __attribute__((weak)) struct zCapability zx13;

// register x14
extern "C" __attribute__((weak)) struct zCapability zx14;

// register x15
extern "C" __attribute__((weak)) struct zCapability zx15;

// register x16
extern "C" __attribute__((weak)) struct zCapability zx16;

// register x17
extern "C" __attribute__((weak)) struct zCapability zx17;

// register x18
extern "C" __attribute__((weak)) struct zCapability zx18;

// register x19
extern "C" __attribute__((weak)) struct zCapability zx19;

// register x20
extern "C" __attribute__((weak)) struct zCapability zx20;

// register x21
extern "C" __attribute__((weak)) struct zCapability zx21;

// register x22
extern "C" __attribute__((weak)) struct zCapability zx22;

// register x23
extern "C" __attribute__((weak)) struct zCapability zx23;

// register x24
extern "C" __attribute__((weak)) struct zCapability zx24;

// register x25
extern "C" __attribute__((weak)) struct zCapability zx25;

// register x26
extern "C" __attribute__((weak)) struct zCapability zx26;

// register x27
extern "C" __attribute__((weak)) struct zCapability zx27;

// register x28
extern "C" __attribute__((weak)) struct zCapability zx28;

// register x29
extern "C" __attribute__((weak)) struct zCapability zx29;

// register x30
extern "C" __attribute__((weak)) struct zCapability zx30;

// register x31
extern "C" __attribute__((weak)) struct zCapability zx31;

// register PC
extern "C" __attribute__((weak)) uint64_t zPC;

// register nextPC
extern "C" __attribute__((weak)) uint64_t znextPC;

// register PCC
extern "C" __attribute__((weak)) struct zCapability zPCC;

// register nextPCC
extern "C" __attribute__((weak)) struct zCapability znextPCC;

// register DDC
extern "C" __attribute__((weak)) struct zCapability zDDC;

// register UTCC
extern "C" __attribute__((weak)) struct zCapability zUTCC;

// register UTDC
extern "C" __attribute__((weak)) struct zCapability zUTDC;

// register UScratchC
extern "C" __attribute__((weak)) struct zCapability zUScratchC;

// register UEPCC
extern "C" __attribute__((weak)) struct zCapability zUEPCC;

// register STCC
extern "C" __attribute__((weak)) struct zCapability zSTCC;

// register STDC
extern "C" __attribute__((weak)) struct zCapability zSTDC;

// register SScratchC
extern "C" __attribute__((weak)) struct zCapability zSScratchC;

// register SEPCC
extern "C" __attribute__((weak)) struct zCapability zSEPCC;

// register MTCC
extern "C" __attribute__((weak)) struct zCapability zMTCC;

// register MTDC
extern "C" __attribute__((weak)) struct zCapability zMTDC;

// register MScratchC
extern "C" __attribute__((weak)) struct zCapability zMScratchC;

// register MEPCC
extern "C" __attribute__((weak)) struct zCapability zMEPCC;

/**
 * Register used in sail-riscv.
 */
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

extern "C" __attribute__((weak)) uint64_t zinstbits;
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
extern "C" __attribute__((weak)) bool zparity_mismatch;

extern "C" __attribute__((weak)) uint64_t globalEntry;
extern "C" __attribute__((weak)) void reinit_sail(uint64_t elf_entry);

#endif
