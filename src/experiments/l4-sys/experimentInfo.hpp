#ifndef __L4SYS_EXPERIMENT_INFO_HPP__
  #define __L4SYS_EXPERIMENT_INFO_HPP__

// the maximum number of bytes in a Bochs instruction
#define MAX_INSTR_BYTES 15

// the bounds of the program (space, instructions and time)
#define L4SYS_ADDRESS_SPACE		0x1fd6e000
#define L4SYS_FUNC_ENTRY		0x01000200
#define L4SYS_FUNC_EXIT			0x01000245

// Instruction filtering: Allows to specify a range within
// which to perform injection experiments. If in doubt, set
// to the whole user-addressable area (0, 0xC0000000)
#define L4SYS_FILTER_INSTRUCTIONS 1
#define L4SYS_ADDRESS_LBOUND 0
#define L4SYS_ADDRESS_UBOUND 0xC0000000

// kernel: 2377547, userland: 79405472
#define L4SYS_NUMINSTR 3281
#define L4SYS_TOTINSTR 228218
#define L4SYS_BOCHS_IPS			5000000

// several file names used
#define L4SYS_STATE_FOLDER		"l4sys.state"
#define L4SYS_INSTRUCTION_LIST	"ip.list"
#define L4SYS_ALU_INSTRUCTIONS	"alu.list"
#define L4SYS_CORRECT_OUTPUT	"golden.out"

// flags
//  0 - preparation complete
// >0 - next step to execute
#define PREPARATION_STEP 0

#endif // __L4SYS_EXPERIMENT_INFO_HPP__
