/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2001-2011  The Bochs Project
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA B 02110-1301 USA
//
/////////////////////////////////////////////////////////////////////////

#define NEED_CPU_REG_SHORTCUTS 1
#include "bochs.h"
#include "cpu.h"
#define LOG_THIS BX_CPU_THIS_PTR

#if BX_SUPPORT_X86_64

///////////////////////////
// prefix bytes
// opcode bytes
// modrm/sib
// address displacement
// immediate constant
///////////////////////////

// The table for 64-bit is slightly different from the
// table for 32-bit due to undefined opcodes, which
// were valid in 32-bit mode

#define X 0 /* undefined opcode */

static const Bit8u BxOpcodeHasModrm64[512] = {
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f          */
  /*       -------------------------------          */
  /* 00 */ 1,1,1,1,0,0,X,X,1,1,1,1,0,0,X,X,
  /* 10 */ 1,1,1,1,0,0,X,X,1,1,1,1,0,0,X,X,
  /* 20 */ 1,1,1,1,0,0,X,X,1,1,1,1,0,0,X,X,
  /* 30 */ 1,1,1,1,0,0,X,X,1,1,1,1,0,0,X,X,
  /* 40 */ X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,
  /* 50 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 60 */ X,X,X,1,X,X,X,X,0,1,0,1,0,0,0,0,
  /* 70 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 80 */ 1,1,X,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 90 */ 0,0,0,0,0,0,0,0,0,0,X,0,0,0,0,0,
  /* A0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* B0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* C0 */ 1,1,0,0,X,X,1,1,0,0,0,0,0,0,X,0,
  /* D0 */ 1,1,1,1,X,X,X,0,1,1,1,1,1,1,1,1,
  /* E0 */ 0,0,0,0,0,0,0,0,0,0,X,0,0,0,0,0,
  /* F0 */ X,0,X,X,0,0,1,1,0,0,0,0,0,0,1,1,
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f           */
  /*       -------------------------------           */
           1,1,1,1,X,0,0,0,0,0,X,0,X,1,0,1, /* 0F 00 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F 10 */
           1,1,1,1,X,X,X,X,1,1,1,1,1,1,1,1, /* 0F 20 */
           0,0,0,0,X,X,X,X,1,X,1,X,X,X,X,X, /* 0F 30 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F 40 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F 50 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F 60 */
           1,1,1,1,1,1,1,0,1,1,X,X,1,1,1,1, /* 0F 70 */
           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0F 80 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F 90 */
           0,0,0,1,1,1,X,X,0,0,0,1,1,1,1,1, /* 0F A0 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F B0 */
           1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, /* 0F C0 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F D0 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F E0 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,X  /* 0F F0 */
  /*       -------------------------------           */
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f           */
};

#undef X

// Segment override prefixes
// -------------------------
// In 64-bit mode the CS, DS, ES, and SS segment overrides are ignored.

// decoding instructions; accessing seg reg's by index
static unsigned sreg_mod0_base32[16] = {
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_SS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS
};

static unsigned sreg_mod1or2_base32[16] = {
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_SS,
  BX_SEG_REG_SS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS,
  BX_SEG_REG_DS
};

// common fetchdecode32/64 opcode tables
#include "fetchdecode.h"

// table of all Bochs opcodes
extern struct bxIAOpcodeTable BxOpcodesTable[];

// 512 entries for 16bit operand size
// 512 entries for 32bit operand size
// 512 entries for 64bit operand size

