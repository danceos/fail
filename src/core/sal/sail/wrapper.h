#ifndef SAILWRAPPER_H
#define SAILWRAPPER_H

#include <stdint.h>
#include <stdbool.h>
#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
 * Wrapper function for SailController::startup().
 * @param argc main()'s argument counter
 * @param argv main()'s argument value vector
 */

void fail_startup(int argc, char** argv);
/**
 * Wrapper function for SailController::onGuestSystem(). Has to be executed
 * after a guest system communication call.
 * @param data the "message" from the guest system
 * @param port the port used for communications
 */
int fail_onGuestSystem(char data, unsigned port);
/**
 * Wrapper function for SailController::onMemoryAccess() triggered by a memory
 * read. Has to be executed before a memory read operation.
 * The memory read event is surpressed, if the simulator is currently fetching
 * an instruction from memory.
 * @param addr the accessed memory address
 * @param data the accessed data
 * @param mem_type the type of memory which was accessed, must be convertible to memory_type_t.
 * @param instrPtr the address of the instruction causing the memory
 *        access
 */
int fail_onMemoryRead(uint64_t addr, mpz_t data, mpz_t size, uint64_t mem_type, uint64_t instrPtr);
/**
 * Wrapper function for SailController::onMemoryAccess(). Has to be executed
 * after a memory write operation.
 * @param addr the accessed memory address
 * @param data the accessed data
 * @param mem_type the type of memory which was accessed, must be convertible to memory_type_t.
 * @param instrPtr the address of the instruction causing the memory
 *        access
 */
int fail_onMemoryWrite(uint64_t addr, mpz_t data, mpz_t size, uint64_t mem_type, uint64_t instrPtr);

/**
 * Wrapper function for SailController::onBreakpoint().
 * Has to be execute before the instruction is fetched, and before any hooks are executed
 * through fail_executeHooks() since this may add additional save/restore hooks.
 * @param instrPtr the instruction pointer which will be executed
 */
int fail_willExecuteInstruction(uint64_t instrPtr);

/**
 * Wrapper function to check & execute any pending FAIL requests, such as save and restore.
 * Must be called _after_ onWillExecuteInstruction but _before_ any processing of the instruction begins.
 */
int fail_executeRequests();

/**
 * Wrapper function for SailController::onJump.
 * Has to be executed after an instruction has been fully processed, but before the next instruction is prepared
 * XXX: this is untested!
 * @param instrPtr The instruction pointer which was executed.
 * @param instr The instruction which was executed.
 */
int fail_didExecuteInstruction(uint64_t instrPtr, uint64_t instr);
/**
 * Wrapper function for SailController::onTrap(). Has to be executed before
 * the trap handling is executed.
 * @param trapNum the trap-type id
 */
int fail_onTrap(unsigned trapNum);
/**
 * Wrapper function for SailController::onInterrupt(). Has to be executed before
 * the interrupt handling is executed.
 * @param interruptNum the interrupt-type id
 * @param nmi nmi-value from guest-system
 */
int fail_onInterrupt(unsigned interruptNum, bool nmi);
/**
 * Wrapper function for ConcreteCPU::isSuppressedInterrupt(). Has the be executed
 * before the interrupt handling is executed.
 * @param interruptNum the interrupt-type id
 * @return \c true if the interrupt is suppressed, \c false otherwise
 */
bool fail_isSuppressedInterrupt(unsigned interruptNum);
/**
 * Wrapper function for SailController::setInstructionFetch(). Set to true before
 * an instruction fetch from memory and to false after the instrucion was fetched.
 */
int fail_setInstructionFetch(bool instructionFetch);

#ifdef __cplusplus
}  // end extern "C"
#endif

#endif /* SAILWRAPPER_H */
