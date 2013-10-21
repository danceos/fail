#ifndef __L4SYS_EXPERIMENT_INFO_HPP__
  #define __L4SYS_EXPERIMENT_INFO_HPP__

// the maximum number of bytes in a Bochs instruction
#define MAX_INSTR_BYTES 15

// the bounds of the program (space, instructions and time)
// client
#define L4SYS_ADDRESS_SPACE 0x1fd4c000
// master
//#define L4SYS_ADDRESS_SPACE_TRACE 0x1fd4c000
// vcpu task
#define L4SYS_ADDRESS_SPACE_TRACE 0x1fd5a000
//#define L4SYS_ADDRESS_SPACE_TRACE L4SYS_ADDRESS_SPACE

// FUNC_{ENTRY,EXIT} specifies the range that needs to
// be captured to log program output properly
#define L4SYS_FUNC_ENTRY		0x60002052
//#define L4SYS_FUNC_ENTRY		0x10000
#define L4SYS_FUNC_EXIT			0x600020ae
// FILTER_{ENTRY,EXIT} specifies the range that injections
// should be carried out on (should be a subset of the above)
// and only works with FILTER_INSTRUCTIONS turned on
#define L4SYS_FILTER_ENTRY      0x60002067
#define L4SYS_FILTER_EXIT       0x60002075

// select instruction filtering
// XXX: this should be always on and the code should be
//      reworked to do the non-filtering work with an empty
//      filter list
#define L4SYS_FILTER_INSTRUCTIONS 1

// kernel: 2377547, userland: 79405472
#define L4SYS_NUMINSTR 141
#define L4SYS_TOTINSTR 154205
#define L4SYS_BOCHS_IPS			5000000

// several file names used
#define L4SYS_STATE_FOLDER		"l4sys.state"
#define L4SYS_INSTRUCTION_LIST	"ip.list"
#define L4SYS_ALU_INSTRUCTIONS	"alu.list"
#define L4SYS_CORRECT_OUTPUT	"golden.out"
#define L4SYS_FILTER            "filter.list"

// flags
//  0 - preparation complete
// >0 - next step to execute
#define PREPARATION_STEP 0

#endif // __L4SYS_EXPERIMENT_INFO_HPP__
