#ifndef __EXPERIMENT_INFO_HPP__
#define __EXPERIMENT_INFO_HPP__

#define COOL_FAULTSPACE_PRUNING 0

// FIXME autogenerate this

#if 1 // with ECC

// the task function's entry address:
// nm -C ecc.elf|fgrep Alpha::functionTaskTask0
#define COOL_ECC_FUNC_ENTRY		0x00200b32
// one of the last instructions before the task calls printf:
// (objdump -Cd ecc.elf|less)
#define COOL_ECC_CALCDONE		0x00200bdf
// number of instructions the target executes under non-error conditions from ENTRY to CALCDONE:
// (result of experiment's step #2)
#define COOL_ECC_NUMINSTR		1995
// the ECC protected object's address:
// nm -C ecc.elf|fgrep objectUnderTest
#define COOL_ECC_OBJUNDERTEST		0x002127a4
// the ECC protected object's payload size:
// (we know that from the object's definition and usual memory layout)
#define COOL_ECC_OBJUNDERTEST_SIZE	10
// the variable that's increased if ECC corrects an error:
// nm -C ecc.elf|fgrep error_corrected
#define COOL_ECC_ERROR_CORRECTED	0x002127b0

#else // without ECC

#define COOL_ECC_FUNC_ENTRY		0x00200a90
#define COOL_ECC_CALCDONE		0x00200ab7
#define COOL_ECC_NUMINSTR		97
#define COOL_ECC_OBJUNDERTEST		0x0021263c
#define COOL_ECC_OBJUNDERTEST_SIZE	10
#define COOL_ECC_ERROR_CORRECTED	0x002127b0 // dummy

#endif

#endif // __EXPERIMENT_INFO_HPP__