static const BxOpcodeInfo_t BxOpcodeInfo64[512*3] = {
  // 512 entries for 16bit mode
  /* 00 /w */ { BxArithDstRM | BxLockable, BX_IA_ADD_EbGb },
  /* 01 /w */ { BxArithDstRM | BxLockable, BX_IA_ADD_EwGw },
  /* 02 /w */ { 0, BX_IA_ADD_GbEb },
  /* 03 /w */ { 0, BX_IA_ADD_GwEw },
  /* 04 /w */ { BxImmediate_Ib, BX_IA_ADD_ALIb },
  /* 05 /w */ { BxImmediate_Iw, BX_IA_ADD_AXIw },
  /* 06 /w */ { 0, BX_IA_ERROR },
  /* 07 /w */ { 0, BX_IA_ERROR },
  /* 08 /w */ { BxArithDstRM | BxLockable, BX_IA_OR_EbGb },
  /* 09 /w */ { BxArithDstRM | BxLockable, BX_IA_OR_EwGw },
  /* 0A /w */ { 0, BX_IA_OR_GbEb },
  /* 0B /w */ { 0, BX_IA_OR_GwEw },
  /* 0C /w */ { BxImmediate_Ib, BX_IA_OR_ALIb },
  /* 0D /w */ { BxImmediate_Iw, BX_IA_OR_AXIw },
  /* 0E /w */ { 0, BX_IA_ERROR },
  /* 0F /w */ { 0, BX_IA_ERROR }, // 2-byte escape
  /* 10 /w */ { BxArithDstRM | BxLockable, BX_IA_ADC_EbGb },
  /* 11 /w */ { BxArithDstRM | BxLockable, BX_IA_ADC_EwGw },
  /* 12 /w */ { 0, BX_IA_ADC_GbEb },
  /* 13 /w */ { 0, BX_IA_ADC_GwEw },
  /* 14 /w */ { BxImmediate_Ib, BX_IA_ADC_ALIb },
  /* 15 /w */ { BxImmediate_Iw, BX_IA_ADC_AXIw },
  /* 16 /w */ { 0, BX_IA_ERROR },
  /* 17 /w */ { 0, BX_IA_ERROR },
  /* 18 /w */ { BxArithDstRM | BxLockable, BX_IA_SBB_EbGb },
  /* 19 /w */ { BxArithDstRM | BxLockable, BX_IA_SBB_EwGw },
  /* 1A /w */ { 0, BX_IA_SBB_GbEb },
  /* 1B /w */ { 0, BX_IA_SBB_GwEw },
  /* 1C /w */ { BxImmediate_Ib, BX_IA_SBB_ALIb },
  /* 1D /w */ { BxImmediate_Iw, BX_IA_SBB_AXIw },
  /* 1E /w */ { 0, BX_IA_ERROR },
  /* 1F /w */ { 0, BX_IA_ERROR },
  /* 20 /w */ { BxArithDstRM | BxLockable, BX_IA_AND_EbGb },
  /* 21 /w */ { BxArithDstRM | BxLockable, BX_IA_AND_EwGw },
  /* 22 /w */ { 0, BX_IA_AND_GbEb },
  /* 23 /w */ { 0, BX_IA_AND_GwEw },
  /* 24 /w */ { BxImmediate_Ib, BX_IA_AND_ALIb },
  /* 25 /w */ { BxImmediate_Iw, BX_IA_AND_AXIw },
  /* 26 /w */ { 0, BX_IA_ERROR }, // ES:
  /* 27 /w */ { 0, BX_IA_ERROR },
  /* 28 /w */ { BxArithDstRM | BxLockable, BX_IA_SUB_EbGb },
  /* 29 /w */ { BxArithDstRM | BxLockable, BX_IA_SUB_EwGw },
  /* 2A /w */ { 0, BX_IA_SUB_GbEb },
  /* 2B /w */ { 0, BX_IA_SUB_GwEw },
  /* 2C /w */ { BxImmediate_Ib, BX_IA_SUB_ALIb },
  /* 2D /w */ { BxImmediate_Iw, BX_IA_SUB_AXIw },
  /* 2E /w */ { 0, BX_IA_ERROR }, // CS:
  /* 2F /w */ { 0, BX_IA_ERROR },
  /* 30 /w */ { BxArithDstRM | BxLockable, BX_IA_XOR_EbGb },
  /* 31 /w */ { BxArithDstRM | BxLockable, BX_IA_XOR_EwGw },
  /* 32 /w */ { 0, BX_IA_XOR_GbEb },
  /* 33 /w */ { 0, BX_IA_XOR_GwEw },
  /* 34 /w */ { BxImmediate_Ib, BX_IA_XOR_ALIb },
  /* 35 /w */ { BxImmediate_Iw, BX_IA_XOR_AXIw },
  /* 36 /w */ { 0, BX_IA_ERROR }, // SS:
  /* 37 /w */ { 0, BX_IA_ERROR },
  /* 38 /w */ { BxArithDstRM, BX_IA_CMP_EbGb },
  /* 39 /w */ { BxArithDstRM, BX_IA_CMP_EwGw },
  /* 3A /w */ { 0, BX_IA_CMP_GbEb },
  /* 3B /w */ { 0, BX_IA_CMP_GwEw },
  /* 3C /w */ { BxImmediate_Ib, BX_IA_CMP_ALIb },
  /* 3D /w */ { BxImmediate_Iw, BX_IA_CMP_AXIw },
  /* 3E /w */ { 0, BX_IA_ERROR }, // DS:
  /* 3F /w */ { 0, BX_IA_ERROR },
  /* 40 /w */ { 0, BX_IA_ERROR }, // REX:
  /* 41 /w */ { 0, BX_IA_ERROR }, // REX:
  /* 42 /w */ { 0, BX_IA_ERROR }, // REX:
  /* 43 /w */ { 0, BX_IA_ERROR }, // REX:
  /* 44 /w */ { 0, BX_IA_ERROR }, // REX:
  /* 45 /w */ { 0, BX_IA_ERROR }, // REX:
  /* 46 /w */ { 0, BX_IA_ERROR }, // REX:
  /* 47 /w */ { 0, BX_IA_ERROR }, // REX:
  /* 48 /w */ { 0, BX_IA_ERROR }, // REX:
  /* 49 /w */ { 0, BX_IA_ERROR }, // REX:
  /* 4A /w */ { 0, BX_IA_ERROR }, // REX:
  /* 4B /w */ { 0, BX_IA_ERROR }, // REX:
  /* 4C /w */ { 0, BX_IA_ERROR }, // REX:
  /* 4D /w */ { 0, BX_IA_ERROR }, // REX:
  /* 4E /w */ { 0, BX_IA_ERROR }, // REX:
  /* 4F /w */ { 0, BX_IA_ERROR }, // REX:
  /* 50 /w */ { 0, BX_IA_PUSH_RX },
  /* 51 /w */ { 0, BX_IA_PUSH_RX },
  /* 52 /w */ { 0, BX_IA_PUSH_RX },
  /* 53 /w */ { 0, BX_IA_PUSH_RX },
  /* 54 /w */ { 0, BX_IA_PUSH_RX },
  /* 55 /w */ { 0, BX_IA_PUSH_RX },
  /* 56 /w */ { 0, BX_IA_PUSH_RX },
  /* 57 /w */ { 0, BX_IA_PUSH_RX },
  /* 58 /w */ { 0, BX_IA_POP_RX },
  /* 59 /w */ { 0, BX_IA_POP_RX },
  /* 5A /w */ { 0, BX_IA_POP_RX },
  /* 5B /w */ { 0, BX_IA_POP_RX },
  /* 5C /w */ { 0, BX_IA_POP_RX },
  /* 5D /w */ { 0, BX_IA_POP_RX },
  /* 5E /w */ { 0, BX_IA_POP_RX },
  /* 5F /w */ { 0, BX_IA_POP_RX },
  /* 60 /w */ { 0, BX_IA_ERROR },
  /* 61 /w */ { 0, BX_IA_ERROR },
  /* 62 /w */ { 0, BX_IA_ERROR },
  /* 63 /w */ { 0, BX_IA_MOV_GwEw }, // MOVSX_GwEw
  /* 64 /w */ { 0, BX_IA_ERROR }, // FS:
  /* 65 /w */ { 0, BX_IA_ERROR }, // GS:
  /* 66 /w */ { 0, BX_IA_ERROR }, // OS:
  /* 67 /w */ { 0, BX_IA_ERROR }, // AS:
  /* 68 /w */ { BxImmediate_Iw, BX_IA_PUSH_Iw },
  /* 69 /w */ { BxImmediate_Iw, BX_IA_IMUL_GwEwIw },
  /* 6A /w */ { BxImmediate_Ib_SE, BX_IA_PUSH_Iw },
  /* 6B /w */ { BxImmediate_Ib_SE, BX_IA_IMUL_GwEwIw },
  /* 6C /w */ { 0, BX_IA_REP_INSB_YbDX },
  /* 6D /w */ { 0, BX_IA_REP_INSW_YwDX },
  /* 6E /w */ { 0, BX_IA_REP_OUTSB_DXXb },
  /* 6F /w */ { 0, BX_IA_REP_OUTSW_DXXw },
  /* 70 /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JO_Jq },
  /* 71 /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNO_Jq },
  /* 72 /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JB_Jq },
  /* 73 /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNB_Jq },
  /* 74 /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JZ_Jq },
  /* 75 /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNZ_Jq },
  /* 76 /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JBE_Jq },
  /* 77 /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNBE_Jq },
  /* 78 /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JS_Jq },
  /* 79 /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNS_Jq },
  /* 7A /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JP_Jq },
  /* 7B /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNP_Jq },
  /* 7C /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JL_Jq },
  /* 7D /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNL_Jq },
  /* 7E /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JLE_Jq },
  /* 7F /w */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNLE_Jq },
  /* 80 /w */ { BxGroup1 | BxImmediate_Ib, BX_IA_ERROR, BxOpcodeInfoG1EbIb },
  /* 81 /w */ { BxGroup1 | BxImmediate_Iw, BX_IA_ERROR, BxOpcodeInfoG1Ew },
  /* 82 /w */ { 0, BX_IA_ERROR },
  /* 83 /w */ { BxGroup1 | BxImmediate_Ib_SE, BX_IA_ERROR, BxOpcodeInfoG1Ew },
  /* 84 /w */ { 0, BX_IA_TEST_EbGb },
  /* 85 /w */ { 0, BX_IA_TEST_EwGw },
  /* 86 /w */ { BxLockable, BX_IA_XCHG_EbGb },
  /* 87 /w */ { BxLockable, BX_IA_XCHG_EwGw },
  /* 88 /w */ { BxArithDstRM, BX_IA_MOV_EbGb },
  /* 89 /w */ { BxArithDstRM, BX_IA_MOV_EwGw },
  /* 8A /w */ { 0, BX_IA_MOV_GbEb },
  /* 8B /w */ { 0, BX_IA_MOV_GwEw },
  /* 8C /w */ { 0, BX_IA_MOV_EwSw },
  /* 8D /w */ { 0, BX_IA_LEA_GwM },
  /* 8E /w */ { 0, BX_IA_MOV_SwEw },
  /* 8F /w */ { BxGroup1A, BX_IA_ERROR, BxOpcodeInfoG1AEw },
  /* 90 /w */ { 0, BX_IA_XCHG_RXAX }, // handles XCHG R8w, AX
  /* 91 /w */ { 0, BX_IA_XCHG_RXAX },
  /* 92 /w */ { 0, BX_IA_XCHG_RXAX },
  /* 93 /w */ { 0, BX_IA_XCHG_RXAX },
  /* 94 /w */ { 0, BX_IA_XCHG_RXAX },
  /* 95 /w */ { 0, BX_IA_XCHG_RXAX },
  /* 96 /w */ { 0, BX_IA_XCHG_RXAX },
  /* 97 /w */ { 0, BX_IA_XCHG_RXAX },
  /* 98 /w */ { 0, BX_IA_CBW },
  /* 99 /w */ { 0, BX_IA_CWD },
  /* 9A /w */ { 0, BX_IA_ERROR },
  /* 9B /w */ { 0, BX_IA_FWAIT },
  /* 9C /w */ { 0, BX_IA_PUSHF_Fw },
  /* 9D /w */ { 0, BX_IA_POPF_Fw },
  /* 9E /w */ { 0, BX_IA_SAHF },
  /* 9F /w */ { 0, BX_IA_LAHF },
  /* A0 /w */ { BxImmediate_O, BX_IA_MOV_ALOq },
  /* A1 /w */ { BxImmediate_O, BX_IA_MOV_AXOq },
  /* A2 /w */ { BxImmediate_O, BX_IA_MOV_OqAL },
  /* A3 /w */ { BxImmediate_O, BX_IA_MOV_OqAX },
  /* A4 /w */ { 0, BX_IA_REP_MOVSB_XbYb },
  /* A5 /w */ { 0, BX_IA_REP_MOVSW_XwYw },
  /* A6 /w */ { 0, BX_IA_REP_CMPSB_XbYb },
  /* A7 /w */ { 0, BX_IA_REP_CMPSW_XwYw },
  /* A8 /w */ { BxImmediate_Ib, BX_IA_TEST_ALIb },
  /* A9 /w */ { BxImmediate_Iw, BX_IA_TEST_AXIw },
  /* AA /w */ { 0, BX_IA_REP_STOSB_YbAL },
  /* AB /w */ { 0, BX_IA_REP_STOSW_YwAX },
  /* AC /w */ { 0, BX_IA_REP_LODSB_ALXb },
  /* AD /w */ { 0, BX_IA_REP_LODSW_AXXw },
  /* AE /w */ { 0, BX_IA_REP_SCASB_ALXb },
  /* AF /w */ { 0, BX_IA_REP_SCASW_AXXw },
  /* B0 /w */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B1 /w */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B2 /w */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B3 /w */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B4 /w */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B5 /w */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B6 /w */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B7 /w */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B8 /w */ { BxImmediate_Iw, BX_IA_MOV_RXIw },
  /* B9 /w */ { BxImmediate_Iw, BX_IA_MOV_RXIw },
  /* BA /w */ { BxImmediate_Iw, BX_IA_MOV_RXIw },
  /* BB /w */ { BxImmediate_Iw, BX_IA_MOV_RXIw },
  /* BC /w */ { BxImmediate_Iw, BX_IA_MOV_RXIw },
  /* BD /w */ { BxImmediate_Iw, BX_IA_MOV_RXIw },
  /* BE /w */ { BxImmediate_Iw, BX_IA_MOV_RXIw },
  /* BF /w */ { BxImmediate_Iw, BX_IA_MOV_RXIw },
  /* C0 /w */ { BxGroup2 | BxImmediate_Ib, BX_IA_ERROR, BxOpcodeInfoG2Eb },
  /* C1 /w */ { BxGroup2 | BxImmediate_Ib, BX_IA_ERROR, BxOpcodeInfoG2Ew },
  /* C2 /w */ { BxImmediate_Iw | BxTraceEnd, BX_IA_RETnear64_Iw },
  /* C3 /w */ { BxTraceEnd,                  BX_IA_RETnear64 },
  /* C4 /w */ { 0, BX_IA_ERROR },
  /* C5 /w */ { 0, BX_IA_ERROR },
  /* C6 /w */ { BxGroup11, BX_IA_ERROR, BxOpcodeInfoG11Eb },
  /* C7 /w */ { BxGroup11, BX_IA_ERROR, BxOpcodeInfoG11Ew },
  /* C8 /w */ { BxImmediate_Iw | BxImmediate_Ib2, BX_IA_ENTER64_IwIb },
  /* C9 /w */ { 0, BX_IA_LEAVE64 },
  /* CA /w */ { BxImmediate_Iw | BxTraceEnd, BX_IA_RETfar16_Iw },
  /* CB /w */ { BxTraceEnd,                  BX_IA_RETfar16 },
  /* CC /w */ { BxTraceEnd, BX_IA_INT3 },
  /* CD /w */ { BxImmediate_Ib | BxTraceEnd, BX_IA_INT_Ib },
  /* CE /w */ { 0, BX_IA_ERROR },
  /* CF /w */ { BxTraceEnd, BX_IA_IRET64 },
  /* D0 /w */ { BxGroup2 | BxImmediate_I1, BX_IA_ERROR, BxOpcodeInfoG2Eb },
  /* D1 /w */ { BxGroup2 | BxImmediate_I1, BX_IA_ERROR, BxOpcodeInfoG2Ew },
  /* D2 /w */ { BxGroup2, BX_IA_ERROR, BxOpcodeInfoG2Eb },
  /* D3 /w */ { BxGroup2, BX_IA_ERROR, BxOpcodeInfoG2Ew },
  /* D4 /w */ { 0, BX_IA_ERROR },
  /* D5 /w */ { 0, BX_IA_ERROR },
  /* D6 /w */ { 0, BX_IA_ERROR },
  /* D7 /w */ { 0, BX_IA_XLAT },
  /* D8 /w */ { BxGroupFP, BX_IA_ERROR, BxOpcodeInfo_FPGroupD8 },
  /* D9 /w */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointD9 },
  /* DA /w */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDA },
  /* DB /w */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDB },
  /* DC /w */ { BxGroupFP, BX_IA_ERROR, BxOpcodeInfo_FPGroupDC },
  /* DD /w */ { BxGroupFP, BX_IA_ERROR, BxOpcodeInfo_FPGroupDD },
  /* DE /w */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDE },
  /* DF /w */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDF },
  /* E0 /w */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_LOOPNE64_Jb },
  /* E1 /w */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_LOOPE64_Jb },
  /* E2 /w */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_LOOP64_Jb },
  /* E3 /w */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_JRCXZ_Jb },
  /* E4 /w */ { BxImmediate_Ib, BX_IA_IN_ALIb },
  /* E5 /w */ { BxImmediate_Ib, BX_IA_IN_AXIb },
  /* E6 /w */ { BxImmediate_Ib, BX_IA_OUT_IbAL },
  /* E7 /w */ { BxImmediate_Ib, BX_IA_OUT_IbAX },
  /* E8 /w */ { BxImmediate_BrOff32 | BxTraceEnd, BX_IA_CALL_Jq },
  /* E9 /w */ { BxImmediate_BrOff32 | BxTraceEnd, BX_IA_JMP_Jq },
  /* EA /w */ { 0, BX_IA_ERROR },
  /* EB /w */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_JMP_Jq },
  /* EC /w */ { 0, BX_IA_IN_ALDX },
  /* ED /w */ { 0, BX_IA_IN_AXDX },
  /* EE /w */ { 0, BX_IA_OUT_DXAL },
  /* EF /w */ { 0, BX_IA_OUT_DXAX },
  /* F0 /w */ { 0, BX_IA_ERROR }, // LOCK
  /* F1 /w */ { BxTraceEnd, BX_IA_INT1 },
  /* F2 /w */ { 0, BX_IA_ERROR }, // REPNE/REPNZ
  /* F3 /w */ { 0, BX_IA_ERROR }, // REP, REPE/REPZ
  /* F4 /w */ { BxTraceEnd, BX_IA_HLT },
  /* F5 /w */ { 0, BX_IA_CMC },
  /* F6 /w */ { BxGroup3, BX_IA_ERROR, BxOpcodeInfoG3Eb },
  /* F7 /w */ { BxGroup3, BX_IA_ERROR, BxOpcodeInfoG3Ew },
  /* F8 /w */ { 0, BX_IA_CLC },
  /* F9 /w */ { 0, BX_IA_STC },
  /* FA /w */ { 0, BX_IA_CLI },
  /* FB /w */ { 0, BX_IA_STI },
  /* FC /w */ { 0, BX_IA_CLD },
  /* FD /w */ { 0, BX_IA_STD },
  /* FE /w */ { BxGroup4, BX_IA_ERROR, BxOpcodeInfoG4 },
  /* FF /w */ { BxGroup5, BX_IA_ERROR, BxOpcodeInfo64G5w },

  /* 0F 00 /w */ { BxGroup6, BX_IA_ERROR, BxOpcodeInfoG6 },
  /* 0F 01 /w */ { BxGroup7, BX_IA_ERROR, BxOpcodeInfoG7q },
  /* 0F 02 /w */ { 0, BX_IA_LAR_GvEw },
  /* 0F 03 /w */ { 0, BX_IA_LSL_GvEw },
  /* 0F 04 /w */ { 0, BX_IA_ERROR },
  /* 0F 05 /w */ { BxTraceEnd, BX_IA_SYSCALL },
  /* 0F 06 /w */ { 0, BX_IA_CLTS },
  /* 0F 07 /w */ { BxTraceEnd, BX_IA_SYSRET },
  /* 0F 08 /w */ { BxTraceEnd, BX_IA_INVD },
  /* 0F 09 /w */ { BxTraceEnd, BX_IA_WBINVD },
  /* 0F 0A /w */ { 0, BX_IA_ERROR },
  /* 0F 0B /w */ { BxTraceEnd, BX_IA_UD2A },
  /* 0F 0C /w */ { 0, BX_IA_ERROR },
  /* 0F 0D /w */ { 0, BX_IA_PREFETCHW },          // 3DNow! PREFETCHW on AMD, NOP on Intel
  /* 0F 0E /w */ { 0, BX_IA_FEMMS },              // 3DNow! FEMMS
  /* 0F 0F /w */ { BxImmediate_Ib, BX_IA_ERROR }, // 3DNow! Opcode Table
  /* 0F 10 /w */ { BxPrefixSSE,                BX_IA_MOVUPS_VpsWps, BxOpcodeGroupSSE_0f10 },
  /* 0F 11 /w */ { BxPrefixSSE | BxArithDstRM, BX_IA_MOVUPS_WpsVps, BxOpcodeGroupSSE_0f11 },
  /* 0F 12 /w */ { BxPrefixSSE, BX_IA_MOVLPS_VpsMq, BxOpcodeGroupSSE_0f12 },
  /* 0F 13 /w */ { BxPrefixSSE, BX_IA_MOVLPS_MqVps, BxOpcodeGroupSSE_0f13M },
  /* 0F 14 /w */ { BxPrefixSSE, BX_IA_UNPCKLPS_VpsWdq, BxOpcodeGroupSSE_0f14 },
  /* 0F 15 /w */ { BxPrefixSSE, BX_IA_UNPCKHPS_VpsWdq, BxOpcodeGroupSSE_0f15 },
  /* 0F 16 /w */ { BxPrefixSSE, BX_IA_MOVHPS_VpsMq, BxOpcodeGroupSSE_0f16 },
  /* 0F 17 /w */ { BxPrefixSSE, BX_IA_MOVHPS_MqVps, BxOpcodeGroupSSE_0f17M },
  /* 0F 18 /w */ { 0, BX_IA_PREFETCH }, // opcode group G16, PREFETCH hints
  /* 0F 19 /w */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1A /w */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1B /w */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1C /w */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1D /w */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1E /w */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1F /w */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 20 /w */ { BxGroupN, BX_IA_ERROR, BxOpcodeInfoMOV_RqCq },
  /* 0F 21 /w */ { 0, BX_IA_MOV_RqDq },
  /* 0F 22 /w */ { BxGroupN, BX_IA_ERROR, BxOpcodeInfoMOV_CqRq },
  /* 0F 23 /w */ { BxTraceEnd, BX_IA_MOV_DqRq },
  /* 0F 24 /w */ { 0, BX_IA_ERROR },
  /* 0F 25 /w */ { 0, BX_IA_ERROR },
  /* 0F 26 /w */ { 0, BX_IA_ERROR },
  /* 0F 27 /w */ { 0, BX_IA_ERROR },
  /* 0F 28 /w */ { BxPrefixSSE,                BX_IA_MOVAPS_VpsWps, BxOpcodeGroupSSE_0f28 },
  /* 0F 29 /w */ { BxPrefixSSE | BxArithDstRM, BX_IA_MOVAPS_WpsVps, BxOpcodeGroupSSE_0f29 },
  /* 0F 2A /w */ { BxPrefixSSE, BX_IA_CVTPI2PS_VpsQq, BxOpcodeGroupSSE_0f2a },
  /* 0F 2B /w */ { BxPrefixSSE, BX_IA_MOVNTPS_MpsVps, BxOpcodeGroupSSE_0f2bM },
  /* 0F 2C /w */ { BxPrefixSSE, BX_IA_CVTTPS2PI_PqWps, BxOpcodeGroupSSE_0f2c },
  /* 0F 2D /w */ { BxPrefixSSE, BX_IA_CVTPS2PI_PqWps, BxOpcodeGroupSSE_0f2d },
  /* 0F 2E /w */ { BxPrefixSSE, BX_IA_UCOMISS_VssWss, BxOpcodeGroupSSE_0f2e },
  /* 0F 2F /w */ { BxPrefixSSE, BX_IA_COMISS_VpsWps, BxOpcodeGroupSSE_0f2f },
  /* 0F 30 /w */ { 0, BX_IA_WRMSR },
  /* 0F 31 /w */ { 0, BX_IA_RDTSC },
  /* 0F 32 /w */ { 0, BX_IA_RDMSR },
  /* 0F 33 /w */ { 0, BX_IA_RDPMC },
  /* 0F 34 /w */ { BxTraceEnd, BX_IA_SYSENTER },
  /* 0F 35 /w */ { BxTraceEnd, BX_IA_SYSEXIT },
  /* 0F 36 /w */ { 0, BX_IA_ERROR },
  /* 0F 37 /w */ { 0, BX_IA_ERROR },
  /* 0F 38 /w */ { Bx3ByteOp, BX_IA_ERROR, BxOpcode3ByteTable0f38 }, // 3-byte escape
  /* 0F 39 /w */ { 0, BX_IA_ERROR },
  /* 0F 3A /w */ { Bx3ByteOp | BxImmediate_Ib, BX_IA_ERROR, BxOpcode3ByteTable0f3a }, // 3-byte escape
  /* 0F 3B /w */ { 0, BX_IA_ERROR },
  /* 0F 3C /w */ { 0, BX_IA_ERROR },
  /* 0F 3D /w */ { 0, BX_IA_ERROR },
  /* 0F 3E /w */ { 0, BX_IA_ERROR },
  /* 0F 3F /w */ { 0, BX_IA_ERROR },
  /* 0F 40 /w */ { 0, BX_IA_CMOVO_GwEw },
  /* 0F 41 /w */ { 0, BX_IA_CMOVNO_GwEw },
  /* 0F 42 /w */ { 0, BX_IA_CMOVB_GwEw },
  /* 0F 43 /w */ { 0, BX_IA_CMOVNB_GwEw },
  /* 0F 44 /w */ { 0, BX_IA_CMOVZ_GwEw },
  /* 0F 45 /w */ { 0, BX_IA_CMOVNZ_GwEw },
  /* 0F 46 /w */ { 0, BX_IA_CMOVBE_GwEw },
  /* 0F 47 /w */ { 0, BX_IA_CMOVNBE_GwEw },
  /* 0F 48 /w */ { 0, BX_IA_CMOVS_GwEw },
  /* 0F 49 /w */ { 0, BX_IA_CMOVNS_GwEw },
  /* 0F 4A /w */ { 0, BX_IA_CMOVP_GwEw },
  /* 0F 4B /w */ { 0, BX_IA_CMOVNP_GwEw },
  /* 0F 4C /w */ { 0, BX_IA_CMOVL_GwEw },
  /* 0F 4D /w */ { 0, BX_IA_CMOVNL_GwEw },
  /* 0F 4E /w */ { 0, BX_IA_CMOVLE_GwEw },
  /* 0F 4F /w */ { 0, BX_IA_CMOVNLE_GwEw },
  /* 0F 50 /w */ { BxPrefixSSE, BX_IA_MOVMSKPS_GdVRps, BxOpcodeGroupSSE_0f50R },
  /* 0F 51 /w */ { BxPrefixSSE, BX_IA_SQRTPS_VpsWps, BxOpcodeGroupSSE_0f51 },
  /* 0F 52 /w */ { BxPrefixSSE, BX_IA_RSQRTPS_VpsWps, BxOpcodeGroupSSE_0f52 },
  /* 0F 53 /w */ { BxPrefixSSE, BX_IA_RCPPS_VpsWps, BxOpcodeGroupSSE_0f53 },
  /* 0F 54 /w */ { BxPrefixSSE, BX_IA_ANDPS_VpsWps, BxOpcodeGroupSSE_0f54 },
  /* 0F 55 /w */ { BxPrefixSSE, BX_IA_ANDNPS_VpsWps, BxOpcodeGroupSSE_0f55 },
  /* 0F 56 /w */ { BxPrefixSSE, BX_IA_ORPS_VpsWps, BxOpcodeGroupSSE_0f56 },
  /* 0F 57 /w */ { BxPrefixSSE, BX_IA_XORPS_VpsWps, BxOpcodeGroupSSE_0f57 },
  /* 0F 58 /w */ { BxPrefixSSE, BX_IA_ADDPS_VpsWps, BxOpcodeGroupSSE_0f58 },
  /* 0F 59 /w */ { BxPrefixSSE, BX_IA_MULPS_VpsWps, BxOpcodeGroupSSE_0f59 },
  /* 0F 5A /w */ { BxPrefixSSE, BX_IA_CVTPS2PD_VpsWps, BxOpcodeGroupSSE_0f5a },
  /* 0F 5B /w */ { BxPrefixSSE, BX_IA_CVTDQ2PS_VpsWdq, BxOpcodeGroupSSE_0f5b },
  /* 0F 5C /w */ { BxPrefixSSE, BX_IA_SUBPS_VpsWps, BxOpcodeGroupSSE_0f5c },
  /* 0F 5D /w */ { BxPrefixSSE, BX_IA_MINPS_VpsWps, BxOpcodeGroupSSE_0f5d },
  /* 0F 5E /w */ { BxPrefixSSE, BX_IA_DIVPS_VpsWps, BxOpcodeGroupSSE_0f5e },
  /* 0F 5F /w */ { BxPrefixSSE, BX_IA_MAXPS_VpsWps, BxOpcodeGroupSSE_0f5f },
  /* 0F 60 /w */ { BxPrefixSSE, BX_IA_PUNPCKLBW_PqQd, BxOpcodeGroupSSE_0f60 },
  /* 0F 61 /w */ { BxPrefixSSE, BX_IA_PUNPCKLWD_PqQd, BxOpcodeGroupSSE_0f61 },
  /* 0F 62 /w */ { BxPrefixSSE, BX_IA_PUNPCKLDQ_PqQd, BxOpcodeGroupSSE_0f62 },
  /* 0F 63 /w */ { BxPrefixSSE, BX_IA_PACKSSWB_PqQq, BxOpcodeGroupSSE_0f63 },
  /* 0F 64 /w */ { BxPrefixSSE, BX_IA_PCMPGTB_PqQq, BxOpcodeGroupSSE_0f64 },
  /* 0F 65 /w */ { BxPrefixSSE, BX_IA_PCMPGTW_PqQq, BxOpcodeGroupSSE_0f65 },
  /* 0F 66 /w */ { BxPrefixSSE, BX_IA_PCMPGTD_PqQq, BxOpcodeGroupSSE_0f66 },
  /* 0F 67 /w */ { BxPrefixSSE, BX_IA_PACKUSWB_PqQq, BxOpcodeGroupSSE_0f67 },
  /* 0F 68 /w */ { BxPrefixSSE, BX_IA_PUNPCKHBW_PqQq, BxOpcodeGroupSSE_0f68 },
  /* 0F 69 /w */ { BxPrefixSSE, BX_IA_PUNPCKHWD_PqQq, BxOpcodeGroupSSE_0f69 },
  /* 0F 6A /w */ { BxPrefixSSE, BX_IA_PUNPCKHDQ_PqQq, BxOpcodeGroupSSE_0f6a },
  /* 0F 6B /w */ { BxPrefixSSE, BX_IA_PACKSSDW_PqQq, BxOpcodeGroupSSE_0f6b },
  /* 0F 6C /w */ { BxPrefixSSE66, BX_IA_PUNPCKLQDQ_VdqWdq },
  /* 0F 6D /w */ { BxPrefixSSE66, BX_IA_PUNPCKHQDQ_VdqWdq },
  /* 0F 6E /w */ { BxPrefixSSE, BX_IA_MOVD_PqEd, BxOpcodeGroupSSE_0f6e },
  /* 0F 6F /w */ { BxPrefixSSE, BX_IA_MOVQ_PqQq, BxOpcodeGroupSSE_0f6f },
  /* 0F 70 /w */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_PSHUFW_PqQqIb, BxOpcodeGroupSSE_0f70 },
  /* 0F 71 /w */ { BxGroup12, BX_IA_ERROR, BxOpcodeInfoG12R },
  /* 0F 72 /w */ { BxGroup13, BX_IA_ERROR, BxOpcodeInfoG13R },
  /* 0F 73 /w */ { BxGroup14, BX_IA_ERROR, BxOpcodeInfoG14R },
  /* 0F 74 /w */ { BxPrefixSSE, BX_IA_PCMPEQB_PqQq, BxOpcodeGroupSSE_0f74 },
  /* 0F 75 /w */ { BxPrefixSSE, BX_IA_PCMPEQW_PqQq, BxOpcodeGroupSSE_0f75 },
  /* 0F 76 /w */ { BxPrefixSSE, BX_IA_PCMPEQD_PqQq, BxOpcodeGroupSSE_0f76 },
  /* 0F 77 /w */ { BxPrefixSSE, BX_IA_EMMS, BxOpcodeGroupSSE_ERR },
  /* 0F 78 /w */ { BxPrefixSSE, BX_IA_VMREAD_EqGq, BxOpcodeGroupSSE_ERR },
  /* 0F 79 /w */ { BxPrefixSSE, BX_IA_VMWRITE_GqEq, BxOpcodeGroupSSE_ERR },
  /* 0F 7A /w */ { 0, BX_IA_ERROR },
  /* 0F 7B /w */ { 0, BX_IA_ERROR },
  /* 0F 7C /w */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0f7c },
  /* 0F 7D /w */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0f7d },
  /* 0F 7E /w */ { BxPrefixSSE, BX_IA_MOVD_EdPd, BxOpcodeGroupSSE_0f7e },
  /* 0F 7F /w */ { BxPrefixSSE, BX_IA_MOVQ_QqPq, BxOpcodeGroupSSE_0f7f },
  /* 0F 80 /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JO_Jq },
  /* 0F 81 /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNO_Jq },
  /* 0F 82 /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JB_Jq },
  /* 0F 83 /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNB_Jq },
  /* 0F 84 /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JZ_Jq },
  /* 0F 85 /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNZ_Jq },
  /* 0F 86 /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JBE_Jq },
  /* 0F 87 /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNBE_Jq },
  /* 0F 88 /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JS_Jq },
  /* 0F 89 /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNS_Jq },
  /* 0F 8A /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JP_Jq },
  /* 0F 8B /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNP_Jq },
  /* 0F 8C /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JL_Jq },
  /* 0F 8D /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNL_Jq },
  /* 0F 8E /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JLE_Jq },
  /* 0F 8F /w */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNLE_Jq },
  /* 0F 90 /w */ { 0, BX_IA_SETO_Eb },
  /* 0F 91 /w */ { 0, BX_IA_SETNO_Eb },
  /* 0F 92 /w */ { 0, BX_IA_SETB_Eb },
  /* 0F 93 /w */ { 0, BX_IA_SETNB_Eb },
  /* 0F 94 /w */ { 0, BX_IA_SETZ_Eb },
  /* 0F 95 /w */ { 0, BX_IA_SETNZ_Eb },
  /* 0F 96 /w */ { 0, BX_IA_SETBE_Eb },
  /* 0F 97 /w */ { 0, BX_IA_SETNBE_Eb },
  /* 0F 98 /w */ { 0, BX_IA_SETS_Eb },
  /* 0F 99 /w */ { 0, BX_IA_SETNS_Eb },
  /* 0F 9A /w */ { 0, BX_IA_SETP_Eb },
  /* 0F 9B /w */ { 0, BX_IA_SETNP_Eb },
  /* 0F 9C /w */ { 0, BX_IA_SETL_Eb },
  /* 0F 9D /w */ { 0, BX_IA_SETNL_Eb },
  /* 0F 9E /w */ { 0, BX_IA_SETLE_Eb },
  /* 0F 9F /w */ { 0, BX_IA_SETNLE_Eb },
  /* 0F A0 /w */ { 0, BX_IA_PUSH16_FS },
  /* 0F A1 /w */ { 0, BX_IA_POP16_FS },
  /* 0F A2 /w */ { 0, BX_IA_CPUID },
  /* 0F A3 /w */ { 0, BX_IA_BT_EwGw },
  /* 0F A4 /w */ { BxImmediate_Ib, BX_IA_SHLD_EwGw },
  /* 0F A5 /w */ { 0,              BX_IA_SHLD_EwGw },
  /* 0F A6 /w */ { 0, BX_IA_ERROR },
  /* 0F A7 /w */ { 0, BX_IA_ERROR },
  /* 0F A8 /w */ { 0, BX_IA_PUSH16_GS },
  /* 0F A9 /w */ { 0, BX_IA_POP16_GS },
  /* 0F AA /w */ { BxTraceEnd, BX_IA_RSM },
  /* 0F AB /w */ { BxLockable, BX_IA_BTS_EwGw },
  /* 0F AC /w */ { BxImmediate_Ib, BX_IA_SHRD_EwGw },
  /* 0F AD /w */ { 0,              BX_IA_SHRD_EwGw },
  /* 0F AE /w */ { BxGroup15, BX_IA_ERROR, BxOpcodeInfoG15q },
  /* 0F AF /w */ { 0, BX_IA_IMUL_GwEw },
  /* 0F B0 /w */ { BxLockable, BX_IA_CMPXCHG_EbGb },
  /* 0F B1 /w */ { BxLockable, BX_IA_CMPXCHG_EwGw },
  /* 0F B2 /w */ { 0, BX_IA_LSS_GwMp },
  /* 0F B3 /w */ { BxLockable, BX_IA_BTR_EwGw },
  /* 0F B4 /w */ { 0, BX_IA_LFS_GwMp },
  /* 0F B5 /w */ { 0, BX_IA_LGS_GwMp },
  /* 0F B6 /w */ { 0, BX_IA_MOVZX_GwEb },
  /* 0F B7 /w */ { 0, BX_IA_MOV_GwEw }, // MOVZX_GwEw
  /* 0F B8 /w */ { BxPrefixSSEF3, BX_IA_POPCNT_GwEw },
  /* 0F B9 /w */ { BxTraceEnd, BX_IA_UD2B },
  /* 0F BA /w */ { BxGroup8, BX_IA_ERROR, BxOpcodeInfoG8EwIb },
  /* 0F BB /w */ { BxLockable, BX_IA_BTC_EwGw },
  /* 0F BC /w */ { 0, BX_IA_BSF_GwEw },
  /* 0F BD /w */ { 0, BX_IA_BSR_GwEw },
  /* 0F BE /w */ { 0, BX_IA_MOVSX_GwEb },
  /* 0F BF /w */ { 0, BX_IA_MOV_GwEw }, // MOVSX_GwEw
  /* 0F C0 /w */ { BxLockable, BX_IA_XADD_EbGb },
  /* 0F C1 /w */ { BxLockable, BX_IA_XADD_EwGw },
  /* 0F C2 /w */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_CMPPS_VpsWpsIb, BxOpcodeGroupSSE_0fc2 },
  /* 0F C3 /w */ { BxPrefixSSE, BX_IA_MOVNTI_MdGd, BxOpcodeGroupSSE_ERR },
  /* 0F C4 /w */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_PINSRW_PqEwIb, BxOpcodeGroupSSE_0fc4 },
  /* 0F C5 /w */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_PEXTRW_GdPqIb, BxOpcodeGroupSSE_0fc5R },
  /* 0F C6 /w */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_SHUFPS_VpsWpsIb, BxOpcodeGroupSSE_0fc6 },
  /* 0F C7 /w */ { BxGroup9, BX_IA_ERROR, BxOpcodeInfoG9M },
  /* 0F C8 /w */ { 0, BX_IA_BSWAP_RX },
  /* 0F C9 /w */ { 0, BX_IA_BSWAP_RX },
  /* 0F CA /w */ { 0, BX_IA_BSWAP_RX },
  /* 0F CB /w */ { 0, BX_IA_BSWAP_RX },
  /* 0F CC /w */ { 0, BX_IA_BSWAP_RX },
  /* 0F CD /w */ { 0, BX_IA_BSWAP_RX },
  /* 0F CE /w */ { 0, BX_IA_BSWAP_RX },
  /* 0F CF /w */ { 0, BX_IA_BSWAP_RX },
  /* 0F D0 /w */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0fd0 },
  /* 0F D1 /w */ { BxPrefixSSE, BX_IA_PSRLW_PqQq, BxOpcodeGroupSSE_0fd1 },
  /* 0F D2 /w */ { BxPrefixSSE, BX_IA_PSRLD_PqQq, BxOpcodeGroupSSE_0fd2 },
  /* 0F D3 /w */ { BxPrefixSSE, BX_IA_PSRLQ_PqQq, BxOpcodeGroupSSE_0fd3 },
  /* 0F D4 /w */ { BxPrefixSSE, BX_IA_PADDQ_PqQq, BxOpcodeGroupSSE_0fd4 },
  /* 0F D5 /w */ { BxPrefixSSE, BX_IA_PMULLW_PqQq, BxOpcodeGroupSSE_0fd5 },
  /* 0F D6 /w */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0fd6 },
  /* 0F D7 /w */ { BxPrefixSSE, BX_IA_PMOVMSKB_GdPRq, BxOpcodeGroupSSE_0fd7R },
  /* 0F D8 /w */ { BxPrefixSSE, BX_IA_PSUBUSB_PqQq, BxOpcodeGroupSSE_0fd8 },
  /* 0F D9 /w */ { BxPrefixSSE, BX_IA_PSUBUSW_PqQq, BxOpcodeGroupSSE_0fd9 },
  /* 0F DA /w */ { BxPrefixSSE, BX_IA_PMINUB_PqQq, BxOpcodeGroupSSE_0fda },
  /* 0F DB /w */ { BxPrefixSSE, BX_IA_PAND_PqQq, BxOpcodeGroupSSE_0fdb },
  /* 0F DC /w */ { BxPrefixSSE, BX_IA_PADDUSB_PqQq, BxOpcodeGroupSSE_0fdc },
  /* 0F DD /w */ { BxPrefixSSE, BX_IA_PADDUSW_PqQq, BxOpcodeGroupSSE_0fdd },
  /* 0F DE /w */ { BxPrefixSSE, BX_IA_PMAXUB_PqQq, BxOpcodeGroupSSE_0fde },
  /* 0F DF /w */ { BxPrefixSSE, BX_IA_PANDN_PqQq, BxOpcodeGroupSSE_0fdf },
  /* 0F E0 /w */ { BxPrefixSSE, BX_IA_PAVGB_PqQq, BxOpcodeGroupSSE_0fe0 },
  /* 0F E1 /w */ { BxPrefixSSE, BX_IA_PSRAW_PqQq, BxOpcodeGroupSSE_0fe1 },
  /* 0F E2 /w */ { BxPrefixSSE, BX_IA_PSRAD_PqQq, BxOpcodeGroupSSE_0fe2 },
  /* 0F E3 /w */ { BxPrefixSSE, BX_IA_PAVGW_PqQq, BxOpcodeGroupSSE_0fe3 },
  /* 0F E4 /w */ { BxPrefixSSE, BX_IA_PMULHUW_PqQq, BxOpcodeGroupSSE_0fe4 },
  /* 0F E5 /w */ { BxPrefixSSE, BX_IA_PMULHW_PqQq, BxOpcodeGroupSSE_0fe5 },
  /* 0F E6 /w */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0fe6 },
  /* 0F E7 /w */ { BxPrefixSSE, BX_IA_MOVNTQ_MqPq, BxOpcodeGroupSSE_0fe7M },
  /* 0F E8 /w */ { BxPrefixSSE, BX_IA_PSUBSB_PqQq, BxOpcodeGroupSSE_0fe8 },
  /* 0F E9 /w */ { BxPrefixSSE, BX_IA_PSUBSW_PqQq, BxOpcodeGroupSSE_0fe9 },
  /* 0F EA /w */ { BxPrefixSSE, BX_IA_PMINSW_PqQq, BxOpcodeGroupSSE_0fea },
  /* 0F EB /w */ { BxPrefixSSE, BX_IA_POR_PqQq, BxOpcodeGroupSSE_0feb },
  /* 0F EC /w */ { BxPrefixSSE, BX_IA_PADDSB_PqQq, BxOpcodeGroupSSE_0fec },
  /* 0F ED /w */ { BxPrefixSSE, BX_IA_PADDSW_PqQq, BxOpcodeGroupSSE_0fed },
  /* 0F EE /w */ { BxPrefixSSE, BX_IA_PMAXSW_PqQq, BxOpcodeGroupSSE_0fee },
  /* 0F EF /w */ { BxPrefixSSE, BX_IA_PXOR_PqQq, BxOpcodeGroupSSE_0fef },
  /* 0F F0 /w */ { BxPrefixSSEF2, BX_IA_LDDQU_VdqMdq },
  /* 0F F1 /w */ { BxPrefixSSE, BX_IA_PSLLW_PqQq, BxOpcodeGroupSSE_0ff1 },
  /* 0F F2 /w */ { BxPrefixSSE, BX_IA_PSLLD_PqQq, BxOpcodeGroupSSE_0ff2 },
  /* 0F F3 /w */ { BxPrefixSSE, BX_IA_PSLLQ_PqQq, BxOpcodeGroupSSE_0ff3 },
  /* 0F F4 /w */ { BxPrefixSSE, BX_IA_PMULUDQ_PqQq, BxOpcodeGroupSSE_0ff4 },
  /* 0F F5 /w */ { BxPrefixSSE, BX_IA_PMADDWD_PqQq, BxOpcodeGroupSSE_0ff5 },
  /* 0F F6 /w */ { BxPrefixSSE, BX_IA_PSADBW_PqQq, BxOpcodeGroupSSE_0ff6 },
  /* 0F F7 /w */ { BxPrefixSSE, BX_IA_MASKMOVQ_PqPRq, BxOpcodeGroupSSE_0ff7R },
  /* 0F F8 /w */ { BxPrefixSSE, BX_IA_PSUBB_PqQq, BxOpcodeGroupSSE_0ff8 },
  /* 0F F9 /w */ { BxPrefixSSE, BX_IA_PSUBW_PqQq, BxOpcodeGroupSSE_0ff9 },
  /* 0F FA /w */ { BxPrefixSSE, BX_IA_PSUBD_PqQq, BxOpcodeGroupSSE_0ffa },
  /* 0F FB /w */ { BxPrefixSSE, BX_IA_PSUBQ_PqQq, BxOpcodeGroupSSE_0ffb },
  /* 0F FC /w */ { BxPrefixSSE, BX_IA_PADDB_PqQq, BxOpcodeGroupSSE_0ffc },
  /* 0F FD /w */ { BxPrefixSSE, BX_IA_PADDW_PqQq, BxOpcodeGroupSSE_0ffd },
  /* 0F FE /w */ { BxPrefixSSE, BX_IA_PADDD_PqQq, BxOpcodeGroupSSE_0ffe },
  /* 0F FF /w */ { 0, BX_IA_ERROR },

  // 512 entries for 32bit mode
  /* 00 /d */ { BxArithDstRM | BxLockable, BX_IA_ADD_EbGb },
  /* 01 /d */ { BxArithDstRM | BxLockable, BX_IA_ADD_EdGd },
  /* 02 /d */ { 0, BX_IA_ADD_GbEb },
  /* 03 /d */ { 0, BX_IA_ADD_GdEd },
  /* 04 /d */ { BxImmediate_Ib, BX_IA_ADD_ALIb },
  /* 05 /d */ { BxImmediate_Id, BX_IA_ADD_EAXId },
  /* 06 /d */ { 0, BX_IA_ERROR },
  /* 07 /d */ { 0, BX_IA_ERROR },
  /* 08 /d */ { BxArithDstRM | BxLockable, BX_IA_OR_EbGb },
  /* 09 /d */ { BxArithDstRM | BxLockable, BX_IA_OR_EdGd },
  /* 0A /d */ { 0, BX_IA_OR_GbEb },
  /* 0B /d */ { 0, BX_IA_OR_GdEd },
  /* 0C /d */ { BxImmediate_Ib, BX_IA_OR_ALIb },
  /* 0D /d */ { BxImmediate_Id, BX_IA_OR_EAXId },
  /* 0E /d */ { 0, BX_IA_ERROR },
  /* 0F /d */ { 0, BX_IA_ERROR }, // 2-byte escape
  /* 10 /d */ { BxArithDstRM | BxLockable, BX_IA_ADC_EbGb },
  /* 11 /d */ { BxArithDstRM | BxLockable, BX_IA_ADC_EdGd },
  /* 12 /d */ { 0, BX_IA_ADC_GbEb },
  /* 13 /d */ { 0, BX_IA_ADC_GdEd },
  /* 14 /d */ { BxImmediate_Ib, BX_IA_ADC_ALIb },
  /* 15 /d */ { BxImmediate_Id, BX_IA_ADC_EAXId },
  /* 16 /d */ { 0, BX_IA_ERROR },
  /* 17 /d */ { 0, BX_IA_ERROR },
  /* 18 /d */ { BxArithDstRM | BxLockable, BX_IA_SBB_EbGb },
  /* 19 /d */ { BxArithDstRM | BxLockable, BX_IA_SBB_EdGd },
  /* 1A /d */ { 0, BX_IA_SBB_GbEb },
  /* 1B /d */ { 0, BX_IA_SBB_GdEd },
  /* 1C /d */ { BxImmediate_Ib, BX_IA_SBB_ALIb },
  /* 1D /d */ { BxImmediate_Id, BX_IA_SBB_EAXId },
  /* 1E /d */ { 0, BX_IA_ERROR },
  /* 1F /d */ { 0, BX_IA_ERROR },
  /* 20 /d */ { BxArithDstRM | BxLockable, BX_IA_AND_EbGb },
  /* 21 /d */ { BxArithDstRM | BxLockable, BX_IA_AND_EdGd },
  /* 22 /d */ { 0, BX_IA_AND_GbEb },
  /* 23 /d */ { 0, BX_IA_AND_GdEd },
  /* 24 /d */ { BxImmediate_Ib, BX_IA_AND_ALIb },
  /* 25 /d */ { BxImmediate_Id, BX_IA_AND_EAXId },
  /* 26 /d */ { 0, BX_IA_ERROR }, // ES:
  /* 27 /d */ { 0, BX_IA_ERROR },
  /* 28 /d */ { BxArithDstRM | BxLockable, BX_IA_SUB_EbGb },
  /* 29 /d */ { BxArithDstRM | BxLockable, BX_IA_SUB_EdGd },
  /* 2A /d */ { 0, BX_IA_SUB_GbEb },
  /* 2B /d */ { 0, BX_IA_SUB_GdEd },
  /* 2C /d */ { BxImmediate_Ib, BX_IA_SUB_ALIb },
  /* 2D /d */ { BxImmediate_Id, BX_IA_SUB_EAXId },
  /* 2E /d */ { 0, BX_IA_ERROR }, // CS:
  /* 2F /d */ { 0, BX_IA_ERROR },
  /* 30 /d */ { BxArithDstRM | BxLockable, BX_IA_XOR_EbGb },
  /* 31 /d */ { BxArithDstRM | BxLockable, BX_IA_XOR_EdGd },
  /* 32 /d */ { 0, BX_IA_XOR_GbEb },
  /* 33 /d */ { 0, BX_IA_XOR_GdEd },
  /* 34 /d */ { BxImmediate_Ib, BX_IA_XOR_ALIb },
  /* 35 /d */ { BxImmediate_Id, BX_IA_XOR_EAXId },
  /* 36 /d */ { 0, BX_IA_ERROR }, // SS:
  /* 37 /d */ { 0, BX_IA_ERROR },
  /* 38 /d */ { BxArithDstRM, BX_IA_CMP_EbGb },
  /* 39 /d */ { BxArithDstRM, BX_IA_CMP_EdGd },
  /* 3A /d */ { 0, BX_IA_CMP_GbEb },
  /* 3B /d */ { 0, BX_IA_CMP_GdEd },
  /* 3C /d */ { BxImmediate_Ib, BX_IA_CMP_ALIb },
  /* 3D /d */ { BxImmediate_Id, BX_IA_CMP_EAXId },
  /* 3E /d */ { 0, BX_IA_ERROR }, // DS:
  /* 3F /d */ { 0, BX_IA_ERROR },
  /* 40 /d */ { 0, BX_IA_ERROR }, // REX:
  /* 41 /d */ { 0, BX_IA_ERROR }, // REX:
  /* 42 /d */ { 0, BX_IA_ERROR }, // REX:
  /* 43 /d */ { 0, BX_IA_ERROR }, // REX:
  /* 44 /d */ { 0, BX_IA_ERROR }, // REX:
  /* 45 /d */ { 0, BX_IA_ERROR }, // REX:
  /* 46 /d */ { 0, BX_IA_ERROR }, // REX:
  /* 47 /d */ { 0, BX_IA_ERROR }, // REX:
  /* 48 /d */ { 0, BX_IA_ERROR }, // REX:
  /* 49 /d */ { 0, BX_IA_ERROR }, // REX:
  /* 4A /d */ { 0, BX_IA_ERROR }, // REX:
  /* 4B /d */ { 0, BX_IA_ERROR }, // REX:
  /* 4C /d */ { 0, BX_IA_ERROR }, // REX:
  /* 4D /d */ { 0, BX_IA_ERROR }, // REX:
  /* 4E /d */ { 0, BX_IA_ERROR }, // REX:
  /* 4F /d */ { 0, BX_IA_ERROR }, // REX:
  /* 50 /d */ { 0, BX_IA_PUSH_RRX },
  /* 51 /d */ { 0, BX_IA_PUSH_RRX },
  /* 52 /d */ { 0, BX_IA_PUSH_RRX },
  /* 53 /d */ { 0, BX_IA_PUSH_RRX },
  /* 54 /d */ { 0, BX_IA_PUSH_RRX },
  /* 55 /d */ { 0, BX_IA_PUSH_RRX },
  /* 56 /d */ { 0, BX_IA_PUSH_RRX },
  /* 57 /d */ { 0, BX_IA_PUSH_RRX },
  /* 58 /d */ { 0, BX_IA_POP_RRX },
  /* 59 /d */ { 0, BX_IA_POP_RRX },
  /* 5A /d */ { 0, BX_IA_POP_RRX },
  /* 5B /d */ { 0, BX_IA_POP_RRX },
  /* 5C /d */ { 0, BX_IA_POP_RRX },
  /* 5D /d */ { 0, BX_IA_POP_RRX },
  /* 5E /d */ { 0, BX_IA_POP_RRX },
  /* 5F /d */ { 0, BX_IA_POP_RRX },
  /* 60 /d */ { 0, BX_IA_ERROR },
  /* 61 /d */ { 0, BX_IA_ERROR },
  /* 62 /d */ { 0, BX_IA_ERROR },
  /* 63 /d */ { 0, BX_IA_MOV64_GdEd }, // MOVSX_GdEd
  /* 64 /d */ { 0, BX_IA_ERROR }, // FS:
  /* 65 /d */ { 0, BX_IA_ERROR }, // GS:
  /* 66 /d */ { 0, BX_IA_ERROR }, // OS:
  /* 67 /d */ { 0, BX_IA_ERROR }, // AS:
  /* 68 /d */ { BxImmediate_Id, BX_IA_PUSH64_Id },
  /* 69 /d */ { BxImmediate_Id, BX_IA_IMUL_GdEdId },
  /* 6A /d */ { BxImmediate_Ib_SE, BX_IA_PUSH64_Id },
  /* 6B /d */ { BxImmediate_Ib_SE, BX_IA_IMUL_GdEdId },
  /* 6C /d */ { 0, BX_IA_REP_INSB_YbDX },
  /* 6D /d */ { 0, BX_IA_REP_INSD_YdDX },
  /* 6E /d */ { 0, BX_IA_REP_OUTSB_DXXb },
  /* 6F /d */ { 0, BX_IA_REP_OUTSD_DXXd },
  /* 70 /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JO_Jq },
  /* 71 /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNO_Jq },
  /* 72 /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JB_Jq },
  /* 73 /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNB_Jq },
  /* 74 /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JZ_Jq },
  /* 75 /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNZ_Jq },
  /* 76 /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JBE_Jq },
  /* 77 /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNBE_Jq },
  /* 78 /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JS_Jq },
  /* 79 /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNS_Jq },
  /* 7A /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JP_Jq },
  /* 7B /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNP_Jq },
  /* 7C /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JL_Jq },
  /* 7D /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNL_Jq },
  /* 7E /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JLE_Jq },
  /* 7F /d */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNLE_Jq },
  /* 80 /d */ { BxGroup1 | BxImmediate_Ib, BX_IA_ERROR, BxOpcodeInfoG1EbIb },
  /* 81 /d */ { BxGroup1 | BxImmediate_Id, BX_IA_ERROR, BxOpcodeInfoG1Ed },
  /* 82 /d */ { 0, BX_IA_ERROR },
  /* 83 /d */ { BxGroup1 | BxImmediate_Ib_SE, BX_IA_ERROR, BxOpcodeInfoG1Ed },
  /* 84 /d */ { 0, BX_IA_TEST_EbGb },
  /* 85 /d */ { 0, BX_IA_TEST_EdGd },
  /* 86 /d */ { BxLockable, BX_IA_XCHG_EbGb },
  /* 87 /d */ { BxLockable, BX_IA_XCHG_EdGd },
  /* 88 /d */ { BxArithDstRM, BX_IA_MOV_EbGb },
  /* 89 /d */ { BxArithDstRM, BX_IA_MOV64_EdGd },
  /* 8A /d */ { 0, BX_IA_MOV_GbEb },
  /* 8B /d */ { 0, BX_IA_MOV64_GdEd },
  /* 8C /d */ { 0, BX_IA_MOV_EwSw },
  /* 8D /d */ { 0, BX_IA_LEA_GdM },
  /* 8E /d */ { 0, BX_IA_MOV_SwEw },
  /* 8F /d */ { BxGroup1A, BX_IA_ERROR, BxOpcodeInfo64G1AEq },
  /* 90 /d */ { 0, BX_IA_XCHG_ERXEAX }, // handles XCHG R8d, EAX
  /* 91 /d */ { 0, BX_IA_XCHG_ERXEAX },
  /* 92 /d */ { 0, BX_IA_XCHG_ERXEAX },
  /* 93 /d */ { 0, BX_IA_XCHG_ERXEAX },
  /* 94 /d */ { 0, BX_IA_XCHG_ERXEAX },
  /* 95 /d */ { 0, BX_IA_XCHG_ERXEAX },
  /* 96 /d */ { 0, BX_IA_XCHG_ERXEAX },
  /* 97 /d */ { 0, BX_IA_XCHG_ERXEAX },
  /* 98 /d */ { 0, BX_IA_CWDE },
  /* 99 /d */ { 0, BX_IA_CDQ },
  /* 9A /d */ { 0, BX_IA_ERROR },
  /* 9B /d */ { 0, BX_IA_FWAIT },
  /* 9C /d */ { 0, BX_IA_PUSHF_Fq },
  /* 9D /d */ { 0, BX_IA_POPF_Fq },
  /* 9E /d */ { 0, BX_IA_SAHF },
  /* 9F /d */ { 0, BX_IA_LAHF },
  /* A0 /d */ { BxImmediate_O, BX_IA_MOV_ALOq },
  /* A1 /d */ { BxImmediate_O, BX_IA_MOV_EAXOq },
  /* A2 /d */ { BxImmediate_O, BX_IA_MOV_OqAL },
  /* A3 /d */ { BxImmediate_O, BX_IA_MOV_OqEAX },
  /* A4 /d */ { 0, BX_IA_REP_MOVSB_XbYb },
  /* A5 /d */ { 0, BX_IA_REP_MOVSD_XdYd },
  /* A6 /d */ { 0, BX_IA_REP_CMPSB_XbYb },
  /* A7 /d */ { 0, BX_IA_REP_CMPSD_XdYd },
  /* A8 /d */ { BxImmediate_Ib, BX_IA_TEST_ALIb },
  /* A9 /d */ { BxImmediate_Id, BX_IA_TEST_EAXId },
  /* AA /d */ { 0, BX_IA_REP_STOSB_YbAL },
  /* AB /d */ { 0, BX_IA_REP_STOSD_YdEAX },
  /* AC /d */ { 0, BX_IA_REP_LODSB_ALXb },
  /* AD /d */ { 0, BX_IA_REP_LODSD_EAXXd },
  /* AE /d */ { 0, BX_IA_REP_SCASB_ALXb  },
  /* AF /d */ { 0, BX_IA_REP_SCASD_EAXXd },
  /* B0 /d */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B1 /d */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B2 /d */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B3 /d */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B4 /d */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B5 /d */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B6 /d */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B7 /d */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B8 /d */ { BxImmediate_Id, BX_IA_MOV_ERXId },
  /* B9 /d */ { BxImmediate_Id, BX_IA_MOV_ERXId },
  /* BA /d */ { BxImmediate_Id, BX_IA_MOV_ERXId },
  /* BB /d */ { BxImmediate_Id, BX_IA_MOV_ERXId },
  /* BC /d */ { BxImmediate_Id, BX_IA_MOV_ERXId },
  /* BD /d */ { BxImmediate_Id, BX_IA_MOV_ERXId },
  /* BE /d */ { BxImmediate_Id, BX_IA_MOV_ERXId },
  /* BF /d */ { BxImmediate_Id, BX_IA_MOV_ERXId },
  /* C0 /d */ { BxGroup2 | BxImmediate_Ib, BX_IA_ERROR, BxOpcodeInfoG2Eb },
  /* C1 /d */ { BxGroup2 | BxImmediate_Ib, BX_IA_ERROR, BxOpcodeInfoG2Ed },
  /* C2 /d */ { BxImmediate_Iw | BxTraceEnd, BX_IA_RETnear64_Iw },
  /* C3 /d */ { BxTraceEnd,                  BX_IA_RETnear64 },
  /* C4 /d */ { 0, BX_IA_ERROR },
  /* C5 /d */ { 0, BX_IA_ERROR },
  /* C6 /d */ { BxGroup11, BX_IA_ERROR, BxOpcodeInfoG11Eb },
  /* C7 /d */ { BxGroup11, BX_IA_ERROR, BxOpcodeInfoG11Ed },
  /* C8 /d */ { BxImmediate_Iw | BxImmediate_Ib2, BX_IA_ENTER64_IwIb },
  /* C9 /d */ { 0, BX_IA_LEAVE64 },
  /* CA /d */ { BxImmediate_Iw | BxTraceEnd, BX_IA_RETfar32_Iw },
  /* CB /d */ { BxTraceEnd,                  BX_IA_RETfar32 },
  /* CC /d */ { BxTraceEnd, BX_IA_INT3 },
  /* CD /d */ { BxImmediate_Ib | BxTraceEnd, BX_IA_INT_Ib },
  /* CE /d */ { 0, BX_IA_ERROR },
  /* CF /d */ { BxTraceEnd, BX_IA_IRET64 },
  /* D0 /d */ { BxGroup2 | BxImmediate_I1, BX_IA_ERROR, BxOpcodeInfoG2Eb },
  /* D1 /d */ { BxGroup2 | BxImmediate_I1, BX_IA_ERROR, BxOpcodeInfoG2Ed },
  /* D2 /d */ { BxGroup2, BX_IA_ERROR, BxOpcodeInfoG2Eb },
  /* D3 /d */ { BxGroup2, BX_IA_ERROR, BxOpcodeInfoG2Ed },
  /* D4 /d */ { 0, BX_IA_ERROR },
  /* D5 /d */ { 0, BX_IA_ERROR },
  /* D6 /d */ { 0, BX_IA_ERROR },
  /* D7 /d */ { 0, BX_IA_XLAT },
  /* D8 /d */ { BxGroupFP, BX_IA_ERROR, BxOpcodeInfo_FPGroupD8 },
  /* D9 /d */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointD9 },
  /* DA /d */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDA },
  /* DB /d */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDB },
  /* DC /d */ { BxGroupFP, BX_IA_ERROR, BxOpcodeInfo_FPGroupDC },
  /* DD /d */ { BxGroupFP, BX_IA_ERROR, BxOpcodeInfo_FPGroupDD },
  /* DE /d */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDE },
  /* DF /d */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDF },
  /* E0 /d */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_LOOPNE64_Jb },
  /* E1 /d */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_LOOPE64_Jb },
  /* E2 /d */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_LOOP64_Jb },
  /* E3 /d */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_JRCXZ_Jb },
  /* E4 /d */ { BxImmediate_Ib, BX_IA_IN_ALIb },
  /* E5 /d */ { BxImmediate_Ib, BX_IA_IN_EAXIb },
  /* E6 /d */ { BxImmediate_Ib, BX_IA_OUT_IbAL },
  /* E7 /d */ { BxImmediate_Ib, BX_IA_OUT_IbEAX },
  /* E8 /d */ { BxImmediate_BrOff32 | BxTraceEnd, BX_IA_CALL_Jq },
  /* E9 /d */ { BxImmediate_BrOff32 | BxTraceEnd, BX_IA_JMP_Jq },
  /* EA /d */ { 0, BX_IA_ERROR },
  /* EB /d */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_JMP_Jq },
  /* EC /d */ { 0, BX_IA_IN_ALDX },
  /* ED /d */ { 0, BX_IA_IN_EAXDX },
  /* EE /d */ { 0, BX_IA_OUT_DXAL },
  /* EF /d */ { 0, BX_IA_OUT_DXEAX },
  /* F0 /d */ { 0, BX_IA_ERROR }, // LOCK:
  /* F1 /d */ { BxTraceEnd, BX_IA_INT1 },
  /* F2 /d */ { 0, BX_IA_ERROR }, // REPNE/REPNZ
  /* F3 /d */ { 0, BX_IA_ERROR }, // REP,REPE/REPZ
  /* F4 /d */ { BxTraceEnd, BX_IA_HLT },
  /* F5 /d */ { 0, BX_IA_CMC },
  /* F6 /d */ { BxGroup3, BX_IA_ERROR, BxOpcodeInfoG3Eb },
  /* F7 /d */ { BxGroup3, BX_IA_ERROR, BxOpcodeInfoG3Ed },
  /* F8 /d */ { 0, BX_IA_CLC },
  /* F9 /d */ { 0, BX_IA_STC },
  /* FA /d */ { 0, BX_IA_CLI },
  /* FB /d */ { 0, BX_IA_STI },
  /* FC /d */ { 0, BX_IA_CLD },
  /* FD /d */ { 0, BX_IA_STD },
  /* FE /d */ { BxGroup4, BX_IA_ERROR, BxOpcodeInfoG4 },
  /* FF /d */ { BxGroup5, BX_IA_ERROR, BxOpcodeInfo64G5d },

  /* 0F 00 /d */ { BxGroup6, BX_IA_ERROR, BxOpcodeInfoG6 },
  /* 0F 01 /d */ { BxGroup7, BX_IA_ERROR, BxOpcodeInfoG7q },
  /* 0F 02 /d */ { 0, BX_IA_LAR_GvEw },
  /* 0F 03 /d */ { 0, BX_IA_LSL_GvEw },
  /* 0F 04 /d */ { 0, BX_IA_ERROR },
  /* 0F 05 /d */ { BxTraceEnd, BX_IA_SYSCALL },
  /* 0F 06 /d */ { 0, BX_IA_CLTS },
  /* 0F 07 /d */ { BxTraceEnd, BX_IA_SYSRET },
  /* 0F 08 /d */ { BxTraceEnd, BX_IA_INVD },
  /* 0F 09 /d */ { BxTraceEnd, BX_IA_WBINVD },
  /* 0F 0A /d */ { 0, BX_IA_ERROR },
  /* 0F 0B /d */ { BxTraceEnd, BX_IA_UD2A },
  /* 0F 0C /d */ { 0, BX_IA_ERROR },
  /* 0F 0D /d */ { 0, BX_IA_PREFETCHW },          // 3DNow! PREFETCHW on AMD, NOP on Intel
  /* 0F 0E /d */ { 0, BX_IA_FEMMS },              // 3DNow! FEMMS
  /* 0F 0F /d */ { BxImmediate_Ib, BX_IA_ERROR }, // 3DNow! Opcode Table
  /* 0F 10 /d */ { BxPrefixSSE,                BX_IA_MOVUPS_VpsWps, BxOpcodeGroupSSE_0f10 },
  /* 0F 11 /d */ { BxPrefixSSE | BxArithDstRM, BX_IA_MOVUPS_WpsVps, BxOpcodeGroupSSE_0f11 },
  /* 0F 12 /d */ { BxPrefixSSE, BX_IA_MOVLPS_VpsMq, BxOpcodeGroupSSE_0f12 },
  /* 0F 13 /d */ { BxPrefixSSE, BX_IA_MOVLPS_MqVps, BxOpcodeGroupSSE_0f13M },
  /* 0F 14 /d */ { BxPrefixSSE, BX_IA_UNPCKLPS_VpsWdq, BxOpcodeGroupSSE_0f14 },
  /* 0F 15 /d */ { BxPrefixSSE, BX_IA_UNPCKHPS_VpsWdq, BxOpcodeGroupSSE_0f15 },
  /* 0F 16 /d */ { BxPrefixSSE, BX_IA_MOVHPS_VpsMq, BxOpcodeGroupSSE_0f16 },
  /* 0F 17 /d */ { BxPrefixSSE, BX_IA_MOVHPS_MqVps, BxOpcodeGroupSSE_0f17M },
  /* 0F 18 /d */ { 0, BX_IA_PREFETCH }, // opcode group G16, PREFETCH hints
  /* 0F 19 /d */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1A /d */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1B /d */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1C /d */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1D /d */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1E /d */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1F /d */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 20 /d */ { BxGroupN, BX_IA_ERROR, BxOpcodeInfoMOV_RqCq },
  /* 0F 21 /d */ { 0, BX_IA_MOV_RqDq },
  /* 0F 22 /d */ { BxGroupN, BX_IA_ERROR, BxOpcodeInfoMOV_CqRq },
  /* 0F 23 /d */ { BxTraceEnd, BX_IA_MOV_DqRq },
  /* 0F 24 /d */ { 0, BX_IA_ERROR },
  /* 0F 25 /d */ { 0, BX_IA_ERROR },
  /* 0F 26 /d */ { 0, BX_IA_ERROR },
  /* 0F 27 /d */ { 0, BX_IA_ERROR },
  /* 0F 28 /d */ { BxPrefixSSE,                BX_IA_MOVAPS_VpsWps, BxOpcodeGroupSSE_0f28 },
  /* 0F 29 /d */ { BxPrefixSSE | BxArithDstRM, BX_IA_MOVAPS_WpsVps, BxOpcodeGroupSSE_0f29 },
  /* 0F 2A /d */ { BxPrefixSSE, BX_IA_CVTPI2PS_VpsQq, BxOpcodeGroupSSE_0f2a },
  /* 0F 2B /d */ { BxPrefixSSE, BX_IA_MOVNTPS_MpsVps, BxOpcodeGroupSSE_0f2bM },
  /* 0F 2C /d */ { BxPrefixSSE, BX_IA_CVTTPS2PI_PqWps, BxOpcodeGroupSSE_0f2c },
  /* 0F 2D /d */ { BxPrefixSSE, BX_IA_CVTPS2PI_PqWps, BxOpcodeGroupSSE_0f2d },
  /* 0F 2E /d */ { BxPrefixSSE, BX_IA_UCOMISS_VssWss, BxOpcodeGroupSSE_0f2e },
  /* 0F 2F /d */ { BxPrefixSSE, BX_IA_COMISS_VpsWps, BxOpcodeGroupSSE_0f2f },
  /* 0F 30 /d */ { 0, BX_IA_WRMSR },
  /* 0F 31 /d */ { 0, BX_IA_RDTSC },
  /* 0F 32 /d */ { 0, BX_IA_RDMSR },
  /* 0F 33 /d */ { 0, BX_IA_RDPMC },
  /* 0F 34 /d */ { BxTraceEnd, BX_IA_SYSENTER },
  /* 0F 35 /d */ { BxTraceEnd, BX_IA_SYSEXIT },
  /* 0F 36 /d */ { 0, BX_IA_ERROR },
  /* 0F 37 /d */ { 0, BX_IA_ERROR },
  /* 0F 38 /d */ { Bx3ByteOp, BX_IA_ERROR, BxOpcode3ByteTable0f38 }, // 3-byte escape
  /* 0F 39 /d */ { 0, BX_IA_ERROR },
  /* 0F 3A /d */ { Bx3ByteOp | BxImmediate_Ib, BX_IA_ERROR, BxOpcode3ByteTable0f3a }, // 3-byte escape
  /* 0F 3B /d */ { 0, BX_IA_ERROR },
  /* 0F 3C /d */ { 0, BX_IA_ERROR },
  /* 0F 3D /d */ { 0, BX_IA_ERROR },
  /* 0F 3E /d */ { 0, BX_IA_ERROR },
  /* 0F 3F /d */ { 0, BX_IA_ERROR },
  /* 0F 40 /d */ { 0, BX_IA_CMOVO_GdEd },
  /* 0F 41 /d */ { 0, BX_IA_CMOVNO_GdEd },
  /* 0F 42 /d */ { 0, BX_IA_CMOVB_GdEd },
  /* 0F 43 /d */ { 0, BX_IA_CMOVNB_GdEd },
  /* 0F 44 /d */ { 0, BX_IA_CMOVZ_GdEd },
  /* 0F 45 /d */ { 0, BX_IA_CMOVNZ_GdEd },
  /* 0F 46 /d */ { 0, BX_IA_CMOVBE_GdEd },
  /* 0F 47 /d */ { 0, BX_IA_CMOVNBE_GdEd },
  /* 0F 48 /d */ { 0, BX_IA_CMOVS_GdEd },
  /* 0F 49 /d */ { 0, BX_IA_CMOVNS_GdEd },
  /* 0F 4A /d */ { 0, BX_IA_CMOVP_GdEd },
  /* 0F 4B /d */ { 0, BX_IA_CMOVNP_GdEd },
  /* 0F 4C /d */ { 0, BX_IA_CMOVL_GdEd },
  /* 0F 4D /d */ { 0, BX_IA_CMOVNL_GdEd },
  /* 0F 4E /d */ { 0, BX_IA_CMOVLE_GdEd },
  /* 0F 4F /d */ { 0, BX_IA_CMOVNLE_GdEd },
  /* 0F 50 /d */ { BxPrefixSSE, BX_IA_MOVMSKPS_GdVRps, BxOpcodeGroupSSE_0f50R },
  /* 0F 51 /d */ { BxPrefixSSE, BX_IA_SQRTPS_VpsWps, BxOpcodeGroupSSE_0f51 },
  /* 0F 52 /d */ { BxPrefixSSE, BX_IA_RSQRTPS_VpsWps, BxOpcodeGroupSSE_0f52 },
  /* 0F 53 /d */ { BxPrefixSSE, BX_IA_RCPPS_VpsWps, BxOpcodeGroupSSE_0f53 },
  /* 0F 54 /d */ { BxPrefixSSE, BX_IA_ANDPS_VpsWps, BxOpcodeGroupSSE_0f54 },
  /* 0F 55 /d */ { BxPrefixSSE, BX_IA_ANDNPS_VpsWps, BxOpcodeGroupSSE_0f55 },
  /* 0F 56 /d */ { BxPrefixSSE, BX_IA_ORPS_VpsWps, BxOpcodeGroupSSE_0f56 },
  /* 0F 57 /d */ { BxPrefixSSE, BX_IA_XORPS_VpsWps, BxOpcodeGroupSSE_0f57 },
  /* 0F 58 /d */ { BxPrefixSSE, BX_IA_ADDPS_VpsWps, BxOpcodeGroupSSE_0f58 },
  /* 0F 59 /d */ { BxPrefixSSE, BX_IA_MULPS_VpsWps, BxOpcodeGroupSSE_0f59 },
  /* 0F 5A /d */ { BxPrefixSSE, BX_IA_CVTPS2PD_VpsWps, BxOpcodeGroupSSE_0f5a },
  /* 0F 5B /d */ { BxPrefixSSE, BX_IA_CVTDQ2PS_VpsWdq, BxOpcodeGroupSSE_0f5b },
  /* 0F 5C /d */ { BxPrefixSSE, BX_IA_SUBPS_VpsWps, BxOpcodeGroupSSE_0f5c },
  /* 0F 5D /d */ { BxPrefixSSE, BX_IA_MINPS_VpsWps, BxOpcodeGroupSSE_0f5d },
  /* 0F 5E /d */ { BxPrefixSSE, BX_IA_DIVPS_VpsWps, BxOpcodeGroupSSE_0f5e },
  /* 0F 5F /d */ { BxPrefixSSE, BX_IA_MAXPS_VpsWps, BxOpcodeGroupSSE_0f5f },
  /* 0F 60 /d */ { BxPrefixSSE, BX_IA_PUNPCKLBW_PqQd, BxOpcodeGroupSSE_0f60 },
  /* 0F 61 /d */ { BxPrefixSSE, BX_IA_PUNPCKLWD_PqQd, BxOpcodeGroupSSE_0f61 },
  /* 0F 62 /d */ { BxPrefixSSE, BX_IA_PUNPCKLDQ_PqQd, BxOpcodeGroupSSE_0f62 },
  /* 0F 63 /d */ { BxPrefixSSE, BX_IA_PACKSSWB_PqQq, BxOpcodeGroupSSE_0f63 },
  /* 0F 64 /d */ { BxPrefixSSE, BX_IA_PCMPGTB_PqQq, BxOpcodeGroupSSE_0f64 },
  /* 0F 65 /d */ { BxPrefixSSE, BX_IA_PCMPGTW_PqQq, BxOpcodeGroupSSE_0f65 },
  /* 0F 66 /d */ { BxPrefixSSE, BX_IA_PCMPGTD_PqQq, BxOpcodeGroupSSE_0f66 },
  /* 0F 67 /d */ { BxPrefixSSE, BX_IA_PACKUSWB_PqQq, BxOpcodeGroupSSE_0f67 },
  /* 0F 68 /d */ { BxPrefixSSE, BX_IA_PUNPCKHBW_PqQq, BxOpcodeGroupSSE_0f68 },
  /* 0F 69 /d */ { BxPrefixSSE, BX_IA_PUNPCKHWD_PqQq, BxOpcodeGroupSSE_0f69 },
  /* 0F 6A /d */ { BxPrefixSSE, BX_IA_PUNPCKHDQ_PqQq, BxOpcodeGroupSSE_0f6a },
  /* 0F 6B /d */ { BxPrefixSSE, BX_IA_PACKSSDW_PqQq, BxOpcodeGroupSSE_0f6b },
  /* 0F 6C /d */ { BxPrefixSSE66, BX_IA_PUNPCKLQDQ_VdqWdq },
  /* 0F 6D /d */ { BxPrefixSSE66, BX_IA_PUNPCKHQDQ_VdqWdq },
  /* 0F 6E /d */ { BxPrefixSSE, BX_IA_MOVD_PqEd, BxOpcodeGroupSSE_0f6e },
  /* 0F 6F /d */ { BxPrefixSSE, BX_IA_MOVQ_PqQq, BxOpcodeGroupSSE_0f6f },
  /* 0F 70 /d */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_PSHUFW_PqQqIb, BxOpcodeGroupSSE_0f70 },
  /* 0F 71 /d */ { BxGroup12, BX_IA_ERROR, BxOpcodeInfoG12R },
  /* 0F 72 /d */ { BxGroup13, BX_IA_ERROR, BxOpcodeInfoG13R },
  /* 0F 73 /d */ { BxGroup14, BX_IA_ERROR, BxOpcodeInfoG14R },
  /* 0F 74 /d */ { BxPrefixSSE, BX_IA_PCMPEQB_PqQq, BxOpcodeGroupSSE_0f74 },
  /* 0F 75 /d */ { BxPrefixSSE, BX_IA_PCMPEQW_PqQq, BxOpcodeGroupSSE_0f75 },
  /* 0F 76 /d */ { BxPrefixSSE, BX_IA_PCMPEQD_PqQq, BxOpcodeGroupSSE_0f76 },
  /* 0F 77 /d */ { BxPrefixSSE, BX_IA_EMMS, BxOpcodeGroupSSE_ERR },
  /* 0F 78 /d */ { BxPrefixSSE, BX_IA_VMREAD_EqGq, BxOpcodeGroupSSE_ERR },
  /* 0F 79 /d */ { BxPrefixSSE, BX_IA_VMWRITE_GqEq, BxOpcodeGroupSSE_ERR },
  /* 0F 7A /d */ { 0, BX_IA_ERROR },
  /* 0F 7B /d */ { 0, BX_IA_ERROR },
  /* 0F 7C /d */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0f7c },
  /* 0F 7D /d */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0f7d },
  /* 0F 7E /d */ { BxPrefixSSE, BX_IA_MOVD_EdPd, BxOpcodeGroupSSE_0f7e },
  /* 0F 7F /d */ { BxPrefixSSE, BX_IA_MOVQ_QqPq, BxOpcodeGroupSSE_0f7f },
  /* 0F 80 /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JO_Jq },
  /* 0F 81 /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNO_Jq },
  /* 0F 82 /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JB_Jq },
  /* 0F 83 /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNB_Jq },
  /* 0F 84 /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JZ_Jq },
  /* 0F 85 /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNZ_Jq },
  /* 0F 86 /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JBE_Jq },
  /* 0F 87 /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNBE_Jq },
  /* 0F 88 /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JS_Jq },
  /* 0F 89 /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNS_Jq },
  /* 0F 8A /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JP_Jq },
  /* 0F 8B /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNP_Jq },
  /* 0F 8C /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JL_Jq },
  /* 0F 8D /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNL_Jq },
  /* 0F 8E /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JLE_Jq },
  /* 0F 8F /d */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNLE_Jq },
  /* 0F 90 /d */ { 0, BX_IA_SETO_Eb },
  /* 0F 91 /d */ { 0, BX_IA_SETNO_Eb },
  /* 0F 92 /d */ { 0, BX_IA_SETB_Eb },
  /* 0F 93 /d */ { 0, BX_IA_SETNB_Eb },
  /* 0F 94 /d */ { 0, BX_IA_SETZ_Eb },
  /* 0F 95 /d */ { 0, BX_IA_SETNZ_Eb },
  /* 0F 96 /d */ { 0, BX_IA_SETBE_Eb },
  /* 0F 97 /d */ { 0, BX_IA_SETNBE_Eb },
  /* 0F 98 /d */ { 0, BX_IA_SETS_Eb },
  /* 0F 99 /d */ { 0, BX_IA_SETNS_Eb },
  /* 0F 9A /d */ { 0, BX_IA_SETP_Eb },
  /* 0F 9B /d */ { 0, BX_IA_SETNP_Eb },
  /* 0F 9C /d */ { 0, BX_IA_SETL_Eb },
  /* 0F 9D /d */ { 0, BX_IA_SETNL_Eb },
  /* 0F 9E /d */ { 0, BX_IA_SETLE_Eb },
  /* 0F 9F /d */ { 0, BX_IA_SETNLE_Eb },
  /* 0F A0 /d */ { 0, BX_IA_PUSH64_FS },
  /* 0F A1 /d */ { 0, BX_IA_POP64_FS },
  /* 0F A2 /d */ { 0, BX_IA_CPUID },
  /* 0F A3 /d */ { 0, BX_IA_BT_EdGd },
  /* 0F A4 /d */ { BxImmediate_Ib, BX_IA_SHLD_EdGd },
  /* 0F A5 /d */ { 0,              BX_IA_SHLD_EdGd },
  /* 0F A6 /d */ { 0, BX_IA_ERROR },
  /* 0F A7 /d */ { 0, BX_IA_ERROR },
  /* 0F A8 /d */ { 0, BX_IA_PUSH64_GS },
  /* 0F A9 /d */ { 0, BX_IA_POP64_GS },
  /* 0F AA /d */ { BxTraceEnd, BX_IA_RSM },
  /* 0F AB /d */ { BxLockable, BX_IA_BTS_EdGd },
  /* 0F AC /d */ { BxImmediate_Ib, BX_IA_SHRD_EdGd },
  /* 0F AD /d */ { 0,              BX_IA_SHRD_EdGd },
  /* 0F AE /d */ { BxGroup15, BX_IA_ERROR, BxOpcodeInfoG15q },
  /* 0F AF /d */ { 0, BX_IA_IMUL_GdEd },
  /* 0F B0 /d */ { BxLockable, BX_IA_CMPXCHG_EbGb },
  /* 0F B1 /d */ { BxLockable, BX_IA_CMPXCHG_EdGd },
  /* 0F B2 /d */ { 0, BX_IA_LSS_GdMp },
  /* 0F B3 /d */ { BxLockable, BX_IA_BTR_EdGd },
  /* 0F B4 /d */ { 0, BX_IA_LFS_GdMp },
  /* 0F B5 /d */ { 0, BX_IA_LGS_GdMp },
  /* 0F B6 /d */ { 0, BX_IA_MOVZX_GdEb },
  /* 0F B7 /d */ { 0, BX_IA_MOVZX_GdEw },
  /* 0F B8 /d */ { BxPrefixSSEF3, BX_IA_POPCNT_GdEd },
  /* 0F B9 /d */ { BxTraceEnd, BX_IA_UD2B },
  /* 0F BA /d */ { BxGroup8, BX_IA_ERROR, BxOpcodeInfoG8EdIb },
  /* 0F BB /d */ { BxLockable, BX_IA_BTC_EdGd },
  /* 0F BC /d */ { 0, BX_IA_BSF_GdEd },
  /* 0F BD /d */ { 0, BX_IA_BSR_GdEd },
  /* 0F BE /d */ { 0, BX_IA_MOVSX_GdEb },
  /* 0F BF /d */ { 0, BX_IA_MOVSX_GdEw },
  /* 0F C0 /d */ { BxLockable, BX_IA_XADD_EbGb },
  /* 0F C1 /d */ { BxLockable, BX_IA_XADD_EdGd },
  /* 0F C2 /d */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_CMPPS_VpsWpsIb, BxOpcodeGroupSSE_0fc2 },
  /* 0F C3 /d */ { BxPrefixSSE, BX_IA_MOVNTI_MdGd, BxOpcodeGroupSSE_ERR },
  /* 0F C4 /d */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_PINSRW_PqEwIb, BxOpcodeGroupSSE_0fc4 },
  /* 0F C5 /d */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_PEXTRW_GdPqIb, BxOpcodeGroupSSE_0fc5R },
  /* 0F C6 /d */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_SHUFPS_VpsWpsIb, BxOpcodeGroupSSE_0fc6 },
  /* 0F C7 /d */ { BxGroup9, BX_IA_ERROR, BxOpcodeInfoG9M },
  /* 0F C8 /d */ { 0, BX_IA_BSWAP_ERX },
  /* 0F C9 /d */ { 0, BX_IA_BSWAP_ERX },
  /* 0F CA /d */ { 0, BX_IA_BSWAP_ERX },
  /* 0F CB /d */ { 0, BX_IA_BSWAP_ERX },
  /* 0F CC /d */ { 0, BX_IA_BSWAP_ERX },
  /* 0F CD /d */ { 0, BX_IA_BSWAP_ERX },
  /* 0F CE /d */ { 0, BX_IA_BSWAP_ERX },
  /* 0F CF /d */ { 0, BX_IA_BSWAP_ERX },
  /* 0F D0 /d */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0fd0 },
  /* 0F D1 /d */ { BxPrefixSSE, BX_IA_PSRLW_PqQq, BxOpcodeGroupSSE_0fd1 },
  /* 0F D2 /d */ { BxPrefixSSE, BX_IA_PSRLD_PqQq, BxOpcodeGroupSSE_0fd2 },
  /* 0F D3 /d */ { BxPrefixSSE, BX_IA_PSRLQ_PqQq, BxOpcodeGroupSSE_0fd3 },
  /* 0F D4 /d */ { BxPrefixSSE, BX_IA_PADDQ_PqQq, BxOpcodeGroupSSE_0fd4 },
  /* 0F D5 /d */ { BxPrefixSSE, BX_IA_PMULLW_PqQq, BxOpcodeGroupSSE_0fd5 },
  /* 0F D6 /d */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0fd6 },
  /* 0F D7 /d */ { BxPrefixSSE, BX_IA_PMOVMSKB_GdPRq, BxOpcodeGroupSSE_0fd7R },
  /* 0F D8 /d */ { BxPrefixSSE, BX_IA_PSUBUSB_PqQq, BxOpcodeGroupSSE_0fd8 },
  /* 0F D9 /d */ { BxPrefixSSE, BX_IA_PSUBUSW_PqQq, BxOpcodeGroupSSE_0fd9 },
  /* 0F DA /d */ { BxPrefixSSE, BX_IA_PMINUB_PqQq, BxOpcodeGroupSSE_0fda },
  /* 0F DB /d */ { BxPrefixSSE, BX_IA_PAND_PqQq, BxOpcodeGroupSSE_0fdb },
  /* 0F DC /d */ { BxPrefixSSE, BX_IA_PADDUSB_PqQq, BxOpcodeGroupSSE_0fdc },
  /* 0F DD /d */ { BxPrefixSSE, BX_IA_PADDUSW_PqQq, BxOpcodeGroupSSE_0fdd },
  /* 0F DE /d */ { BxPrefixSSE, BX_IA_PMAXUB_PqQq, BxOpcodeGroupSSE_0fde },
  /* 0F DF /d */ { BxPrefixSSE, BX_IA_PANDN_PqQq, BxOpcodeGroupSSE_0fdf },
  /* 0F E0 /d */ { BxPrefixSSE, BX_IA_PAVGB_PqQq, BxOpcodeGroupSSE_0fe0 },
  /* 0F E1 /d */ { BxPrefixSSE, BX_IA_PSRAW_PqQq, BxOpcodeGroupSSE_0fe1 },
  /* 0F E2 /d */ { BxPrefixSSE, BX_IA_PSRAD_PqQq, BxOpcodeGroupSSE_0fe2 },
  /* 0F E3 /d */ { BxPrefixSSE, BX_IA_PAVGW_PqQq, BxOpcodeGroupSSE_0fe3 },
  /* 0F E4 /d */ { BxPrefixSSE, BX_IA_PMULHUW_PqQq, BxOpcodeGroupSSE_0fe4 },
  /* 0F E5 /d */ { BxPrefixSSE, BX_IA_PMULHW_PqQq, BxOpcodeGroupSSE_0fe5 },
  /* 0F E6 /d */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0fe6 },
  /* 0F E7 /d */ { BxPrefixSSE, BX_IA_MOVNTQ_MqPq, BxOpcodeGroupSSE_0fe7M },
  /* 0F E8 /d */ { BxPrefixSSE, BX_IA_PSUBSB_PqQq, BxOpcodeGroupSSE_0fe8 },
  /* 0F E9 /d */ { BxPrefixSSE, BX_IA_PSUBSW_PqQq, BxOpcodeGroupSSE_0fe9 },
  /* 0F EA /d */ { BxPrefixSSE, BX_IA_PMINSW_PqQq, BxOpcodeGroupSSE_0fea },
  /* 0F EB /d */ { BxPrefixSSE, BX_IA_POR_PqQq, BxOpcodeGroupSSE_0feb },
  /* 0F EC /d */ { BxPrefixSSE, BX_IA_PADDSB_PqQq, BxOpcodeGroupSSE_0fec },
  /* 0F ED /d */ { BxPrefixSSE, BX_IA_PADDSW_PqQq, BxOpcodeGroupSSE_0fed },
  /* 0F EE /d */ { BxPrefixSSE, BX_IA_PMAXSW_PqQq, BxOpcodeGroupSSE_0fee },
  /* 0F EF /d */ { BxPrefixSSE, BX_IA_PXOR_PqQq, BxOpcodeGroupSSE_0fef },
  /* 0F F0 /d */ { BxPrefixSSEF2, BX_IA_LDDQU_VdqMdq },
  /* 0F F1 /d */ { BxPrefixSSE, BX_IA_PSLLW_PqQq, BxOpcodeGroupSSE_0ff1 },
  /* 0F F2 /d */ { BxPrefixSSE, BX_IA_PSLLD_PqQq, BxOpcodeGroupSSE_0ff2 },
  /* 0F F3 /d */ { BxPrefixSSE, BX_IA_PSLLQ_PqQq, BxOpcodeGroupSSE_0ff3 },
  /* 0F F4 /d */ { BxPrefixSSE, BX_IA_PMULUDQ_PqQq, BxOpcodeGroupSSE_0ff4 },
  /* 0F F5 /d */ { BxPrefixSSE, BX_IA_PMADDWD_PqQq, BxOpcodeGroupSSE_0ff5 },
  /* 0F F6 /d */ { BxPrefixSSE, BX_IA_PSADBW_PqQq, BxOpcodeGroupSSE_0ff6 },
  /* 0F F7 /d */ { BxPrefixSSE, BX_IA_MASKMOVQ_PqPRq, BxOpcodeGroupSSE_0ff7R },
  /* 0F F8 /d */ { BxPrefixSSE, BX_IA_PSUBB_PqQq, BxOpcodeGroupSSE_0ff8 },
  /* 0F F9 /d */ { BxPrefixSSE, BX_IA_PSUBW_PqQq, BxOpcodeGroupSSE_0ff9 },
  /* 0F FA /d */ { BxPrefixSSE, BX_IA_PSUBD_PqQq, BxOpcodeGroupSSE_0ffa },
  /* 0F FB /d */ { BxPrefixSSE, BX_IA_PSUBQ_PqQq, BxOpcodeGroupSSE_0ffb },
  /* 0F FC /d */ { BxPrefixSSE, BX_IA_PADDB_PqQq, BxOpcodeGroupSSE_0ffc },
  /* 0F FD /d */ { BxPrefixSSE, BX_IA_PADDW_PqQq, BxOpcodeGroupSSE_0ffd },
  /* 0F FE /d */ { BxPrefixSSE, BX_IA_PADDD_PqQq, BxOpcodeGroupSSE_0ffe },
  /* 0F FF /d */ { 0, BX_IA_ERROR },

  // 512 entries for 64bit mode
  /* 00 /q */ { BxArithDstRM | BxLockable, BX_IA_ADD_EbGb },
  /* 01 /q */ { BxArithDstRM | BxLockable, BX_IA_ADD_EqGq },
  /* 02 /q */ { 0, BX_IA_ADD_GbEb },
  /* 03 /q */ { 0, BX_IA_ADD_GqEq },
  /* 04 /q */ { BxImmediate_Ib, BX_IA_ADD_ALIb },
  /* 05 /q */ { BxImmediate_Id, BX_IA_ADD_RAXId },
  /* 06 /q */ { 0, BX_IA_ERROR },
  /* 07 /q */ { 0, BX_IA_ERROR },
  /* 08 /q */ { BxArithDstRM | BxLockable, BX_IA_OR_EbGb },
  /* 09 /q */ { BxArithDstRM | BxLockable, BX_IA_OR_EqGq },
  /* 0A /q */ { 0, BX_IA_OR_GbEb },
  /* 0B /q */ { 0, BX_IA_OR_GqEq },
  /* 0C /q */ { BxImmediate_Ib, BX_IA_OR_ALIb },
  /* 0D /q */ { BxImmediate_Id, BX_IA_OR_RAXId },
  /* 0E /q */ { 0, BX_IA_ERROR },
  /* 0F /q */ { 0, BX_IA_ERROR }, // 2-byte escape
  /* 10 /q */ { BxArithDstRM | BxLockable, BX_IA_ADC_EbGb },
  /* 11 /q */ { BxArithDstRM | BxLockable, BX_IA_ADC_EqGq },
  /* 12 /q */ { 0, BX_IA_ADC_GbEb },
  /* 13 /q */ { 0, BX_IA_ADC_GqEq },
  /* 14 /q */ { BxImmediate_Ib, BX_IA_ADC_ALIb },
  /* 15 /q */ { BxImmediate_Id, BX_IA_ADC_RAXId },
  /* 16 /q */ { 0, BX_IA_ERROR },
  /* 17 /q */ { 0, BX_IA_ERROR },
  /* 18 /q */ { BxArithDstRM | BxLockable, BX_IA_SBB_EbGb },
  /* 19 /q */ { BxArithDstRM | BxLockable, BX_IA_SBB_EqGq },
  /* 1A /q */ { 0, BX_IA_SBB_GbEb },
  /* 1B /q */ { 0, BX_IA_SBB_GqEq },
  /* 1C /q */ { BxImmediate_Ib, BX_IA_SBB_ALIb },
  /* 1D /q */ { BxImmediate_Id, BX_IA_SBB_RAXId },
  /* 1E /q */ { 0, BX_IA_ERROR },
  /* 1F /q */ { 0, BX_IA_ERROR },
  /* 20 /q */ { BxArithDstRM | BxLockable, BX_IA_AND_EbGb },
  /* 21 /q */ { BxArithDstRM | BxLockable, BX_IA_AND_EqGq },
  /* 22 /q */ { 0, BX_IA_AND_GbEb },
  /* 23 /q */ { 0, BX_IA_AND_GqEq },
  /* 24 /q */ { BxImmediate_Ib, BX_IA_AND_ALIb },
  /* 25 /q */ { BxImmediate_Id, BX_IA_AND_RAXId },
  /* 26 /q */ { 0, BX_IA_ERROR }, // ES:
  /* 27 /q */ { 0, BX_IA_ERROR },
  /* 28 /q */ { BxArithDstRM | BxLockable, BX_IA_SUB_EbGb },
  /* 29 /q */ { BxArithDstRM | BxLockable, BX_IA_SUB_EqGq },
  /* 2A /q */ { 0, BX_IA_SUB_GbEb },
  /* 2B /q */ { 0, BX_IA_SUB_GqEq },
  /* 2C /q */ { BxImmediate_Ib, BX_IA_SUB_ALIb },
  /* 2D /q */ { BxImmediate_Id, BX_IA_SUB_RAXId },
  /* 2E /q */ { 0, BX_IA_ERROR }, // CS:
  /* 2F /q */ { 0, BX_IA_ERROR },
  /* 30 /q */ { BxArithDstRM | BxLockable, BX_IA_XOR_EbGb },
  /* 31 /q */ { BxArithDstRM | BxLockable, BX_IA_XOR_EqGq },
  /* 32 /q */ { 0, BX_IA_XOR_GbEb },
  /* 33 /q */ { 0, BX_IA_XOR_GqEq },
  /* 34 /q */ { BxImmediate_Ib, BX_IA_XOR_ALIb },
  /* 35 /q */ { BxImmediate_Id, BX_IA_XOR_RAXId },
  /* 36 /q */ { 0, BX_IA_ERROR }, // SS:
  /* 37 /q */ { 0, BX_IA_ERROR },
  /* 38 /q */ { BxArithDstRM, BX_IA_CMP_EbGb },
  /* 39 /q */ { BxArithDstRM, BX_IA_CMP_EqGq },
  /* 3A /q */ { 0, BX_IA_CMP_GbEb },
  /* 3B /q */ { 0, BX_IA_CMP_GqEq },
  /* 3C /q */ { BxImmediate_Ib, BX_IA_CMP_ALIb },
  /* 3D /q */ { BxImmediate_Id, BX_IA_CMP_RAXId },
  /* 3E /q */ { 0, BX_IA_ERROR }, // DS:
  /* 3F /q */ { 0, BX_IA_ERROR },
  /* 40 /q */ { 0, BX_IA_ERROR }, // REX:
  /* 41 /q */ { 0, BX_IA_ERROR }, // REX:
  /* 42 /q */ { 0, BX_IA_ERROR }, // REX:
  /* 43 /q */ { 0, BX_IA_ERROR }, // REX:
  /* 44 /q */ { 0, BX_IA_ERROR }, // REX:
  /* 45 /q */ { 0, BX_IA_ERROR }, // REX:
  /* 46 /q */ { 0, BX_IA_ERROR }, // REX:
  /* 47 /q */ { 0, BX_IA_ERROR }, // REX:
  /* 48 /q */ { 0, BX_IA_ERROR }, // REX:
  /* 49 /q */ { 0, BX_IA_ERROR }, // REX:
  /* 4A /q */ { 0, BX_IA_ERROR }, // REX:
  /* 4B /q */ { 0, BX_IA_ERROR }, // REX:
  /* 4C /q */ { 0, BX_IA_ERROR }, // REX:
  /* 4D /q */ { 0, BX_IA_ERROR }, // REX:
  /* 4E /q */ { 0, BX_IA_ERROR }, // REX:
  /* 4F /q */ { 0, BX_IA_ERROR }, // REX:
  /* 50 /q */ { 0, BX_IA_PUSH_RRX },
  /* 51 /q */ { 0, BX_IA_PUSH_RRX },
  /* 52 /q */ { 0, BX_IA_PUSH_RRX },
  /* 53 /q */ { 0, BX_IA_PUSH_RRX },
  /* 54 /q */ { 0, BX_IA_PUSH_RRX },
  /* 55 /q */ { 0, BX_IA_PUSH_RRX },
  /* 56 /q */ { 0, BX_IA_PUSH_RRX },
  /* 57 /q */ { 0, BX_IA_PUSH_RRX },
  /* 58 /q */ { 0, BX_IA_POP_RRX },
  /* 59 /q */ { 0, BX_IA_POP_RRX },
  /* 5A /q */ { 0, BX_IA_POP_RRX },
  /* 5B /q */ { 0, BX_IA_POP_RRX },
  /* 5C /q */ { 0, BX_IA_POP_RRX },
  /* 5D /q */ { 0, BX_IA_POP_RRX },
  /* 5E /q */ { 0, BX_IA_POP_RRX },
  /* 5F /q */ { 0, BX_IA_POP_RRX },
  /* 60 /q */ { 0, BX_IA_ERROR },
  /* 61 /q */ { 0, BX_IA_ERROR },
  /* 62 /q */ { 0, BX_IA_ERROR },
  /* 63 /q */ { 0, BX_IA_MOVSX_GqEd },
  /* 64 /q */ { 0, BX_IA_ERROR }, // FS:
  /* 65 /q */ { 0, BX_IA_ERROR }, // GS:
  /* 66 /q */ { 0, BX_IA_ERROR }, // OS:
  /* 67 /q */ { 0, BX_IA_ERROR }, // AS:
  /* 68 /q */ { BxImmediate_Id, BX_IA_PUSH64_Id },
  /* 69 /q */ { BxImmediate_Id, BX_IA_IMUL_GqEqId },
  /* 6A /q */ { BxImmediate_Ib_SE, BX_IA_PUSH64_Id },
  /* 6B /q */ { BxImmediate_Ib_SE, BX_IA_IMUL_GqEqId },
  /* 6C /q */ { 0, BX_IA_REP_INSB_YbDX },
  /* 6D /q */ { 0, BX_IA_REP_INSD_YdDX },
  /* 6E /q */ { 0, BX_IA_REP_OUTSB_DXXb },
  /* 6F /q */ { 0, BX_IA_REP_OUTSD_DXXd },
  /* 70 /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JO_Jq },
  /* 71 /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNO_Jq },
  /* 72 /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JB_Jq },
  /* 73 /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNB_Jq },
  /* 74 /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JZ_Jq },
  /* 75 /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNZ_Jq },
  /* 76 /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JBE_Jq },
  /* 77 /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNBE_Jq },
  /* 78 /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JS_Jq },
  /* 79 /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNS_Jq },
  /* 7A /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JP_Jq },
  /* 7B /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNP_Jq },
  /* 7C /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JL_Jq },
  /* 7D /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNL_Jq },
  /* 7E /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JLE_Jq },
  /* 7F /q */ { BxImmediate_BrOff8 | BxTraceJCC, BX_IA_JNLE_Jq },
  /* 80 /q */ { BxGroup1 | BxImmediate_Ib, BX_IA_ERROR, BxOpcodeInfoG1EbIb },
  /* 81 /q */ { BxGroup1 | BxImmediate_Id, BX_IA_ERROR, BxOpcodeInfo64G1Eq },
  /* 82 /q */ { 0, BX_IA_ERROR },
  /* 83 /q */ { BxGroup1 | BxImmediate_Ib_SE, BX_IA_ERROR, BxOpcodeInfo64G1Eq },
  /* 84 /q */ { 0, BX_IA_TEST_EbGb },
  /* 85 /q */ { 0, BX_IA_TEST_EqGq },
  /* 86 /q */ { BxLockable, BX_IA_XCHG_EbGb },
  /* 87 /q */ { BxLockable, BX_IA_XCHG_EqGq },
  /* 88 /q */ { BxArithDstRM, BX_IA_MOV_EbGb },
  /* 89 /q */ { BxArithDstRM, BX_IA_MOV_EqGq },
  /* 8A /q */ { 0, BX_IA_MOV_GbEb },
  /* 8B /q */ { 0, BX_IA_MOV_GqEq },
  /* 8C /q */ { 0, BX_IA_MOV_EwSw },
  /* 8D /q */ { 0, BX_IA_LEA_GqM },
  /* 8E /q */ { 0, BX_IA_MOV_SwEw },
  /* 8F /q */ { BxGroup1A, BX_IA_ERROR, BxOpcodeInfo64G1AEq },
  /* 90 /q */ { 0, BX_IA_XCHG_RRXRAX }, // handles XCHG R8, RAX
  /* 91 /q */ { 0, BX_IA_XCHG_RRXRAX },
  /* 92 /q */ { 0, BX_IA_XCHG_RRXRAX },
  /* 93 /q */ { 0, BX_IA_XCHG_RRXRAX },
  /* 94 /q */ { 0, BX_IA_XCHG_RRXRAX },
  /* 95 /q */ { 0, BX_IA_XCHG_RRXRAX },
  /* 96 /q */ { 0, BX_IA_XCHG_RRXRAX },
  /* 97 /q */ { 0, BX_IA_XCHG_RRXRAX },
  /* 98 /q */ { 0, BX_IA_CDQE },
  /* 99 /q */ { 0, BX_IA_CQO },
  /* 9A /q */ { 0, BX_IA_ERROR },
  /* 9B /q */ { 0, BX_IA_FWAIT },
  /* 9C /q */ { 0, BX_IA_PUSHF_Fq },
  /* 9D /q */ { 0, BX_IA_POPF_Fq },
  /* 9E /q */ { 0, BX_IA_SAHF },
  /* 9F /q */ { 0, BX_IA_LAHF },
  /* A0 /q */ { BxImmediate_O, BX_IA_MOV_ALOq },
  /* A1 /q */ { BxImmediate_O, BX_IA_MOV_RAXOq },
  /* A2 /q */ { BxImmediate_O, BX_IA_MOV_OqAL },
  /* A3 /q */ { BxImmediate_O, BX_IA_MOV_OqRAX },
  /* A4 /q */ { 0, BX_IA_REP_MOVSB_XbYb },
  /* A5 /q */ { 0, BX_IA_REP_MOVSQ_XqYq },
  /* A6 /q */ { 0, BX_IA_REP_CMPSB_XbYb },
  /* A7 /q */ { 0, BX_IA_REP_CMPSQ_XqYq },
  /* A8 /q */ { BxImmediate_Ib, BX_IA_TEST_ALIb },
  /* A9 /q */ { BxImmediate_Id, BX_IA_TEST_RAXId },
  /* AA /q */ { 0, BX_IA_REP_STOSB_YbAL },
  /* AB /q */ { 0, BX_IA_REP_STOSQ_YqRAX },
  /* AC /q */ { 0, BX_IA_REP_LODSB_ALXb },
  /* AD /q */ { 0, BX_IA_REP_LODSQ_RAXXq },
  /* AE /q */ { 0, BX_IA_REP_SCASB_ALXb  },
  /* AF /q */ { 0, BX_IA_REP_SCASQ_RAXXq },
  /* B0 /q */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B1 /q */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B2 /q */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B3 /q */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B4 /q */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B5 /q */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B6 /q */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B7 /q */ { BxImmediate_Ib, BX_IA_MOV_RLIb },
  /* B8 /q */ { BxImmediate_Iq, BX_IA_MOV_RRXIq },
  /* B9 /q */ { BxImmediate_Iq, BX_IA_MOV_RRXIq },
  /* BA /q */ { BxImmediate_Iq, BX_IA_MOV_RRXIq },
  /* BB /q */ { BxImmediate_Iq, BX_IA_MOV_RRXIq },
  /* BC /q */ { BxImmediate_Iq, BX_IA_MOV_RRXIq },
  /* BD /q */ { BxImmediate_Iq, BX_IA_MOV_RRXIq },
  /* BE /q */ { BxImmediate_Iq, BX_IA_MOV_RRXIq },
  /* BF /q */ { BxImmediate_Iq, BX_IA_MOV_RRXIq },
  /* C0 /q */ { BxGroup2 | BxImmediate_Ib, BX_IA_ERROR, BxOpcodeInfoG2Eb },
  /* C1 /q */ { BxGroup2 | BxImmediate_Ib, BX_IA_ERROR, BxOpcodeInfo64G2Eq },
  /* C2 /q */ { BxImmediate_Iw | BxTraceEnd, BX_IA_RETnear64_Iw },
  /* C3 /q */ { BxTraceEnd,                  BX_IA_RETnear64 },
  /* C4 /q */ { 0, BX_IA_ERROR },
  /* C5 /q */ { 0, BX_IA_ERROR },
  /* C6 /q */ { BxGroup11, BX_IA_ERROR, BxOpcodeInfoG11Eb },
  /* C7 /q */ { BxGroup11, BX_IA_ERROR, BxOpcodeInfo64G11Eq },
  /* C8 /q */ { BxImmediate_Iw | BxImmediate_Ib2, BX_IA_ENTER64_IwIb },
  /* C9 /q */ { 0, BX_IA_LEAVE64 },
  /* CA /q */ { BxImmediate_Iw | BxTraceEnd, BX_IA_RETfar64_Iw },
  /* CB /q */ { BxTraceEnd,                  BX_IA_RETfar64 },
  /* CC /q */ { BxTraceEnd, BX_IA_INT3 },
  /* CD /q */ { BxImmediate_Ib | BxTraceEnd, BX_IA_INT_Ib },
  /* CE /q */ { 0, BX_IA_ERROR },
  /* CF /q */ { BxTraceEnd, BX_IA_IRET64 },
  /* D0 /q */ { BxGroup2 | BxImmediate_I1, BX_IA_ERROR, BxOpcodeInfoG2Eb },
  /* D1 /q */ { BxGroup2 | BxImmediate_I1, BX_IA_ERROR, BxOpcodeInfo64G2Eq },
  /* D2 /q */ { BxGroup2, BX_IA_ERROR, BxOpcodeInfoG2Eb },
  /* D3 /q */ { BxGroup2, BX_IA_ERROR, BxOpcodeInfo64G2Eq },
  /* D4 /q */ { 0, BX_IA_ERROR },
  /* D5 /q */ { 0, BX_IA_ERROR },
  /* D6 /q */ { 0, BX_IA_ERROR },
  /* D7 /q */ { 0, BX_IA_XLAT },
  /* D8 /q */ { BxGroupFP, BX_IA_ERROR, BxOpcodeInfo_FPGroupD8 },
  /* D9 /q */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointD9 },
  /* DA /q */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDA },
  /* DB /q */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDB },
  /* DC /q */ { BxGroupFP, BX_IA_ERROR, BxOpcodeInfo_FPGroupDC },
  /* DD /q */ { BxGroupFP, BX_IA_ERROR, BxOpcodeInfo_FPGroupDD },
  /* DE /q */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDE },
  /* DF /q */ { BxFPEscape, BX_IA_ERROR, BxOpcodeInfo_FloatingPointDF },
  /* E0 /q */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_LOOPNE64_Jb },
  /* E1 /q */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_LOOPE64_Jb },
  /* E2 /q */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_LOOP64_Jb },
  /* E3 /q */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_JRCXZ_Jb },
  /* E4 /q */ { BxImmediate_Ib, BX_IA_IN_ALIb },
  /* E5 /q */ { BxImmediate_Ib, BX_IA_IN_EAXIb },
  /* E6 /q */ { BxImmediate_Ib, BX_IA_OUT_IbAL },
  /* E7 /q */ { BxImmediate_Ib, BX_IA_OUT_IbEAX },
  /* E8 /q */ { BxImmediate_BrOff32 | BxTraceEnd, BX_IA_CALL_Jq },
  /* E9 /q */ { BxImmediate_BrOff32 | BxTraceEnd, BX_IA_JMP_Jq },
  /* EA /q */ { 0, BX_IA_ERROR },
  /* EB /q */ { BxImmediate_BrOff8 | BxTraceEnd, BX_IA_JMP_Jq },
  /* EC /q */ { 0, BX_IA_IN_ALDX },
  /* ED /q */ { 0, BX_IA_IN_EAXDX },
  /* EE /q */ { 0, BX_IA_OUT_DXAL },
  /* EF /q */ { 0, BX_IA_OUT_DXEAX },
  /* F0 /q */ { 0, BX_IA_ERROR }, // LOCK:
  /* F1 /q */ { BxTraceEnd, BX_IA_INT1 },
  /* F2 /q */ { 0, BX_IA_ERROR }, // REPNE/REPNZ
  /* F3 /q */ { 0, BX_IA_ERROR }, // REP,REPE/REPZ
  /* F4 /q */ { BxTraceEnd, BX_IA_HLT },
  /* F5 /q */ { 0, BX_IA_CMC },
  /* F6 /q */ { BxGroup3, BX_IA_ERROR, BxOpcodeInfoG3Eb },
  /* F7 /q */ { BxGroup3, BX_IA_ERROR, BxOpcodeInfo64G3Eq },
  /* F8 /q */ { 0, BX_IA_CLC },
  /* F9 /q */ { 0, BX_IA_STC },
  /* FA /q */ { 0, BX_IA_CLI },
  /* FB /q */ { 0, BX_IA_STI },
  /* FC /q */ { 0, BX_IA_CLD },
  /* FD /q */ { 0, BX_IA_STD },
  /* FE /q */ { BxGroup4, BX_IA_ERROR, BxOpcodeInfoG4 },
  /* FF /q */ { BxGroup5, BX_IA_ERROR, BxOpcodeInfo64G5q },

  /* 0F 00 /q */ { BxGroup6, BX_IA_ERROR, BxOpcodeInfoG6 },
  /* 0F 01 /q */ { BxGroup7, BX_IA_ERROR, BxOpcodeInfoG7q },
  /* 0F 02 /q */ { 0, BX_IA_LAR_GvEw },
  /* 0F 03 /q */ { 0, BX_IA_LSL_GvEw },
  /* 0F 04 /q */ { 0, BX_IA_ERROR },
  /* 0F 05 /q */ { BxTraceEnd, BX_IA_SYSCALL },
  /* 0F 06 /q */ { 0, BX_IA_CLTS },
  /* 0F 07 /q */ { BxTraceEnd, BX_IA_SYSRET },
  /* 0F 08 /q */ { BxTraceEnd, BX_IA_INVD },
  /* 0F 09 /q */ { BxTraceEnd, BX_IA_WBINVD },
  /* 0F 0A /q */ { 0, BX_IA_ERROR },
  /* 0F 0B /q */ { BxTraceEnd, BX_IA_UD2A },
  /* 0F 0C /q */ { 0, BX_IA_ERROR },
  /* 0F 0D /q */ { 0, BX_IA_PREFETCHW },          // 3DNow! PREFETCHW on AMD, NOP on Intel
  /* 0F 0E /q */ { 0, BX_IA_FEMMS },              // 3DNow! FEMMS
  /* 0F 0F /q */ { BxImmediate_Ib, BX_IA_ERROR }, // 3DNow! Opcode Table
  /* 0F 10 /q */ { BxPrefixSSE,                BX_IA_MOVUPS_VpsWps, BxOpcodeGroupSSE_0f10 },
  /* 0F 11 /q */ { BxPrefixSSE | BxArithDstRM, BX_IA_MOVUPS_WpsVps, BxOpcodeGroupSSE_0f11 },
  /* 0F 12 /q */ { BxPrefixSSE, BX_IA_MOVLPS_VpsMq, BxOpcodeGroupSSE_0f12 },
  /* 0F 13 /q */ { BxPrefixSSE, BX_IA_MOVLPS_MqVps, BxOpcodeGroupSSE_0f13M },
  /* 0F 14 /q */ { BxPrefixSSE, BX_IA_UNPCKLPS_VpsWdq, BxOpcodeGroupSSE_0f14 },
  /* 0F 15 /q */ { BxPrefixSSE, BX_IA_UNPCKHPS_VpsWdq, BxOpcodeGroupSSE_0f15 },
  /* 0F 16 /q */ { BxPrefixSSE, BX_IA_MOVHPS_VpsMq, BxOpcodeGroupSSE_0f16 },
  /* 0F 17 /q */ { BxPrefixSSE, BX_IA_MOVHPS_MqVps, BxOpcodeGroupSSE_0f17M },
  /* 0F 18 /q */ { 0, BX_IA_PREFETCH }, // opcode group G16, PREFETCH hints
  /* 0F 19 /q */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1A /q */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1B /q */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1C /q */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1D /q */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1E /q */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 1F /q */ { 0, BX_IA_NOP },      // multi-byte NOP
  /* 0F 20 /q */ { BxGroupN, BX_IA_ERROR, BxOpcodeInfoMOV_RqCq },
  /* 0F 21 /q */ { 0, BX_IA_MOV_RqDq },
  /* 0F 22 /q */ { BxGroupN, BX_IA_ERROR, BxOpcodeInfoMOV_CqRq },
  /* 0F 23 /q */ { BxTraceEnd, BX_IA_MOV_DqRq },
  /* 0F 24 /q */ { 0, BX_IA_ERROR },
  /* 0F 25 /q */ { 0, BX_IA_ERROR },
  /* 0F 26 /q */ { 0, BX_IA_ERROR },
  /* 0F 27 /q */ { 0, BX_IA_ERROR },
  /* 0F 28 /q */ { BxPrefixSSE,                BX_IA_MOVAPS_VpsWps, BxOpcodeGroupSSE_0f28 },
  /* 0F 29 /q */ { BxPrefixSSE | BxArithDstRM, BX_IA_MOVAPS_WpsVps, BxOpcodeGroupSSE_0f29 },
  /* 0F 2A /q */ { BxPrefixSSE, BX_IA_CVTPI2PS_VpsQq, BxOpcodeGroupSSE_0f2aQ },
  /* 0F 2B /q */ { BxPrefixSSE, BX_IA_MOVNTPS_MpsVps, BxOpcodeGroupSSE_0f2bM },
  /* 0F 2C /q */ { BxPrefixSSE, BX_IA_CVTTPS2PI_PqWps, BxOpcodeGroupSSE_0f2cQ },
  /* 0F 2D /q */ { BxPrefixSSE, BX_IA_CVTPS2PI_PqWps, BxOpcodeGroupSSE_0f2dQ },
  /* 0F 2E /q */ { BxPrefixSSE, BX_IA_UCOMISS_VssWss, BxOpcodeGroupSSE_0f2e },
  /* 0F 2F /q */ { BxPrefixSSE, BX_IA_COMISS_VpsWps, BxOpcodeGroupSSE_0f2f },
  /* 0F 30 /q */ { 0, BX_IA_WRMSR },
  /* 0F 31 /q */ { 0, BX_IA_RDTSC },
  /* 0F 32 /q */ { 0, BX_IA_RDMSR },
  /* 0F 33 /q */ { 0, BX_IA_RDPMC },
  /* 0F 34 /q */ { BxTraceEnd, BX_IA_SYSENTER },
  /* 0F 35 /q */ { BxTraceEnd, BX_IA_SYSEXIT },
  /* 0F 36 /q */ { 0, BX_IA_ERROR },
  /* 0F 37 /q */ { 0, BX_IA_ERROR },
  /* 0F 38 /q */ { Bx3ByteOp, BX_IA_ERROR, BxOpcode3ByteTable0f38 }, // 3-byte escape
  /* 0F 39 /q */ { 0, BX_IA_ERROR },
  /* 0F 3A /q */ { Bx3ByteOp | BxImmediate_Ib, BX_IA_ERROR, BxOpcode3ByteTable0f3a }, // 3-byte escape
  /* 0F 3B /q */ { 0, BX_IA_ERROR },
  /* 0F 3C /q */ { 0, BX_IA_ERROR },
  /* 0F 3D /q */ { 0, BX_IA_ERROR },
  /* 0F 3E /q */ { 0, BX_IA_ERROR },
  /* 0F 3F /q */ { 0, BX_IA_ERROR },
  /* 0F 40 /q */ { 0, BX_IA_CMOVO_GqEq },
  /* 0F 41 /q */ { 0, BX_IA_CMOVNO_GqEq },
  /* 0F 42 /q */ { 0, BX_IA_CMOVB_GqEq },
  /* 0F 43 /q */ { 0, BX_IA_CMOVNB_GqEq },
  /* 0F 44 /q */ { 0, BX_IA_CMOVZ_GqEq },
  /* 0F 45 /q */ { 0, BX_IA_CMOVNZ_GqEq },
  /* 0F 46 /q */ { 0, BX_IA_CMOVBE_GqEq },
  /* 0F 47 /q */ { 0, BX_IA_CMOVNBE_GqEq },
  /* 0F 48 /q */ { 0, BX_IA_CMOVS_GqEq },
  /* 0F 49 /q */ { 0, BX_IA_CMOVNS_GqEq },
  /* 0F 4A /q */ { 0, BX_IA_CMOVP_GqEq },
  /* 0F 4B /q */ { 0, BX_IA_CMOVNP_GqEq },
  /* 0F 4C /q */ { 0, BX_IA_CMOVL_GqEq },
  /* 0F 4D /q */ { 0, BX_IA_CMOVNL_GqEq },
  /* 0F 4E /q */ { 0, BX_IA_CMOVLE_GqEq },
  /* 0F 4F /q */ { 0, BX_IA_CMOVNLE_GqEq },
  /* 0F 50 /q */ { BxPrefixSSE, BX_IA_MOVMSKPS_GdVRps, BxOpcodeGroupSSE_0f50R },
  /* 0F 51 /q */ { BxPrefixSSE, BX_IA_SQRTPS_VpsWps, BxOpcodeGroupSSE_0f51 },
  /* 0F 52 /q */ { BxPrefixSSE, BX_IA_RSQRTPS_VpsWps, BxOpcodeGroupSSE_0f52 },
  /* 0F 53 /q */ { BxPrefixSSE, BX_IA_RCPPS_VpsWps, BxOpcodeGroupSSE_0f53 },
  /* 0F 54 /q */ { BxPrefixSSE, BX_IA_ANDPS_VpsWps, BxOpcodeGroupSSE_0f54 },
  /* 0F 55 /q */ { BxPrefixSSE, BX_IA_ANDNPS_VpsWps, BxOpcodeGroupSSE_0f55 },
  /* 0F 56 /q */ { BxPrefixSSE, BX_IA_ORPS_VpsWps, BxOpcodeGroupSSE_0f56 },
  /* 0F 57 /q */ { BxPrefixSSE, BX_IA_XORPS_VpsWps, BxOpcodeGroupSSE_0f57 },
  /* 0F 58 /q */ { BxPrefixSSE, BX_IA_ADDPS_VpsWps, BxOpcodeGroupSSE_0f58 },
  /* 0F 59 /q */ { BxPrefixSSE, BX_IA_MULPS_VpsWps, BxOpcodeGroupSSE_0f59 },
  /* 0F 5A /q */ { BxPrefixSSE, BX_IA_CVTPS2PD_VpsWps, BxOpcodeGroupSSE_0f5a },
  /* 0F 5B /q */ { BxPrefixSSE, BX_IA_CVTDQ2PS_VpsWdq, BxOpcodeGroupSSE_0f5b },
  /* 0F 5C /q */ { BxPrefixSSE, BX_IA_SUBPS_VpsWps, BxOpcodeGroupSSE_0f5c },
  /* 0F 5D /q */ { BxPrefixSSE, BX_IA_MINPS_VpsWps, BxOpcodeGroupSSE_0f5d },
  /* 0F 5E /q */ { BxPrefixSSE, BX_IA_DIVPS_VpsWps, BxOpcodeGroupSSE_0f5e },
  /* 0F 5F /q */ { BxPrefixSSE, BX_IA_MAXPS_VpsWps, BxOpcodeGroupSSE_0f5f },
  /* 0F 60 /q */ { BxPrefixSSE, BX_IA_PUNPCKLBW_PqQd, BxOpcodeGroupSSE_0f60 },
  /* 0F 61 /q */ { BxPrefixSSE, BX_IA_PUNPCKLWD_PqQd, BxOpcodeGroupSSE_0f61 },
  /* 0F 62 /q */ { BxPrefixSSE, BX_IA_PUNPCKLDQ_PqQd, BxOpcodeGroupSSE_0f62 },
  /* 0F 63 /q */ { BxPrefixSSE, BX_IA_PACKSSWB_PqQq, BxOpcodeGroupSSE_0f63 },
  /* 0F 64 /q */ { BxPrefixSSE, BX_IA_PCMPGTB_PqQq, BxOpcodeGroupSSE_0f64 },
  /* 0F 65 /q */ { BxPrefixSSE, BX_IA_PCMPGTW_PqQq, BxOpcodeGroupSSE_0f65 },
  /* 0F 66 /q */ { BxPrefixSSE, BX_IA_PCMPGTD_PqQq, BxOpcodeGroupSSE_0f66 },
  /* 0F 67 /q */ { BxPrefixSSE, BX_IA_PACKUSWB_PqQq, BxOpcodeGroupSSE_0f67 },
  /* 0F 68 /q */ { BxPrefixSSE, BX_IA_PUNPCKHBW_PqQq, BxOpcodeGroupSSE_0f68 },
  /* 0F 69 /q */ { BxPrefixSSE, BX_IA_PUNPCKHWD_PqQq, BxOpcodeGroupSSE_0f69 },
  /* 0F 6A /q */ { BxPrefixSSE, BX_IA_PUNPCKHDQ_PqQq, BxOpcodeGroupSSE_0f6a },
  /* 0F 6B /q */ { BxPrefixSSE, BX_IA_PACKSSDW_PqQq, BxOpcodeGroupSSE_0f6b },
  /* 0F 6C /q */ { BxPrefixSSE66, BX_IA_PUNPCKLQDQ_VdqWdq },
  /* 0F 6D /q */ { BxPrefixSSE66, BX_IA_PUNPCKHQDQ_VdqWdq },
  /* 0F 6E /q */ { BxPrefixSSE, BX_IA_MOVQ_PqEq, BxOpcodeGroupSSE_0f6eQ },
  /* 0F 6F /q */ { BxPrefixSSE, BX_IA_MOVQ_PqQq, BxOpcodeGroupSSE_0f6f },
  /* 0F 70 /q */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_PSHUFW_PqQqIb, BxOpcodeGroupSSE_0f70 },
  /* 0F 71 /q */ { BxGroup12, BX_IA_ERROR, BxOpcodeInfoG12R },
  /* 0F 72 /q */ { BxGroup13, BX_IA_ERROR, BxOpcodeInfoG13R },
  /* 0F 73 /q */ { BxGroup14, BX_IA_ERROR, BxOpcodeInfoG14R },
  /* 0F 74 /q */ { BxPrefixSSE, BX_IA_PCMPEQB_PqQq, BxOpcodeGroupSSE_0f74 },
  /* 0F 75 /q */ { BxPrefixSSE, BX_IA_PCMPEQW_PqQq, BxOpcodeGroupSSE_0f75 },
  /* 0F 76 /q */ { BxPrefixSSE, BX_IA_PCMPEQD_PqQq, BxOpcodeGroupSSE_0f76 },
  /* 0F 77 /q */ { BxPrefixSSE, BX_IA_EMMS, BxOpcodeGroupSSE_ERR },
  /* 0F 78 /q */ { BxPrefixSSE, BX_IA_VMREAD_EqGq, BxOpcodeGroupSSE_ERR },
  /* 0F 79 /q */ { BxPrefixSSE, BX_IA_VMWRITE_GqEq, BxOpcodeGroupSSE_ERR },
  /* 0F 7A /q */ { 0, BX_IA_ERROR },
  /* 0F 7B /q */ { 0, BX_IA_ERROR },
  /* 0F 7C /q */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0f7c },
  /* 0F 7D /q */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0f7d },
  /* 0F 7E /q */ { BxPrefixSSE, BX_IA_MOVQ_EqPq, BxOpcodeGroupSSE_0f7eQ },
  /* 0F 7F /q */ { BxPrefixSSE, BX_IA_MOVQ_QqPq, BxOpcodeGroupSSE_0f7f },
  /* 0F 80 /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JO_Jq },
  /* 0F 81 /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNO_Jq },
  /* 0F 82 /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JB_Jq },
  /* 0F 83 /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNB_Jq },
  /* 0F 84 /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JZ_Jq },
  /* 0F 85 /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNZ_Jq },
  /* 0F 86 /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JBE_Jq },
  /* 0F 87 /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNBE_Jq },
  /* 0F 88 /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JS_Jq },
  /* 0F 89 /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNS_Jq },
  /* 0F 8A /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JP_Jq },
  /* 0F 8B /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNP_Jq },
  /* 0F 8C /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JL_Jq },
  /* 0F 8D /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNL_Jq },
  /* 0F 8E /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JLE_Jq },
  /* 0F 8F /q */ { BxImmediate_BrOff32 | BxTraceJCC, BX_IA_JNLE_Jq },
  /* 0F 90 /q */ { 0, BX_IA_SETO_Eb },
  /* 0F 91 /q */ { 0, BX_IA_SETNO_Eb },
  /* 0F 92 /q */ { 0, BX_IA_SETB_Eb },
  /* 0F 93 /q */ { 0, BX_IA_SETNB_Eb },
  /* 0F 94 /q */ { 0, BX_IA_SETZ_Eb },
  /* 0F 95 /q */ { 0, BX_IA_SETNZ_Eb },
  /* 0F 96 /q */ { 0, BX_IA_SETBE_Eb },
  /* 0F 97 /q */ { 0, BX_IA_SETNBE_Eb },
  /* 0F 98 /q */ { 0, BX_IA_SETS_Eb },
  /* 0F 99 /q */ { 0, BX_IA_SETNS_Eb },
  /* 0F 9A /q */ { 0, BX_IA_SETP_Eb },
  /* 0F 9B /q */ { 0, BX_IA_SETNP_Eb },
  /* 0F 9C /q */ { 0, BX_IA_SETL_Eb },
  /* 0F 9D /q */ { 0, BX_IA_SETNL_Eb },
  /* 0F 9E /q */ { 0, BX_IA_SETLE_Eb },
  /* 0F 9F /q */ { 0, BX_IA_SETNLE_Eb },
  /* 0F A0 /q */ { 0, BX_IA_PUSH64_FS },
  /* 0F A1 /q */ { 0, BX_IA_POP64_FS },
  /* 0F A2 /q */ { 0, BX_IA_CPUID },
  /* 0F A3 /q */ { 0, BX_IA_BT_EqGq },
  /* 0F A4 /q */ { BxImmediate_Ib, BX_IA_SHLD_EqGq },
  /* 0F A5 /q */ { 0,              BX_IA_SHLD_EqGq },
  /* 0F A6 /q */ { 0, BX_IA_ERROR },
  /* 0F A7 /q */ { 0, BX_IA_ERROR },
  /* 0F A8 /q */ { 0, BX_IA_PUSH64_GS },
  /* 0F A9 /q */ { 0, BX_IA_POP64_GS },
  /* 0F AA /q */ { BxTraceEnd, BX_IA_RSM },
  /* 0F AB /q */ { BxLockable, BX_IA_BTS_EqGq },
  /* 0F AC /q */ { BxImmediate_Ib, BX_IA_SHRD_EqGq },
  /* 0F AD /q */ { 0,              BX_IA_SHRD_EqGq },
  /* 0F AE /q */ { BxGroup15, BX_IA_ERROR, BxOpcodeInfoG15q },
  /* 0F AF /q */ { 0, BX_IA_IMUL_GqEq },
  /* 0F B0 /q */ { BxLockable, BX_IA_CMPXCHG_EbGb },
  /* 0F B1 /q */ { BxLockable, BX_IA_CMPXCHG_EqGq },
  /* 0F B2 /q */ { 0, BX_IA_LSS_GqMp }, // TODO: LSS_GdMp for AMD CPU
  /* 0F B3 /q */ { BxLockable, BX_IA_BTR_EqGq },
  /* 0F B4 /q */ { 0, BX_IA_LFS_GqMp }, // TODO: LFS_GdMp for AMD CPU
  /* 0F B5 /q */ { 0, BX_IA_LGS_GqMp }, // TODO: LGS_GdMp for AMD CPU
  /* 0F B6 /q */ { 0, BX_IA_MOVZX_GqEb },
  /* 0F B7 /q */ { 0, BX_IA_MOVZX_GqEw },
  /* 0F B8 /q */ { BxPrefixSSEF3, BX_IA_POPCNT_GqEq },
  /* 0F B9 /q */ { BxTraceEnd, BX_IA_UD2B },
  /* 0F BA /q */ { BxGroup8, BX_IA_ERROR, BxOpcodeInfo64G8EqIb },
  /* 0F BB /q */ { BxLockable, BX_IA_BTC_EqGq },
  /* 0F BC /q */ { 0, BX_IA_BSF_GqEq },
  /* 0F BD /q */ { 0, BX_IA_BSR_GqEq },
  /* 0F BE /q */ { 0, BX_IA_MOVSX_GqEb },
  /* 0F BF /q */ { 0, BX_IA_MOVSX_GqEw },
  /* 0F C0 /q */ { BxLockable, BX_IA_XADD_EbGb },
  /* 0F C1 /q */ { BxLockable, BX_IA_XADD_EqGq },
  /* 0F C2 /q */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_CMPPS_VpsWpsIb, BxOpcodeGroupSSE_0fc2 },
  /* 0F C3 /q */ { BxPrefixSSE, BX_IA_MOVNTI_MqGq, BxOpcodeGroupSSE_ERR },
  /* 0F C4 /q */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_PINSRW_PqEwIb, BxOpcodeGroupSSE_0fc4 },
  /* 0F C5 /q */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_PEXTRW_GdPqIb, BxOpcodeGroupSSE_0fc5R },
  /* 0F C6 /q */ { BxPrefixSSE | BxImmediate_Ib, BX_IA_SHUFPS_VpsWpsIb, BxOpcodeGroupSSE_0fc6 },
  /* 0F C7 /q */ { BxGroup9, BX_IA_ERROR, BxOpcodeInfo64G9qM },
  /* 0F C8 /q */ { 0, BX_IA_BSWAP_RRX },
  /* 0F C9 /q */ { 0, BX_IA_BSWAP_RRX },
  /* 0F CA /q */ { 0, BX_IA_BSWAP_RRX },
  /* 0F CB /q */ { 0, BX_IA_BSWAP_RRX },
  /* 0F CC /q */ { 0, BX_IA_BSWAP_RRX },
  /* 0F CD /q */ { 0, BX_IA_BSWAP_RRX },
  /* 0F CE /q */ { 0, BX_IA_BSWAP_RRX },
  /* 0F CF /q */ { 0, BX_IA_BSWAP_RRX },
  /* 0F D0 /q */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0fd0 },
  /* 0F D1 /q */ { BxPrefixSSE, BX_IA_PSRLW_PqQq, BxOpcodeGroupSSE_0fd1 },
  /* 0F D2 /q */ { BxPrefixSSE, BX_IA_PSRLD_PqQq, BxOpcodeGroupSSE_0fd2 },
  /* 0F D3 /q */ { BxPrefixSSE, BX_IA_PSRLQ_PqQq, BxOpcodeGroupSSE_0fd3 },
  /* 0F D4 /q */ { BxPrefixSSE, BX_IA_PADDQ_PqQq, BxOpcodeGroupSSE_0fd4 },
  /* 0F D5 /q */ { BxPrefixSSE, BX_IA_PMULLW_PqQq, BxOpcodeGroupSSE_0fd5 },
  /* 0F D6 /q */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0fd6 },
  /* 0F D7 /q */ { BxPrefixSSE, BX_IA_PMOVMSKB_GdPRq, BxOpcodeGroupSSE_0fd7R },
  /* 0F D8 /q */ { BxPrefixSSE, BX_IA_PSUBUSB_PqQq, BxOpcodeGroupSSE_0fd8 },
  /* 0F D9 /q */ { BxPrefixSSE, BX_IA_PSUBUSW_PqQq, BxOpcodeGroupSSE_0fd9 },
  /* 0F DA /q */ { BxPrefixSSE, BX_IA_PMINUB_PqQq, BxOpcodeGroupSSE_0fda },
  /* 0F DB /q */ { BxPrefixSSE, BX_IA_PAND_PqQq, BxOpcodeGroupSSE_0fdb },
  /* 0F DC /q */ { BxPrefixSSE, BX_IA_PADDUSB_PqQq, BxOpcodeGroupSSE_0fdc },
  /* 0F DD /q */ { BxPrefixSSE, BX_IA_PADDUSW_PqQq, BxOpcodeGroupSSE_0fdd },
  /* 0F DE /q */ { BxPrefixSSE, BX_IA_PMAXUB_PqQq, BxOpcodeGroupSSE_0fde },
  /* 0F DF /q */ { BxPrefixSSE, BX_IA_PANDN_PqQq, BxOpcodeGroupSSE_0fdf },
  /* 0F E0 /q */ { BxPrefixSSE, BX_IA_PAVGB_PqQq, BxOpcodeGroupSSE_0fe0 },
  /* 0F E1 /q */ { BxPrefixSSE, BX_IA_PSRAW_PqQq, BxOpcodeGroupSSE_0fe1 },
  /* 0F E2 /q */ { BxPrefixSSE, BX_IA_PSRAD_PqQq, BxOpcodeGroupSSE_0fe2 },
  /* 0F E3 /q */ { BxPrefixSSE, BX_IA_PAVGW_PqQq, BxOpcodeGroupSSE_0fe3 },
  /* 0F E4 /q */ { BxPrefixSSE, BX_IA_PMULHUW_PqQq, BxOpcodeGroupSSE_0fe4 },
  /* 0F E5 /q */ { BxPrefixSSE, BX_IA_PMULHW_PqQq, BxOpcodeGroupSSE_0fe5 },
  /* 0F E6 /q */ { BxPrefixSSE, BX_IA_ERROR, BxOpcodeGroupSSE_0fe6 },
  /* 0F E7 /q */ { BxPrefixSSE, BX_IA_MOVNTQ_MqPq, BxOpcodeGroupSSE_0fe7M },
  /* 0F E8 /q */ { BxPrefixSSE, BX_IA_PSUBSB_PqQq, BxOpcodeGroupSSE_0fe8 },
  /* 0F E9 /q */ { BxPrefixSSE, BX_IA_PSUBSW_PqQq, BxOpcodeGroupSSE_0fe9 },
  /* 0F EA /q */ { BxPrefixSSE, BX_IA_PMINSW_PqQq, BxOpcodeGroupSSE_0fea },
  /* 0F EB /q */ { BxPrefixSSE, BX_IA_POR_PqQq, BxOpcodeGroupSSE_0feb },
  /* 0F EC /q */ { BxPrefixSSE, BX_IA_PADDSB_PqQq, BxOpcodeGroupSSE_0fec },
  /* 0F ED /q */ { BxPrefixSSE, BX_IA_PADDSW_PqQq, BxOpcodeGroupSSE_0fed },
  /* 0F EE /q */ { BxPrefixSSE, BX_IA_PMAXSW_PqQq, BxOpcodeGroupSSE_0fee },
  /* 0F EF /q */ { BxPrefixSSE, BX_IA_PXOR_PqQq, BxOpcodeGroupSSE_0fef },
  /* 0F F0 /q */ { BxPrefixSSEF2, BX_IA_LDDQU_VdqMdq },
  /* 0F F1 /q */ { BxPrefixSSE, BX_IA_PSLLW_PqQq, BxOpcodeGroupSSE_0ff1 },
  /* 0F F2 /q */ { BxPrefixSSE, BX_IA_PSLLD_PqQq, BxOpcodeGroupSSE_0ff2 },
  /* 0F F3 /q */ { BxPrefixSSE, BX_IA_PSLLQ_PqQq, BxOpcodeGroupSSE_0ff3 },
  /* 0F F4 /q */ { BxPrefixSSE, BX_IA_PMULUDQ_PqQq, BxOpcodeGroupSSE_0ff4 },
  /* 0F F5 /q */ { BxPrefixSSE, BX_IA_PMADDWD_PqQq, BxOpcodeGroupSSE_0ff5 },
  /* 0F F6 /q */ { BxPrefixSSE, BX_IA_PSADBW_PqQq, BxOpcodeGroupSSE_0ff6 },
  /* 0F F7 /q */ { BxPrefixSSE, BX_IA_MASKMOVQ_PqPRq, BxOpcodeGroupSSE_0ff7R },
  /* 0F F8 /q */ { BxPrefixSSE, BX_IA_PSUBB_PqQq, BxOpcodeGroupSSE_0ff8 },
  /* 0F F9 /q */ { BxPrefixSSE, BX_IA_PSUBW_PqQq, BxOpcodeGroupSSE_0ff9 },
  /* 0F FA /q */ { BxPrefixSSE, BX_IA_PSUBD_PqQq, BxOpcodeGroupSSE_0ffa },
  /* 0F FB /q */ { BxPrefixSSE, BX_IA_PSUBQ_PqQq, BxOpcodeGroupSSE_0ffb },
  /* 0F FC /q */ { BxPrefixSSE, BX_IA_PADDB_PqQq, BxOpcodeGroupSSE_0ffc },
  /* 0F FD /q */ { BxPrefixSSE, BX_IA_PADDW_PqQq, BxOpcodeGroupSSE_0ffd },
  /* 0F FE /q */ { BxPrefixSSE, BX_IA_PADDD_PqQq, BxOpcodeGroupSSE_0ffe },
  /* 0F FF /q */ { 0, BX_IA_ERROR }
};

  int BX_CPP_AttrRegparmN(3)
