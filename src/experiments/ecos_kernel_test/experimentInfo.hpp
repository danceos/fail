#pragma once

// FIXME autogenerate this

#if 1 // with ECC

// the task function's entry address:
// nm -C thread1 | fgrep cyg_start
#define ECOS_FUNC_ENTRY		0x00003cc0
// empty function that is called explicitly when the experiment finished
// nm -C thread1 | fgrep cyg_test_exit
#define ECOS_FUNC_FINISH		0x000058dc
// nm -C thread1 | fgrep "cyg_test_output"
#define ECOS_FUNC_TEST_OUTPUT	0x000058e4

// nm -C thread1 | grep "_[se]text"
#define ECOS_TEXT_START		0x00003000
#define ECOS_TEXT_END		0x000092ce

// number of instructions the target executes under non-error conditions from ENTRY to DONE:
// (result of experiment's step #2)
#define ECOS_NUMINSTR		12390
// number of instructions that are executed additionally for error corrections
// (this is a rough guess ... TODO)
#define ECOS_RECOVERYINSTR	0x2000
// the variable that's increased if ECC corrects an error:
// nm -C thread1|fgrep errors_corrected
#define ECOS_ERROR_CORRECTED	0x0010adec //FIXME TODO XXX

#else // without ECC

#define COOL_ECC_FUNC_ENTRY		0x00200a90
#define COOL_ECC_CALCDONE		0x00200ab7
#define COOL_ECC_NUMINSTR		97
#define COOL_ECC_OBJUNDERTEST		0x0021263c
#define COOL_ECC_OBJUNDERTEST_SIZE	10
#define COOL_ECC_ERROR_CORRECTED	0x002127b0 // dummy

#endif

