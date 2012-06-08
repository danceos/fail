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

#ifndef ARM_MORPH_FUNCTIONS_H
#define ARM_MORPH_FUNCTIONS_H

// model header files
#include "armMode.h"
#include "armMorph.h"

// unop instructions
ARM_MORPH_FN(armEmitUnopI);
ARM_MORPH_FN(armEmitUnopR);
ARM_MORPH_FN(armEmitUnopRSI);
ARM_MORPH_FN(armEmitUnopRSR);
ARM_MORPH_FN(armEmitUnopRSRT);
ARM_MORPH_FN(armEmitUnopRX);

// binop instructions
ARM_MORPH_FN(armEmitBinopI);
ARM_MORPH_FN(armEmitBinopR);
ARM_MORPH_FN(armEmitBinopRT);
ARM_MORPH_FN(armEmitBinopIT);
ARM_MORPH_FN(armEmitBinopADR);
ARM_MORPH_FN(armEmitBinopRSI);
ARM_MORPH_FN(armEmitBinopRSR);
ARM_MORPH_FN(armEmitBinopRX);

// cmpop instructions
ARM_MORPH_FN(armEmitCmpopI);
ARM_MORPH_FN(armEmitCmpopR);
ARM_MORPH_FN(armEmitCmpopRSI);
ARM_MORPH_FN(armEmitCmpopRX);

// multiply and divide instructions
ARM_MORPH_FN(armEmitMUL);
ARM_MORPH_FN(armEmitMLA);
ARM_MORPH_FN(armEmitMLS);
ARM_MORPH_FN(armEmitDIV);
ARM_MORPH_FN(armEmitMULL);
ARM_MORPH_FN(armEmitMLAL);

// branch instructions
ARM_MORPH_FN(armEmitBranchC);
ARM_MORPH_FN(armEmitBranchR);
ARM_MORPH_FN(armEmitBLX);

// miscellaneous instructions
ARM_MORPH_FN(armEmitCLZ);
ARM_MORPH_FN(armEmitBKPT);
ARM_MORPH_FN(armEmitSWI);

// load and store instructions
ARM_MORPH_FN(armEmitLDRI);
ARM_MORPH_FN(armEmitLDRR);
ARM_MORPH_FN(armEmitLDRRSI);
ARM_MORPH_FN(armEmitLDM1);
ARM_MORPH_FN(armEmitSTRI);
ARM_MORPH_FN(armEmitSTRR);
ARM_MORPH_FN(armEmitSTRRSI);
ARM_MORPH_FN(armEmitSTM1);

// coprocessor instructions
ARM_MORPH_FN(armEmitUsageFaultCP);

// status register access instructions
ARM_MORPH_FN(armEmitMRS);
ARM_MORPH_FN(armEmitMSR);

// DSP data processing instructions
ARM_MORPH_FN(armEmitQADD);
ARM_MORPH_FN(armEmitQSUB);
ARM_MORPH_FN(armEmitQDADD);
ARM_MORPH_FN(armEmitQDSUB);

// DSP multiply instructions
ARM_MORPH_FN(armEmitSMLABB);
ARM_MORPH_FN(armEmitSMLABT);
ARM_MORPH_FN(armEmitSMLATB);
ARM_MORPH_FN(armEmitSMLATT);
ARM_MORPH_FN(armEmitSMLALBB);
ARM_MORPH_FN(armEmitSMLALBT);
ARM_MORPH_FN(armEmitSMLALTB);
ARM_MORPH_FN(armEmitSMLALTT);
ARM_MORPH_FN(armEmitSMLAWB);
ARM_MORPH_FN(armEmitSMLAWT);
ARM_MORPH_FN(armEmitSMULBB);
ARM_MORPH_FN(armEmitSMULBT);
ARM_MORPH_FN(armEmitSMULTB);
ARM_MORPH_FN(armEmitSMULTT);
ARM_MORPH_FN(armEmitSMULWB);
ARM_MORPH_FN(armEmitSMULWT);

// hint instructions
ARM_MORPH_FN(armEmitNOP);
ARM_MORPH_FN(armEmitWFE);
ARM_MORPH_FN(armEmitWFI);
ARM_MORPH_FN(armEmitSEV);

// move instructions
ARM_MORPH_FN(armEmitMOVW);
ARM_MORPH_FN(armEmitMOVT);

// multiply instructions
ARM_MORPH_FN(armEmitMAAL);

// synchronization instructions
ARM_MORPH_FN(armEmitLDREX);
ARM_MORPH_FN(armEmitSTREX);
ARM_MORPH_FN(armEmitCLREX);

// miscellaneous instructions
ARM_MORPH_FN(armEmitCPS);

