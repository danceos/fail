/*
 * Copyright (c) 2005-2011 Imperas Software Ltd., www.imperas.com
 *
 * YOUR ACCESS TO THE INFORMATION IN THIS MODEL IS CONDITIONAL
 * UPON YOUR ACCEPTANCE THAT YOU WILL NOT USE OR PERMIT OTHERS
 * TO USE THE INFORMATION FOR THE PURPOSES OF DETERMINING WHETHER
 * IMPLEMENTATIONS OF THE ARM ARCHITECTURE INFRINGE ANY THIRD
 * PARTY PATENTS.
 *
 * THE LICENSE BELOW EXTENDS ONLY TO USE OF THE SOFTWARE FOR
 * MODELING PURPOSES AND SHALL NOT BE CONSTRUED AS GRANTING
 * A LICENSE TO CREATE A HARDWARE IMPLEMENTATION OF THE
 * FUNCTIONALITY OF THE SOFTWARE LICENSED HEREUNDER.
 * YOU MAY USE THE SOFTWARE SUBJECT TO THE LICENSE TERMS BELOW
 * PROVIDED THAT YOU ENSURE THAT THIS NOTICE IS REPLICATED UNMODIFIED
 * AND IN ITS ENTIRETY IN ALL DISTRIBUTIONS OF THE SOFTWARE,
 * MODIFIED OR UNMODIFIED, IN SOURCE CODE OR IN BINARY FORM.
 *
 * Licensed under an Imperas Modfied Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.ovpworld.org/licenses/OVP_MODIFIED_1.0_APACHE_OPEN_SOURCE_LICENSE_2.0.pdf
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// model header files
#include "armConfig.h"
#include "armVariant.h"

const struct armConfigS armConfigTable[] = {

    ////////////////////////////////////////////////////////////////////////////
    // ISA CONFIGURATIONS
    ////////////////////////////////////////////////////////////////////////////

    {.name = "ARMv7-M", .arch = ARM_V7, .rotateUnaligned = True},

    ////////////////////////////////////////////////////////////////////////////
    // PROCESSOR MODEL CONFIGURATIONS
    ////////////////////////////////////////////////////////////////////////////

    {
        .name            = "Cortex-M3",
        .arch            = ARM_V7,
        .numInterrupts   = 16,
        .rotateUnaligned = False,
        .align64as32     = True,    // ARMv7-M has no 64-bit load/stores
        .STRoffsetPC12   = False,   // required value for ARMv7 on
        .priorityBitsM1  = 2,       // number of priority bits (minus 1)

        .regDefaults = {
            .CPUID = {
                .REVISION     = 0,
                .PARTNO       = 0xc23,
                .ARCHITECTURE = 0xf,
                .VARIANT      = 0x2,
                .IMPLEMENTER  = AI_ARM
            },
            .ID_PFR0 = {
                .State0 = 0,        // 32-bit ARM instruction set support
                .State1 = 3,        // Thumb encoding support
                .State2 = 0,        // Jazelle support
                .State3 = 0         // ThumbEE support
            },
            .ID_PFR1 = {
                .ProgrammersModel      = 0, // ARM programmer's model
                .SecurityExtension     = 0, // Security Extensions support
                .MicroProgrammersModel = 2  // Microcontroller programmer's model
            },
            .ID_DFR0 = {
                .CoreDebug     = 0, // Core debug model
                .SecureDebug   = 0, // Secure debug model
                .EmbeddedDebug = 0, // Embedded debug model
                .TraceDebugCP  = 0, // Trace debug model, coprocessor-based
                .TraceDebugMM  = 0, // Trace debug model, memory mapped
                .MicroDebug    = 0, // Microcontroller debug model
            },
            .ID_MMFR0 = {
                .VMSA        = 0,   // VMSA support
                .PMSA        = 3,   // PMSA support
                .Cache_Agent = 0,   // Cache coherency + CPU agent
                .Cache_DMA   = 0,   // Cache coherency + associated DMA
                .TCM_DMA     = 0,   // TCM + associated DMA
                .AuxControl  = 0,   // ARMv6 Auxillary Control register
                .FCSE        = 0,   // FCSE support
            },
            .ID_MMFR1 = {
                .L1VAHarvard = 0,   // L1 maintainence by VA, Harvard
                .L1VAUnified = 0,   // L1 maintainence by VA, unified
                .L1SWHarvard = 0,   // L1 maintainence by Set/Way, Harvard
                .L1SWUnified = 0,   // L1 maintainence by Set/Way, unified
                .L1Harvard   = 0,   // L1 maintainence, Harvard
                .L1Unified   = 0,   // L1 maintainence, unified
                .L1TestClean = 0,   // L1 test and clean
                .BTB         = 0    // Branch target buffer
            },
            .ID_MMFR2 = {
                .L1FgndPrefetchHarvard  = 0,    // L1 F/ground cache p/fetch range, Harvard
                .L1BgndPrefetchHarvard  = 0,    // L1 B/ground cache p/fetch range, Harvard
                .L1MaintRangeHarvard    = 0,    // L1 maintanence range, Harvard
                .TLBMaintHarvard        = 0,    // TLB maintanence, Harvard
                .TLBMaintUnified        = 0,    // TLB maintanence, Unified
                .MemoryBarrierCP15      = 0,    // Memory Barrier, CP15 based
                .WaitForInterruptStall  = 0,    // Wait-for-interrupt stalling
                .HWAccessFlag           = 0     // hardware access flag support
            },
            .ID_MMFR3 = {
                .HierMaintSW   = 0, // Hierarchical cache maintainence, set/way
                .HierMaintMVA  = 0, // Hierarchical cache maintainence, MVA
                .BPMaint       = 0, // Branch predictor maintainence
            },
            .ID_ISAR0 = {
                .Swap_instrs      = 0,  // Atomic instructions
                .BitCount_instrs  = 1,  // BitCount instructions
                .BitField_instrs  = 1,  // BitField instructions
                .CmpBranch_instrs = 1,  // CmpBranch instructions
                .Coproc_instrs    = 4,  // Coprocessor instructions
                .Debug_instrs     = 1,  // Debug instructions
                .Divide_instrs    = 1,  // Divide instructions
            },
            .ID_ISAR1 = {
                .Endian_instrs    = 0,  // Endian instructions
                .Except_instrs    = 0,  // Exception instructions
                .Except_AR_instrs = 0,  // A/R profile exception instructions
                .Extend_instrs    = 1,  // Extend instructions
                .IfThen_instrs    = 1,  // IfThen instructions
                .Immediate_instrs = 1,  // Immediate instructions
                .Interwork_instrs = 2,  // Interwork instructions
                .Jazelle_instrs   = 0   // Jazelle instructions
            },
            .ID_ISAR2 = {
                .LoadStore_instrs      = 1, // LoadStore instructions
                .MemHint_instrs        = 3, // MemoryHint instructions
                .MultiAccessInt_instrs = 2, // Multi-access interruptible instructions
                .Mult_instrs           = 2, // Multiply instructions
                .MultS_instrs          = 1, // Multiply instructions, advanced signed
                .MultU_instrs          = 1, // Multiply instructions, advanced unsigned
                .PSR_AR_instrs         = 1, // A/R profile PSR instructions
                .Reversal_instrs       = 2  // Reversal instructions
            },
            .ID_ISAR3 = {
                .Saturate_instrs     = 0,   // Saturate instructions
                .SIMD_instrs         = 1,   // SIMD instructions
                .SVC_instrs          = 1,   // SVC instructions
                .SynchPrim_instrs    = 1,   // SynchPrim instructions
                .TabBranch_instrs    = 1,   // TableBranch instructions
                .ThumbCopy_instrs    = 1,   // ThumbCopy instructions
                .TrueNOP_instrs      = 1,   // TrueNOP instructions
                .T2ExeEnvExtn_instrs = 0    // Thumb-2 Execution env extensions
            },
            .ID_ISAR4 = {
                .Unpriv_instrs         = 2, // Unprivileged instructions
                .WithShifts_instrs     = 0, // Shift instructions
                .Writeback_instrs      = 1, // Writeback instructions
                .SMI_instrs            = 0, // SMI instructions
                .Barrier_instrs        = 1, // Barrier instructions
                .SynchPrim_instrs_frac = 3, // Fractional support for sync primitive instructions
                .PSR_M_instrs          = 1, // M-profile forms of PSR instructions
                .SWP_frac              = 0  // memory system bus locking
            },
            .ICTR = {
                .INTLINESNUM = 0    // number of interrupt lines supported
            },
            .ACTLR = {0},           // auxillary control register
            .MPU_TYPE = {
                .SEPARATE = 0,      // unified MPU
                .DREGION  = 8,      // number of data/unified memory regions
                .IREGION  = 0       // number of instruction memory regions
            },
            .SYST_CALIB = {
                .NOREF = 0,         // reference clock provided
                .SKEW  = 0,         // whether calibration value inexact
                .TENMS = 0          // 10ms reload value
            }
        },

        .regMasks = {
        }
    },

    {
        .name            = "Cortex-M4",
        .arch            = ARM_V7,
        .numInterrupts   = 16,
        .rotateUnaligned = False,
        .align64as32     = True,    // ARMv7-M has no 64-bit load/stores
        .STRoffsetPC12   = False,   // required value for ARMv7 on
        .priorityBitsM1  = 2,       // number of priority bits (minus 1)

        .regDefaults = {
            .CPUID = {
                .REVISION     = 1,
                .PARTNO       = 0xc24,
                .ARCHITECTURE = 0xf,
                .VARIANT      = 0x0,
                .IMPLEMENTER  = AI_ARM
            },
            .ID_PFR0 = {
                .State0 = 0,        // 32-bit ARM instruction set support
                .State1 = 3,        // Thumb encoding support
                .State2 = 0,        // Jazelle support
                .State3 = 0         // ThumbEE support
            },
            .ID_PFR1 = {
                .ProgrammersModel      = 0, // ARM programmer's model
                .SecurityExtension     = 0, // Security Extensions support
                .MicroProgrammersModel = 2  // Microcontroller programmer's model
            },
            .ID_DFR0 = {
                .CoreDebug     = 0, // Core debug model
                .SecureDebug   = 0, // Secure debug model
                .EmbeddedDebug = 0, // Embedded debug model
                .TraceDebugCP  = 0, // Trace debug model, coprocessor-based
                .TraceDebugMM  = 0, // Trace debug model, memory mapped
                .MicroDebug    = 0, // Microcontroller debug model
            },
            .ID_MMFR0 = {
                .VMSA        = 0,   // VMSA support
                .PMSA        = 3,   // PMSA support
                .Cache_Agent = 0,   // Cache coherency + CPU agent
                .Cache_DMA   = 0,   // Cache coherency + associated DMA
                .TCM_DMA     = 0,   // TCM + associated DMA
                .AuxControl  = 0,   // ARMv6 Auxillary Control register
                .FCSE        = 0,   // FCSE support
            },
            .ID_MMFR1 = {
                .L1VAHarvard = 0,   // L1 maintainence by VA, Harvard
                .L1VAUnified = 0,   // L1 maintainence by VA, unified
                .L1SWHarvard = 0,   // L1 maintainence by Set/Way, Harvard
                .L1SWUnified = 0,   // L1 maintainence by Set/Way, unified
                .L1Harvard   = 0,   // L1 maintainence, Harvard
                .L1Unified   = 0,   // L1 maintainence, unified
                .L1TestClean = 0,   // L1 test and clean
                .BTB         = 0    // Branch target buffer
            },
            .ID_MMFR2 = {
                .L1FgndPrefetchHarvard  = 0,    // L1 F/ground cache p/fetch range, Harvard
                .L1BgndPrefetchHarvard  = 0,    // L1 B/ground cache p/fetch range, Harvard
                .L1MaintRangeHarvard    = 0,    // L1 maintanence range, Harvard
                .TLBMaintHarvard        = 0,    // TLB maintanence, Harvard
                .TLBMaintUnified        = 0,    // TLB maintanence, Unified
                .MemoryBarrierCP15      = 0,    // Memory Barrier, CP15 based
                .WaitForInterruptStall  = 0,    // Wait-for-interrupt stalling
                .HWAccessFlag           = 0     // hardware access flag support
            },
            .ID_MMFR3 = {
                .HierMaintSW   = 0, // Hierarchical cache maintainence, set/way
                .HierMaintMVA  = 0, // Hierarchical cache maintainence, MVA
                .BPMaint       = 0, // Branch predictor maintainence
            },
            .ID_ISAR0 = {
                .Swap_instrs      = 0,  // Atomic instructions
                .BitCount_instrs  = 1,  // BitCount instructions
                .BitField_instrs  = 1,  // BitField instructions
                .CmpBranch_instrs = 1,  // CmpBranch instructions
                .Coproc_instrs    = 4,  // Coprocessor instructions
                .Debug_instrs     = 1,  // Debug instructions
                .Divide_instrs    = 1,  // Divide instructions
            },
            .ID_ISAR1 = {
                .Endian_instrs    = 0,  // Endian instructions
                .Except_instrs    = 0,  // Exception instructions
                .Except_AR_instrs = 0,  // A/R profile exception instructions
                .Extend_instrs    = 2,  // Extend instructions
                .IfThen_instrs    = 1,  // IfThen instructions
                .Immediate_instrs = 1,  // Immediate instructions
                .Interwork_instrs = 2,  // Interwork instructions
                .Jazelle_instrs   = 0   // Jazelle instructions
            },
            .ID_ISAR2 = {
                .LoadStore_instrs      = 1, // LoadStore instructions
                .MemHint_instrs        = 3, // MemoryHint instructions
                .MultiAccessInt_instrs = 2, // Multi-access interruptible instructions
                .Mult_instrs           = 2, // Multiply instructions
                .MultS_instrs          = 3, // Multiply instructions, advanced signed
                .MultU_instrs          = 2, // Multiply instructions, advanced unsigned
                .PSR_AR_instrs         = 1, // A/R profile PSR instructions
                .Reversal_instrs       = 2  // Reversal instructions
            },
            .ID_ISAR3 = {
                .Saturate_instrs     = 1,   // Saturate instructions
                .SIMD_instrs         = 3,   // SIMD instructions
                .SVC_instrs          = 1,   // SVC instructions
                .SynchPrim_instrs    = 1,   // SynchPrim instructions
                .TabBranch_instrs    = 1,   // TableBranch instructions
                .ThumbCopy_instrs    = 1,   // ThumbCopy instructions
                .TrueNOP_instrs      = 1,   // TrueNOP instructions
                .T2ExeEnvExtn_instrs = 0    // Thumb-2 Execution env extensions
            },
            .ID_ISAR4 = {
                .Unpriv_instrs         = 2, // Unprivileged instructions
                .WithShifts_instrs     = 3, // Shift instructions
                .Writeback_instrs      = 1, // Writeback instructions
                .SMI_instrs            = 0, // SMI instructions
                .Barrier_instrs        = 1, // Barrier instructions
                .SynchPrim_instrs_frac = 3, // Fractional support for sync primitive instructions
                .PSR_M_instrs          = 1, // M-profile forms of PSR instructions
                .SWP_frac              = 0  // memory system bus locking
            },
            .ICTR = {
                .INTLINESNUM = 0    // number of interrupt lines supported
            },
            .ACTLR = {0},           // auxillary control register
            .MPU_TYPE = {
                .SEPARATE = 0,      // unified MPU
                .DREGION  = 8,      // number of data/unified memory regions
                .IREGION  = 0       // number of instruction memory regions
            },
            .SYST_CALIB = {
                .NOREF = 0,         // reference clock provided
                .SKEW  = 0,         // whether calibration value inexact
                .TENMS = 0          // 10ms reload value
            },
        },

        .regMasks = {
        }
    },

    {
        .name            = "Cortex-M4F",
        .arch            = ARM_V7,
        .numInterrupts   = 16,
        .rotateUnaligned = False,
        .align64as32     = True,    // ARMv7-M has no 64-bit load/stores
        .STRoffsetPC12   = False,   // required value for ARMv7 on
        .priorityBitsM1  = 2,       // number of priority bits (minus 1)

        .regDefaults = {
            .CPUID = {
                .REVISION     = 1,
                .PARTNO       = 0xc24,
                .ARCHITECTURE = 0xf,
                .VARIANT      = 0x0,
                .IMPLEMENTER  = AI_ARM
            },
            .ID_PFR0 = {
                .State0 = 0,        // 32-bit ARM instruction set support
                .State1 = 3,        // Thumb encoding support
                .State2 = 0,        // Jazelle support
                .State3 = 0         // ThumbEE support
            },
            .ID_PFR1 = {
                .ProgrammersModel      = 0, // ARM programmer's model
                .SecurityExtension     = 0, // Security Extensions support
                .MicroProgrammersModel = 2  // Microcontroller programmer's model
            },
            .ID_DFR0 = {
                .CoreDebug     = 0, // Core debug model
                .SecureDebug   = 0, // Secure debug model
                .EmbeddedDebug = 0, // Embedded debug model
                .TraceDebugCP  = 0, // Trace debug model, coprocessor-based
                .TraceDebugMM  = 0, // Trace debug model, memory mapped
                .MicroDebug    = 0, // Microcontroller debug model
            },
            .ID_MMFR0 = {
                .VMSA        = 0,   // VMSA support
                .PMSA        = 3,   // PMSA support
                .Cache_Agent = 0,   // Cache coherency + CPU agent
                .Cache_DMA   = 0,   // Cache coherency + associated DMA
                .TCM_DMA     = 0,   // TCM + associated DMA
                .AuxControl  = 0,   // ARMv6 Auxillary Control register
                .FCSE        = 0,   // FCSE support
            },
            .ID_MMFR1 = {
                .L1VAHarvard = 0,   // L1 maintainence by VA, Harvard
                .L1VAUnified = 0,   // L1 maintainence by VA, unified
                .L1SWHarvard = 0,   // L1 maintainence by Set/Way, Harvard
                .L1SWUnified = 0,   // L1 maintainence by Set/Way, unified
                .L1Harvard   = 0,   // L1 maintainence, Harvard
                .L1Unified   = 0,   // L1 maintainence, unified
                .L1TestClean = 0,   // L1 test and clean
                .BTB         = 0    // Branch target buffer
            },
            .ID_MMFR2 = {
                .L1FgndPrefetchHarvard  = 0,    // L1 F/ground cache p/fetch range, Harvard
                .L1BgndPrefetchHarvard  = 0,    // L1 B/ground cache p/fetch range, Harvard
                .L1MaintRangeHarvard    = 0,    // L1 maintanence range, Harvard
                .TLBMaintHarvard        = 0,    // TLB maintanence, Harvard
                .TLBMaintUnified        = 0,    // TLB maintanence, Unified
                .MemoryBarrierCP15      = 0,    // Memory Barrier, CP15 based
                .WaitForInterruptStall  = 0,    // Wait-for-interrupt stalling
                .HWAccessFlag           = 0     // hardware access flag support
            },
            .ID_MMFR3 = {
                .HierMaintSW   = 0, // Hierarchical cache maintainence, set/way
                .HierMaintMVA  = 0, // Hierarchical cache maintainence, MVA
                .BPMaint       = 0, // Branch predictor maintainence
            },
            .ID_ISAR0 = {
                .Swap_instrs      = 0,  // Atomic instructions
                .BitCount_instrs  = 1,  // BitCount instructions
                .BitField_instrs  = 1,  // BitField instructions
                .CmpBranch_instrs = 1,  // CmpBranch instructions
                .Coproc_instrs    = 4,  // Coprocessor instructions
                .Debug_instrs     = 1,  // Debug instructions
                .Divide_instrs    = 1,  // Divide instructions
            },
            .ID_ISAR1 = {
                .Endian_instrs    = 0,  // Endian instructions
                .Except_instrs    = 0,  // Exception instructions
                .Except_AR_instrs = 0,  // A/R profile exception instructions
                .Extend_instrs    = 2,  // Extend instructions
                .IfThen_instrs    = 1,  // IfThen instructions
                .Immediate_instrs = 1,  // Immediate instructions
                .Interwork_instrs = 2,  // Interwork instructions
                .Jazelle_instrs   = 0   // Jazelle instructions
            },
            .ID_ISAR2 = {
                .LoadStore_instrs      = 1, // LoadStore instructions
                .MemHint_instrs        = 3, // MemoryHint instructions
                .MultiAccessInt_instrs = 2, // Multi-access interruptible instructions
                .Mult_instrs           = 2, // Multiply instructions
                .MultS_instrs          = 3, // Multiply instructions, advanced signed
                .MultU_instrs          = 2, // Multiply instructions, advanced unsigned
                .PSR_AR_instrs         = 1, // A/R profile PSR instructions
                .Reversal_instrs       = 2  // Reversal instructions
            },
            .ID_ISAR3 = {
                .Saturate_instrs     = 1,   // Saturate instructions
                .SIMD_instrs         = 3,   // SIMD instructions
                .SVC_instrs          = 1,   // SVC instructions
                .SynchPrim_instrs    = 1,   // SynchPrim instructions
                .TabBranch_instrs    = 1,   // TableBranch instructions
                .ThumbCopy_instrs    = 1,   // ThumbCopy instructions
                .TrueNOP_instrs      = 1,   // TrueNOP instructions
                .T2ExeEnvExtn_instrs = 0    // Thumb-2 Execution env extensions
            },
            .ID_ISAR4 = {
                .Unpriv_instrs         = 2, // Unprivileged instructions
                .WithShifts_instrs     = 3, // Shift instructions
                .Writeback_instrs      = 1, // Writeback instructions
                .SMI_instrs            = 0, // SMI instructions
                .Barrier_instrs        = 1, // Barrier instructions
                .SynchPrim_instrs_frac = 3, // Fractional support for sync primitive instructions
                .PSR_M_instrs          = 1, // M-profile forms of PSR instructions
                .SWP_frac              = 0  // memory system bus locking
            },
            .ICTR = {
                .INTLINESNUM = 0    // number of interrupt lines supported
            },
            .ACTLR = {0},           // auxillary control register
            .MPU_TYPE = {
                .SEPARATE = 0,      // unified MPU
                .DREGION  = 8,      // number of data/unified memory regions
                .IREGION  = 0       // number of instruction memory regions
            },
            .SYST_CALIB = {
                .NOREF = 0,         // reference clock provided
                .SKEW  = 0,         // whether calibration value inexact
                .TENMS = 0          // 10ms reload value
            },
            .MVFR0 = {
                .A_SIMD_Registers  = 1, // 16x64-bit media register bank
                .SinglePrecision   = 2, // single precision supported
                .DoublePrecision   = 0, // double precision not supported
                .VFP_ExceptionTrap = 0, // trapped exceptions not supported
                .Divide            = 1, // VFP hardware divide supported
                .SquareRoot        = 1, // VFP hardware square root supported
                .ShortVectors      = 0, // VFP short vector not supported
                .VFP_RoundingModes = 1  // all VFP rounding modes supported
            },
            .MVFR1 = {
                .FlushToZeroMode   = 1, // VFP denormal arithmetic supported
                .DefaultNaNMode    = 1, // VFP NaN propagation supported
                .VFP_HalfPrecision = 1, // VFP half-precision not supported
                .VFP_FusedMAC      = 1, // Fused multiply accumulate supported
            }
        },

        .regMasks = {
        }
    },


    // null terminator
    {0}
};

