#ifndef __EXPERIMENT_INFO_HPP__
#define __EXPERIMENT_INFO_HPP__

// FIXME autogenerate this

#if 1 // with ECC

// the task function's entry address:
// nm -C ecc.elf|fgrep main
#define OOSTUBS_FUNC_ENTRY		0x00103f2c 
// empty function that is called explicitly when the experiment finished
// nm -C ecc.elf|fgrep "finished()"
#define OOSTUBS_FUNC_FINISH		0x001093f0
// function executing HLT with no chance for further progress (after panic())
// nm -C ecc.elf|fgrep cpu_halt
#define OOSTUBS_FUNC_CPU_HALT	0x00100987
// number of instructions the target executes under non-error conditions from ENTRY to DONE:
// (result of experiment's step #2)
#define OOSTUBS_NUMINSTR		0x4a3401
// number of instructions that are executed additionally for error corrections
// (this is a rough guess ... TODO)
#define OOSTUBS_RECOVERYINSTR	0x2000
// the ECC protected object's address:
// nm -C ecc.elf|fgrep objectUnderTest
#define COOL_ECC_OBJUNDERTEST		0x002127a4 //FIXME
// the ECC protected object's payload size:
// (we know that from the object's definition and usual memory layout)
#define COOL_ECC_OBJUNDERTEST_SIZE	10 //FIXME
// the variable that's increased if ECC corrects an error:
// nm -C ecc.elf|fgrep errors_corrected
#define OOSTUBS_ERROR_CORRECTED	0x0010e3a4
//
// nm -C ecc.elf|fgrep results
#define OOSTUBS_RESULTS_ADDR	0x0010d794
#define OOSTUBS_RESULTS_BYTES	12
#define OOSTUBS_RESULT0			0xab3566a9
#define OOSTUBS_RESULT1			0x44889112
#define OOSTUBS_RESULT2			0x10420844

#else // without ECC

#define COOL_ECC_FUNC_ENTRY		0x00200a90
#define COOL_ECC_CALCDONE		0x00200ab7
#define COOL_ECC_NUMINSTR		97
#define COOL_ECC_OBJUNDERTEST		0x0021263c
#define COOL_ECC_OBJUNDERTEST_SIZE	10
#define COOL_ECC_ERROR_CORRECTED	0x002127b0 // dummy

#endif

#endif
