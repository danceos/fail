#ifndef BEFORE_INSTRUCTION_HPP
#define BEFORE_INSTRUCTION_HPP

#include "icm/icmCpuManager.hpp"

using namespace icmCpuManager;

/**
 * Prints all the registers for an ARM Cortex M3 processor.
 * @param processor The processor (must be an ARM Cortex M3)
 */
void printArmCortexM3Registers(icmProcessorObject processor);

/**
 * Reads a register and stores its content in an Uns32 variable.
 * @param processor The processor for which to read the register
 * @param regName The name of the register as a string
 * @param value The address of where to store the register's content
 */
Bool readRegister(icmProcessorObject processor, char *regName, Uns32 &value);

/**
 * Writes the value given as an Uns32 variable to a register.
 * @param processor The processor for which to write the variable
 * @param regName The name of the register as a string
 * @param newValue The address of the new value to be written to the register
 */
Bool writeRegister(icmProcessorObject processor, char *regName, Uns32 &newValue);


/**
 * Adds a breakpoint for simulateUntilBreakpoint
 * @param breakAddr The address to be added as a breakpoint
 */
void addBreakpoint(Addr breakAddr);

/**
 * Simulates until breakpoint,
 * or the entire application file at once if no breakpoint was added.
 * Returns the address of the next instruction,
 * or zero if simulation did not stop at a breakpoint.
 * @param processor The processor to run the simulation on
 */
Addr simulateUntilBP(icmProcessorObject processor);

#endif