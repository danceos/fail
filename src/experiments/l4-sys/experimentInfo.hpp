#ifndef __EXPERIMENT_INFO_HPP__
  #define __EXPERIMENT_INFO_HPP__

// the maximum number of bytes in a Bochs instruction
#define MAX_INSTR_BYTES 15

// the bounds of the program (space, instructions and time)
#define L4SYS_ADDRESS_SPACE		0x203d000
#define L4SYS_FUNC_ENTRY		0x10025ca
#define L4SYS_FUNC_EXIT			0x1002810
// kernel: 16300287, userland: 210785625
#define L4SYS_NUMINSTR			227085912
#define L4SYS_BOCHS_IPS			5000000

// several file names used
#define L4SYS_STATE_FOLDER		"l4sys.state"
#define L4SYS_INSTRUCTION_LIST	"ip.list"
#define L4SYS_ALU_INSTRUCTIONS	"alu.list"
#define L4SYS_CORRECT_OUTPUT	"golden.out"

// flags
#define HEADLESS_EXPERIMENT
//  0 - preparation complete
// >0 - next step to execute
#define PREPARATION_STEP		0

#endif // __EXPERIMENT_INFO_HPP__