BX_CPU_C::fetchDecode64(const Bit8u *iptr, bxInstruction_c *i, unsigned remainingInPage)
{
  if (remainingInPage > 15) remainingInPage = 15;

  unsigned remain = remainingInPage; // remain must be at least 1
  unsigned b1, b2 = 0, lock=0, ia_opcode = 0;
  unsigned offset = 512, rex_r = 0, rex_x = 0, rex_b = 0;
  unsigned rm = 0, mod = 0, nnn = 0, mod_mem = 0;
  unsigned seg = BX_SEG_REG_DS, seg_override = BX_SEG_REG_NULL;

#define SSE_PREFIX_NONE 0
#define SSE_PREFIX_66   1
#define SSE_PREFIX_F3   2
#define SSE_PREFIX_F2   3
  unsigned sse_prefix = SSE_PREFIX_NONE;
  unsigned rex_prefix = 0;

  i->ResolveModrm = 0;
  i->init(/*os32*/ 1,  // operand size 32 override defaults to 1
          /*as32*/ 1,  // address size 32 override defaults to 1
          /*os64*/ 0,  // operand size 64 override defaults to 0
          /*as64*/ 1); // address size 64 override defaults to 1

fetch_b1:
  b1 = *iptr++;
  remain--;

  switch (b1) {
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
      rex_prefix = b1;
      if (remain != 0) {
        goto fetch_b1;
      }
      return(-1);
    case 0x0f: // 2 byte escape
      if (remain != 0) {
        remain--;
        b1 = 0x100 | *iptr++;
        break;
      }
      return(-1);
    case 0xf2: // REPNE/REPNZ
    case 0xf3: // REP/REPE/REPZ
      rex_prefix = 0;
      sse_prefix = (b1 & 3) ^ 1;
      i->setRepUsed(b1 & 3);
      if (remain != 0) {
        goto fetch_b1;
      }
      return(-1);
    case 0x2e: // CS:
    case 0x26: // ES:
    case 0x36: // SS:
    case 0x3e: // DS:
      /* ignore segment override prefix */
      rex_prefix = 0;
      if (remain != 0) {
        goto fetch_b1;
      }
      return(-1);
    case 0x64: // FS:
    case 0x65: // GS:
      rex_prefix = 0;
      seg_override = b1 & 0xf;
      if (remain != 0) {
        goto fetch_b1;
      }
      return(-1);
    case 0x66: // OpSize
      rex_prefix = 0;
      if(!sse_prefix) sse_prefix = SSE_PREFIX_66;
      i->setOs32B(0);
      offset = 0;
      if (remain != 0) {
        goto fetch_b1;
      }
      return(-1);
    case 0x67: // AddrSize
      rex_prefix = 0;
      i->clearAs64();
      if (remain != 0) {
        goto fetch_b1;
      }
      return(-1);
    case 0xf0: // LOCK:
      rex_prefix = 0;
      lock = 1;
      if (remain != 0) {
        goto fetch_b1;
      }
      return(-1);
    default:
      break;
  }

  if (rex_prefix) {
    i->assertExtend8bit();
    if (rex_prefix & 0x8) {
      i->assertOs64();
      i->assertOs32();
      offset = 512*2;
    }
    rex_r = ((rex_prefix & 0x4) << 1);
    rex_x = ((rex_prefix & 0x2) << 2);
    rex_b = ((rex_prefix & 0x1) << 3);
  }

  i->setB1(b1);

  unsigned index = b1+offset;

  unsigned attr = BxOpcodeInfo64[index].Attr;

  bx_bool has_modrm = BxOpcodeHasModrm64[b1];

  if (has_modrm) {

    unsigned b3 = 0;
    // handle 3-byte escape
    if ((attr & BxGroupX) == Bx3ByteOp) {
      if (remain != 0) {
        remain--;
        b3 = *iptr++;
      }
      else
        return(-1);
    }

    // opcode requires modrm byte
    if (remain != 0) {
      remain--;
      b2 = *iptr++;
    }
    else
      return(-1);

    // Parse mod-nnn-rm and related bytes
    mod = b2 & 0xc0;
    nnn = ((b2 >> 3) & 0x7) | rex_r;
    rm  = (b2 & 0x7) | rex_b;

    // MOVs with CRx and DRx always use register ops and ignore the mod field.
    if ((b1 & ~3) == 0x120)
      mod = 0xc0;

    i->setModRM(b2);
    i->setNnn(nnn);

    if (mod == 0xc0) { // mod == 11b
      i->setRm(rm);
      i->assertModC0();
      goto modrm_done;
    }

    mod_mem = 1;

    i->setRm(BX_TMP_REGISTER);
    i->setSibBase(rm);      // initialize with rm to use BxResolve32Base
    i->setSibIndex(BX_NIL_REGISTER);
    // initialize displ32 with zero to include cases with no diplacement
    i->modRMForm.displ32u = 0;

    if (i->as64L()) {
      // 64-bit addressing modes; note that mod==11b handled above
      i->ResolveModrm = &BX_CPU_C::BxResolve64Base;
      if ((rm & 0x7) != 4) { // no s-i-b byte
        if (mod == 0x00) { // mod == 00b
          if ((rm & 0x7) == 5) {
            i->setSibBase(BX_64BIT_REG_RIP);
            goto get_32bit_displ;
          }
          // mod==00b, rm!=4, rm!=5
          goto modrm_done;
        }
        // (mod == 0x40), mod==01b or (mod == 0x80), mod==10b
        seg = sreg_mod1or2_base32[rm];
      }
      else { // mod!=11b, rm==4, s-i-b byte follows
        unsigned sib, base, index, scale;
        if (remain != 0) {
          sib = *iptr++;
          remain--;
        }
        else {
          return(-1);
        }
        base  = (sib & 0x7) | rex_b; sib >>= 3;
        index = (sib & 0x7) | rex_x; sib >>= 3;
        scale =  sib;
        i->setSibScale(scale);
        i->setSibBase(base);
        if (index != 4) {
          i->ResolveModrm = &BX_CPU_C::BxResolve64BaseIndex;
          i->setSibIndex(index);
        }
        if (mod == 0x00) { // mod==00b, rm==4
          seg = sreg_mod0_base32[base];
          if ((base & 0x7) == 5) {
            i->setSibBase(BX_NIL_REGISTER);
            goto get_32bit_displ;
          }
          // mod==00b, rm==4, base!=5
          goto modrm_done;
        }
        // (mod == 0x40), mod==01b or (mod == 0x80), mod==10b
        seg = sreg_mod1or2_base32[base];
      }
    }
    else {
      // 32-bit addressing modes; note that mod==11b handled above
      i->ResolveModrm = &BX_CPU_C::BxResolve32Base;
      if ((rm & 0x7) != 4) { // no s-i-b byte
        if (mod == 0x00) { // mod == 00b
          if ((rm & 0x7) == 5) {
            i->setSibBase(BX_32BIT_REG_EIP);
            goto get_32bit_displ;
          }
          // mod==00b, rm!=4, rm!=5
          goto modrm_done;
        }
        // (mod == 0x40), mod==01b or (mod == 0x80), mod==10b
        seg = sreg_mod1or2_base32[rm];
      }
      else { // mod!=11b, rm==4, s-i-b byte follows
        unsigned sib, base, index, scale;
        if (remain != 0) {
          sib = *iptr++;
          remain--;
        }
        else {
          return(-1);
        }
        base  = (sib & 0x7) | rex_b; sib >>= 3;
        index = (sib & 0x7) | rex_x; sib >>= 3;
        scale =  sib;
        i->setSibBase(base);
        i->setSibScale(scale);
        if (index != 4) {
          i->ResolveModrm = &BX_CPU_C::BxResolve32BaseIndex;
          i->setSibIndex(index);
        }
        if (mod == 0x00) { // mod==00b, rm==4
          seg = sreg_mod0_base32[base];
          if ((base & 0x7) == 5) {
            i->setSibBase(BX_NIL_REGISTER);
            goto get_32bit_displ;
          }
          // mod==00b, rm==4, base!=5
          goto modrm_done;
        }
        // (mod == 0x40), mod==01b or (mod == 0x80), mod==10b
        seg = sreg_mod1or2_base32[base];
      }
    }

    // (mod == 0x40), mod==01b
    if (mod == 0x40) {
      if (remain != 0) {
        // 8 sign extended to 32
        i->modRMForm.displ32u = (Bit8s) *iptr++;
        remain--;
      }
      else {
        return(-1);
      }
    }
    else {

get_32bit_displ:

      // (mod == 0x80), mod==10b
      if (remain > 3) {
        i->modRMForm.displ32u = FetchDWORD(iptr);
        iptr += 4;
        remain -= 4;
      }
      else {
        return(-1);
      }
    }

modrm_done:

    // Resolve ExecutePtr and additional opcode Attr
    const BxOpcodeInfo_t *OpcodeInfoPtr = &(BxOpcodeInfo64[index]);
    attr = BxOpcodeInfo64[index].Attr;

    while(attr & BxGroupX) {
      Bit32u group = attr & BxGroupX;
      attr &= ~BxGroupX;

      if (group < BxPrefixSSE) {
        /* For opcodes with only one allowed SSE prefix */
        if (sse_prefix != (group >> 4)) {
          OpcodeInfoPtr = &BxOpcodeGroupSSE_ERR[0]; // BX_IA_ERROR
        }
        /* get additional attributes from group table */
        attr |= OpcodeInfoPtr->Attr;
        break;
      }

      switch(group) {
        case BxGroupN:
          OpcodeInfoPtr = &(OpcodeInfoPtr->AnotherArray[nnn & 0x7]);
          break;
        case BxSplitGroupN:
          OpcodeInfoPtr = &(OpcodeInfoPtr->AnotherArray[(nnn & 0x7) + (mod_mem << 3)]);
          break;
        case Bx3ByteOp:
          OpcodeInfoPtr = &(OpcodeInfoPtr->AnotherArray[b3]);
          break;
        case BxOSizeGrp:
          OpcodeInfoPtr = &(OpcodeInfoPtr->AnotherArray[offset >> 9]);
          break;
        case BxPrefixSSE:
          /* For SSE opcodes look into another table
                     with the opcode prefixes (NONE, 0x66, 0xF3, 0xF2) */
          if (sse_prefix) {
            OpcodeInfoPtr = &(OpcodeInfoPtr->AnotherArray[sse_prefix-1]);
            break;
          }
          continue;
        case BxFPEscape:
          if (mod_mem)
            OpcodeInfoPtr = &(OpcodeInfoPtr->AnotherArray[nnn & 0x7]);
          else
            OpcodeInfoPtr = &(OpcodeInfoPtr->AnotherArray[(b2 & 0x3f) + 8]);
          break;
        default:
          BX_PANIC(("fetchdecode: Unknown opcode group"));
      }

      /* get additional attributes from group table */
      attr |= OpcodeInfoPtr->Attr;
    }

    ia_opcode = OpcodeInfoPtr->IA;
  }
  else {
    // Opcode does not require a MODRM byte.
    // Note that a 2-byte opcode (0F XX) will jump to before
    // the if() above after fetching the 2nd byte, so this path is
    // taken in all cases if a modrm byte is NOT required.

    const BxOpcodeInfo_t *OpcodeInfoPtr = &(BxOpcodeInfo64[index]);

    if (b1 == 0x90 && sse_prefix == SSE_PREFIX_F3) {
      ia_opcode = BX_IA_PAUSE;
    }
    else {
      unsigned group = attr & BxGroupX;
      BX_ASSERT(group == BxPrefixSSE || group == 0);
      if (group == BxPrefixSSE && sse_prefix)
        OpcodeInfoPtr = &(OpcodeInfoPtr->AnotherArray[sse_prefix-1]);

      ia_opcode = OpcodeInfoPtr->IA;
      i->setRm((b1 & 7) | rex_b);
    }
  }

  if (lock) { // lock prefix invalid opcode
    // lock prefix not allowed or destination operand is not memory
    // mod == 0xc0 can't be BxLockable in fetchdecode tables
    if (!mod_mem || !(attr & BxLockable)) {
      BX_INFO(("LOCK prefix unallowed (op1=0x%x, modrm=0x%02x)", b1, b2));
      // replace execution function with undefined-opcode
      ia_opcode = BX_IA_ERROR;
    }
  }

  i->modRMForm.Id = 0;
  unsigned imm_mode = attr & BxImmediate;
  if (imm_mode) {
    // make sure iptr was advanced after Ib(), Iw() and Id()
    switch (imm_mode) {
      case BxImmediate_I1:
        i->modRMForm.Ib = 1;
        break;
      case BxImmediate_Ib:
        if (remain != 0) {
          i->modRMForm.Ib = *iptr++;
          remain--;
        }
        else {
          return(-1);
        }
        break;
      case BxImmediate_Ib_SE: // Sign extend to OS size
        if (remain != 0) {
          Bit8s temp8s = *iptr;
          // this code works correctly both for LE and BE hosts
          if (i->os32L())
            i->modRMForm.Id = (Bit32s) temp8s;
          else
            i->modRMForm.Iw = (Bit16s) temp8s;
          remain--;
        }
        else {
          return(-1);
        }
        break;
      case BxImmediate_Iw:
        if (remain > 1) {
          i->modRMForm.Iw = FetchWORD(iptr);
          iptr += 2;
          remain -= 2;
        }
        else {
          return(-1);
        }
        break;
      case BxImmediate_Id:
        if (remain > 3) {
          i->modRMForm.Id = FetchDWORD(iptr);
          iptr += 4;
          remain -= 4;
        }
        else {
          return(-1);
        }
        break;
      case BxImmediate_Iq: // MOV Rx,imm64
        if (remain > 7) {
          i->IqForm.Iq = FetchQWORD(iptr);
          remain -= 8;
        }
        else {
          return(-1);
        }
        break;
      case BxImmediate_BrOff8:
        if (remain != 0) {
          Bit8s temp8s = *iptr;
          i->modRMForm.Id = (Bit32s) temp8s;
          remain--;
        }
        else {
          return(-1);
        }
        break;
      case BxImmediate_O:
        // For instructions which embed the address in the opcode.
        // There is only 64/32-bit addressing available in long64 mode.
        if (i->as64L()) {
          if (remain > 7) {
            i->IqForm.Iq = FetchQWORD(iptr);
            remain -= 8;
          }
          else return(-1);
        }
        else { // as32
          if (remain > 3) {
            i->IqForm.Iq = (Bit64u) FetchDWORD(iptr);
            remain -= 4;
          }
          else return(-1);
        }
        break;
      default:
        BX_INFO(("b1 was %x", b1));
        BX_PANIC(("fetchdecode: imm_mode = %u", imm_mode));
    }

    unsigned imm_mode2 = attr & BxImmediate2;
    if (imm_mode2) {
      if (imm_mode2 == BxImmediate_Ib2) {
        if (remain != 0) {
          i->modRMForm.Ib2 = *iptr;
          remain--;
        }
        else {
          return(-1);
        }
      }
      else {
        BX_INFO(("b1 was %x", b1));
        BX_PANIC(("fetchdecode: imm_mode2 = %u", imm_mode2));
      }
    }
  }

#if BX_SUPPORT_3DNOW
  if(b1 == 0x10f)
     ia_opcode = Bx3DNowOpcode[i->modRMForm.Ib];
#endif

  if (! BX_NULL_SEG_REG(seg_override))
     seg = seg_override;
  i->setSeg(seg);

  i->setILen(remainingInPage - remain);
  i->setIaOpcode(ia_opcode);

  if (mod_mem) {
    i->execute  = BxOpcodesTable[ia_opcode].execute1;
    i->execute2 = BxOpcodesTable[ia_opcode].execute2;
  }
  else {
    i->execute  = BxOpcodesTable[ia_opcode].execute2;
    i->execute2 = NULL;

    if (attr & BxArithDstRM) {
      i->setRm(nnn);
      i->setNnn(rm);
    }
  }

  BX_ASSERT(i->execute);

  Bit32u op_flags = BxOpcodesTable[ia_opcode].flags;
  if (! BX_CPU_THIS_PTR sse_ok) {
     if (op_flags & BX_PREPARE_SSE) {
        if (i->execute != &BX_CPU_C::BxError) i->execute = &BX_CPU_C::BxNoSSE;
        return(1);
     }
  }

#if BX_SUPPORT_TRACE_CACHE
  if ((attr & BxTraceEnd) || ia_opcode == BX_IA_ERROR)
     return(1);
#endif

  return(0);
}

#endif /* if BX_SUPPORT_X86_64 */
