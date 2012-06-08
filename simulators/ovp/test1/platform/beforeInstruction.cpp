#include "beforeInstruction.hpp"

Uns8 bpCount;
Addr breakpoints[255];

//Ignore warning: deprecated conversion from string constant to ‘char*’
#pragma GCC diagnostic ignored "-Wwrite-strings"

void printArmCortexM3Registers(icmProcessorObject processor) {
    Uns32 registers[17];
    icmPrintf("---------------\t\t---------------\n");
    readRegister(processor, "R0",   registers[0]);
    readRegister(processor, "R1",   registers[1]);
    readRegister(processor, "R2",   registers[2]);
    readRegister(processor, "R3",   registers[3]);
    readRegister(processor, "R4",   registers[4]);
    readRegister(processor, "R5",   registers[5]);
    readRegister(processor, "R6",   registers[6]);
    readRegister(processor, "R7",   registers[7]);
    readRegister(processor, "R8",   registers[8]);
    readRegister(processor, "R9",   registers[9]);
    readRegister(processor, "R10",  registers[10]);
    readRegister(processor, "R11",  registers[11]);
    readRegister(processor, "R12",  registers[12]);
    readRegister(processor, "SP",   registers[13]);
    readRegister(processor, "LR",   registers[14]);
    readRegister(processor, "PC",   registers[15]);
    readRegister(processor, "CPSR", registers[16]);
    icmPrintf("R0:  0x%08x\t\t",    registers[0]);
    icmPrintf("R1:  0x%08x\n",      registers[1]);
    icmPrintf("R2:  0x%08x\t\t",    registers[2]);
    icmPrintf("R3:  0x%08x\n",      registers[3]);
    icmPrintf("R4:  0x%08x\t\t",    registers[4]);
    icmPrintf("R5:  0x%08x\n",      registers[5]);
    icmPrintf("R6:  0x%08x\t\t",    registers[6]);
    icmPrintf("R7:  0x%08x\n",      registers[7]);
    icmPrintf("R8:  0x%08x\t\t",    registers[8]);
    icmPrintf("R9:  0x%08x\n",      registers[9]);
    icmPrintf("R10: 0x%08x\t\t",    registers[10]);
    icmPrintf("R11: 0x%08x\n",      registers[11]);
    icmPrintf("R12: 0x%08x\n\n",    registers[12]);
    icmPrintf("SP:  0x%08x\t\t",    registers[13]);
    icmPrintf("LR:  0x%08x\n",      registers[14]);
    icmPrintf("PC:  0x%08x\n",      registers[15]);
    icmPrintf("PSR: 0x%08x\n\n",    registers[16]);
}

Bool readRegister(icmProcessorObject processor, char *regName, Uns32 &value) {
    if (regName[0] == 'P' && regName[1] == 'C') {
        value = (Uns32)processor.getPC();
        return True;
    } else {
        return processor.readReg(regName, &value);
    }
}

Bool writeRegister(icmProcessorObject processor, char *regName, Uns32 &newValue) {
    if (regName[0] == 'P' && regName[1] == 'C') {
        processor.setPC((Addr)newValue);
        return True;
    } else {
        return processor.writeReg(regName, &newValue);
    }
}


void addBreakpoint(Addr breakAddr) {
    if (breakAddr != 0x00 && bpCount != 255) {
        breakpoints[bpCount] = breakAddr;
        bpCount++;
    }
}

Addr simulateUntilBP(icmProcessorObject processor) {
    if (bpCount == 0) {
        icmPlatform::Instance()->simulate();
        return 0x00;
    }
    
    Addr currentPC = 0x00;
    while (processor.simulate(1) == ICM_SR_SCHED) {
        currentPC = processor.getPC();
        for (Uns8 u = 0; u < bpCount; u++) {
            if (currentPC == breakpoints[u]) {
                return currentPC;
            }
        }
    }
    
    return 0x00;
}