// branch instructions
ARM_MORPH_FN(armEmitCBZ);
ARM_MORPH_FN(armEmitTB);

// basic media instructions
ARM_MORPH_FN(armEmitUSAD8);
ARM_MORPH_FN(armEmitUSADA8);
ARM_MORPH_FN(armEmitSBFX);
ARM_MORPH_FN(armEmitBFC);
ARM_MORPH_FN(armEmitBFI);
ARM_MORPH_FN(armEmitUBFX);

// parallel add/subtract instructions
ARM_MORPH_FN(armEmitParallelBinop8);
ARM_MORPH_FN(armEmitParallelBinop16);

// packing, unpacking, saturation and reversal instructions
ARM_MORPH_FN(armEmitPKHBT);
ARM_MORPH_FN(armEmitPKHTB);
ARM_MORPH_FN(armEmitSSAT);
ARM_MORPH_FN(armEmitSSAT16);
ARM_MORPH_FN(armEmitUSAT);
ARM_MORPH_FN(armEmitUSAT16);
ARM_MORPH_FN(armEmitSXTAB);
ARM_MORPH_FN(armEmitUXTAB);
ARM_MORPH_FN(armEmitSXTAB16);
ARM_MORPH_FN(armEmitUXTAB16);
ARM_MORPH_FN(armEmitSXTAH);
ARM_MORPH_FN(armEmitUXTAH);
ARM_MORPH_FN(armEmitSXTB);
ARM_MORPH_FN(armEmitUXTB);
ARM_MORPH_FN(armEmitSXTB16);
ARM_MORPH_FN(armEmitUXTB16);
ARM_MORPH_FN(armEmitSXTH);
ARM_MORPH_FN(armEmitUXTH);
ARM_MORPH_FN(armEmitSEL);
ARM_MORPH_FN(armEmitREV);
ARM_MORPH_FN(armEmitREV16);
ARM_MORPH_FN(armEmitRBIT);
ARM_MORPH_FN(armEmitREVSH);

// signed multiply instructions
ARM_MORPH_FN(armEmitSMLXD);
ARM_MORPH_FN(armEmitSMUXD);
ARM_MORPH_FN(armEmitSMLXLD);
ARM_MORPH_FN(armEmitSMMLX);

// VFP data processing instructions
ARM_MORPH_FN(armEmitVFPUnop);
ARM_MORPH_FN(armEmitVFPBinop);
ARM_MORPH_FN(armEmitVMulAcc_VFP);
ARM_MORPH_FN(armEmitVFusedMAC);
ARM_MORPH_FN(armEmitVMOVI_VFP);
ARM_MORPH_FN(armEmitVMOVR_VFP);
ARM_MORPH_FN(armEmitVABS_VFP);
ARM_MORPH_FN(armEmitVNEG_VFP);
ARM_MORPH_FN(armEmitVCMP_VFP);
ARM_MORPH_FN(armEmitVCMP0_VFP);
ARM_MORPH_FN(armEmitVCVT_SD_VFP);
ARM_MORPH_FN(armEmitVCVT_SH_VFP);
ARM_MORPH_FN(armEmitVCVT_HS_VFP);
ARM_MORPH_FN(armEmitVCVT_XF_VFP);
ARM_MORPH_FN(armEmitVCVT_FX_VFP);
ARM_MORPH_FN(armEmitVCVT_IF_VFP);
ARM_MORPH_FN(armEmitVCVT_FI_VFP);

// VFP Extension register load/store instructions
ARM_MORPH_FN(armEmitVLDR);
ARM_MORPH_FN(armEmitVSTR);
ARM_MORPH_FN(armEmitVLDM);
ARM_MORPH_FN(armEmitVSTM);

// VFP 8, 16 and 32-bit transfer instructions
ARM_MORPH_FN(armEmitVMRS);
ARM_MORPH_FN(armEmitVMSR);
ARM_MORPH_FN(armEmitVMOVRS);
ARM_MORPH_FN(armEmitVMOVSR);
ARM_MORPH_FN(armEmitVMOVRZ);
ARM_MORPH_FN(armEmitVMOVZR);
ARM_MORPH_FN(armEmitVDUPR);

// VFP 64-bit transfer instructions
ARM_MORPH_FN(armEmitVMOVRRD);
ARM_MORPH_FN(armEmitVMOVDRR);
ARM_MORPH_FN(armEmitVMOVRRSS);
ARM_MORPH_FN(armEmitVMOVSSRR);

//
// If the register index specifies a banked register, return a vmiReg structure
// for the banked register; otherwise, return VMI_NOREG. Also, validate the
// current block mode for banked registers.
//
vmiReg armGetBankedRegMode(Bool useSPProcess, Uns32 r);

#endif
