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

// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiModelInfo.h"
#include "vmi/vmiDoc.h"

#include "armmDoc.h"
#include "armmParameters.h"
#include "armStructure.h"
#include "armConfig.h"

void armmDoc(vmiProcessorP processor, armParamValuesP parameters) {
    vmiDocNodeP root = vmidocAddSection(0, "Root");
    vmiDocNodeP desc = vmidocAddSection(root, "Description");
    vmidocAddText(desc, "ARMM Processor Model");

    vmiDocNodeP lic  = vmidocAddSection(root, "Licensing");
    vmidocAddText(lic,  "Imperas Modified Apache 2.0 Open Source License");

    vmiDocNodeP lim  = vmidocAddSection(root, "Limitations");
    vmidocAddText(lim, "Security Extensions are not yet implemented.");
    vmidocAddText(lim, "Performance Monitors are not implemented.");

    vmiDocNodeP verif = vmidocAddSection(root, "Verification");
    vmidocAddText(verif, "Models have been validated correct by Imperas "
                         "running through extensive unit and full operating system tests "
                         "and validated in some cases against hardware");

    armP arm = (armP) processor;

    vmiDocNodeP features  = vmidocAddSection(root, "Features");

    if (MPU_PRESENT(arm)) {
        vmidocAddText(features, "MPU is implemented.");
    }

    if (ARM_SUPPORT(arm->configInfo.arch, ARM_VT)) {
        vmidocAddText(features, "Thumb instructions are supported.");
    }
    if (ARM_SUPPORT(arm->configInfo.arch, ARM_VT2)) {
        vmidocAddText(features, "Thumb-2 instructions are supported.");
    }
    if (ARM_SUPPORT(arm->configInfo.arch, ARM_J)) {
        vmidocAddText(features, "Jazelle trap is implemented.");
    }

    vmidocProcessor(processor, root);
}
