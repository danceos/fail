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

#include "armFunctions.h"

#define VENDOR  "arm.ovpworld.org"
#define LIBRARY "processor" 
#define NAME    "armm"
#define VERSION "1.0"
#define VARIANT "ARMv7-M"

VMI_PROC_INFO_FN(armProcInfo) {
    static const vmiProcessorInfo info = {
        .vlnv.vendor      = VENDOR            ,
        .vlnv.library     = LIBRARY           ,
        .vlnv.name        = NAME              ,
        .vlnv.version     = VERSION           ,

        .semihost.vendor  = VENDOR,
        .semihost.library = "semihosting"     ,
        .semihost.name    = "armNewlib"       ,
        .semihost.version = "1.0"             ,

        .elfCode          = 40,
        .endianFixed      = False,
        .gdbPath          = "$IMPERAS_HOME/lib/$IMPERAS_ARCH/CrossCompiler/arm-elf/bin/arm-elf-gdb" VMI_EXE_SUFFIX,
        .procdoc          = DOCPATH(VENDOR,LIBRARY,NAME,VERSION,VARIANT),
        .family           = "ARM",
    };
    return &info;
}
