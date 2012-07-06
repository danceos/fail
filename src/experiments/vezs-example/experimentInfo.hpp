#ifndef __EXPERIMENT_INFO_HPP__
  #define __EXPERIMENT_INFO_HPP__

// FIXME autogenerate this


// the task function's entry address:
// nm -C ecc.elf|fgrep main
#define OOSTUBS_FUNC_ENTRY		0x001009d0
// empty function that is called explicitly when the experiment finished
// nm -C ecc.elf|fgrep "finished()"
#define OOSTUBS_FUNC_FINISH		0x001009d6
// function executing HLT with no chance for further progress (after panic())
// nm -C ecc.elf|fgrep cpu_halt
#define OOSTUBS_FUNC_CPU_HALT	0x001009f7

// nm -C ecc.elf | fgrep "_TEXT_"
#define OOSTUBS_TEXT_START		0x00100000
#define OOSTUBS_TEXT_END		0x00100a1b

#define OOSTUBS_NUMINSTR		5


#endif // __EXPERIMENT_INFO_HPP__
