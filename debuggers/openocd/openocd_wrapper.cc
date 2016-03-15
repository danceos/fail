#include "openocd_wrapper.hpp"

extern "C" {
	#include "config.h"

	#include "src/helper/system.h"
	#include "src/helper/log.h"
	#include "src/helper/time_support.h"
	#include "src/helper/command.h"
	#include "src/helper/configuration.h"
	#include "src/helper/util.h"
	#include "src/helper/ioutil.h"

	#include "src/jtag/jtag.h"

	#include "src/target/algorithm.h"
	#include "src/target/breakpoints.h"
	#include "src/target/register.h"
	#include "src/target/target_type.h"
	#include "src/target/target.h"
	#include "src/target/cortex_a.h"
	#include "src/target/arm_adi_v5.h"
	#include "src/target/armv7a.h"
	#include "src/target/arm_opcodes.h"

	#include "jimtcl/jim.h"

	extern struct command_context *setup_command_handler(Jim_Interp *interp);

	#include "opcode_parser/arm-opcode.h"
}

#include <cassert>
#include <ostream>
#include <vector>
#include <sys/time.h>
#include "config/VariantConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/SALConfig.hpp"
#include "sal/arm/ArmArchitecture.hpp"
#include "util/ElfReader.hpp"

#include "util/Logger.hpp"

using std::endl;
using std::cout;
using std::cerr;
using std::hex;
using std::dec;

/*
 * Flag for finishing main loop execution.
 */
static bool oocdw_exection_finished = false;
static int oocdw_exCode;

/*
 * Initially set target struct pointers for use in
 * multiple functions.
 */
static struct target 			*target_a9;
static struct arm 				*arm_a9;
static struct target 			*target_m3;
static struct command_context 	*cmd_ctx;

/*
 * As the normal loop execution is
 * resume - wait for halt - trigger events and set next halt condition(s),
 * single-stepping is different. It is accomplished by setting this state
 * variable.
 */
static bool single_step_requested = false;

static bool single_step_watch_memory = false;

/*
 * Variables for monitoring BP/WP resources
 * Reset at reboot
 */
static uint8_t free_watchpoints = 4;
static uint8_t free_breakpoints = 6;

/*
 * Elf reader for reading symbols in startup-code
 */
static fail::ElfReader elf_reader;

static uint32_t sym_GenericTrapHandler, sym_MmuFaultBreakpointHook,
				sym_SafetyLoopBegin, sym_SafetyLoopEnd,
				sym_addr_PageTableFirstLevel, sym_addr_PageTableSecondLevel;

/*
 * MMU related structures
 *
 * Representation of current MMU configuration as follows:
 *
 *  - In every configuration, we know the location of all descriptors in
 *    pandaboard memory, because memory for all possible descriptors is
 *    initially allocated.
 *
 *  - 1st lvl page table in mmu_conf (4096 entries) initially configured as
 *    linear mapping solely with section descriptors.
 *
 *  - If a descriptor gets unmapped, the bool value mapped is set to false.
 *
 *  - If a descriptor is transformed to page-descriptor, 256 2nd lvl descriptors
 *    are created (dynamic memory), which are described as a bool array (mapped).
 *    In this case, the mapped attribute is of no more function.
 *
 *  - In case we had a hit, the pandaboard will map the according descriptor, so
 *    the memory access can be executed after the experiment handled the
 *    MemoryAccess event.
 *    Afterwards, to be able to recognize subsequent memory accesses, covered by
 *    the the specific descriptor, we have to unmap it again. This is signaled
 *    by mmu_recovery_needed and the according memory address of the descriptor
 *    is stored in mmu_recovery_address.
 *    Also to be able to recognize the special breakpoint for MMU recovery, we
 *    store its description here separately in mmu_recovery_bp.
 *
 *  - Change requests for the MMU configuration are added to the vectors
 *    mmu_watch_add_waiting_list and mmu_watch_del_waiting_list.
 *
 *  - The currently watched memory ranges are stored in mmu_watch.
 */

enum descriptor_e {
	DESCRIPTOR_SECTION,
	DESCRIPTOR_PAGE,
};

struct mmu_first_lvl_descr {
	enum descriptor_e type;
	bool mapped;
	bool *second_lvl_mapped;
};

static bool mmu_recovery_needed = false;
static uint32_t mmu_recovery_address;
static struct halt_condition mmu_recovery_bp;

static struct mmu_first_lvl_descr mmu_conf [4096];

struct mem_range {
	uint32_t address;
	uint32_t width;
};

static std::vector<struct mem_range> mmu_watch_add_waiting_list;
static std::vector<struct mem_range> mmu_watch_del_waiting_list;
static std::vector<struct mem_range> mmu_watch;

/*
 * Timer related structures
 */
#define P_MAX_TIMERS 32
struct timer{
	bool inUse;					// Timer slot is in-use (currently registered).
	uint64_t timeToFire; 		// Time to fire next in microseconds
	struct timeval time_begin;	// Timer value at activation of timer
	bool active;				// 0=inactive, 1=active.
	p_timer_handler_t funct;	// A callback function for when the
								//   timer fires.
	void *this_ptr;				// The this-> pointer for C++ callbacks
								//   has to be stored as well.
	bool freezed;
#define PMaxTimerIDLen 32
	char id[PMaxTimerIDLen]; 	// String ID of timer.
};

static struct timer timers[P_MAX_TIMERS];

fail::Logger LOG("OpenOCD", false);

/** FORWARD DECLARATIONS **/

// As timers are implemented in this wrapper, they must be updated frequently
// and they must be frozen on target system stop and unfrozen on target system
// continuing
static void update_timers();
static void freeze_timers();
static void unfreeze_timers();

// Openocd does not tell us, which watchpoint hit, so we must recognize this
// We have the alternatives to do it with
static bool getHaltingWatchpoint(struct halt_condition *hc);
static bool getCurrentMemAccess(struct halt_condition *access);

// If we halt on any instruction, this function decodes it and tells us, if
// it executes a memory access and as the case may be its form
static bool getMemAccess(uint32_t opcode, struct halt_condition *access);

// Get pc of current halt
static uint32_t getCurrentPC();

// mapping of reg-number to openocd representation of register
static struct reg *get_reg_by_number(unsigned int num);

// read special registers
static void read_dpm_register(uint32_t reg_num, uint32_t *data);

// Readout symbols from ELF binary
static void init_symbols();

// The cycle counter of the pandaboard may be frozen and unfrozen, but its
// accuracy will stay low because dbg-operations will increment it
void freeze_cycle_counter();
void unfreeze_cycle_counter();

// On change of MMU configuration we invalidate the whole TLB
static void invalidate_tlb();

// Update the MMU watch according to the change requests in
// mmu_watch_add_waiting_list and mmu_watch_del_waiting_list.
// For each list we have an own function which will tell, if an invalidation
// of the TLB is necessary (return value)
static void update_mmu_watch();
static bool update_mmu_watch_add();
static bool update_mmu_watch_remove();

// As we only want to configure changes in the configuration, this function
// compares the diff of the current configuration and of the configuration
// afterwards
static void get_range_diff(std::vector<struct mem_range> &before,
		std::vector<struct mem_range> &after, std::vector<struct mem_range> &diff);

// Generic unmap for a 1st or 2nd lvl-descriptor with given address
static void unmap_page_section(uint32_t address);

// This function searches a given memory range in the mmu_watch vector
static std::vector<struct mem_range>::iterator
find_mem_range_in_vector(struct mem_range &elem, std::vector<struct mem_range> &vec);

// Aggregation of MMU configuration writes
static void execute_buffered_writes(std::vector<std::pair<uint32_t, uint32_t> > &buf);

// Merging of memory ranges of current MMU watch configuration
static void merge_mem_ranges (std::vector<struct mem_range> &in_out);

// Custom search predicate for ascending addresses in memory ranges
static bool compareByAddress(const struct mem_range &a, const struct mem_range &b);

// Align memory ranges to 4k-boundaries
static void force_page_alignment_mem_range (std::vector<struct mem_range> &in,
											std::vector<struct mem_range> &out);

/*
 * Main entry and main loop.
 *
 * States:
 * 0) Init
 * 1) Reset
 * 2) Wait for navigational commands from experiment
 * 3) (Execute single-steps if requested)
 * 3) Resume execution
 * 4) Wait for halt
 * 5) Fire event
 * 6) update timers
 * 7) goto 2
 */
int main(int argc, char *argv[])
{
	int ret;

	/* === INITIALIZATION === */
	// This is partly a copy of openocd_main (openocd.c) for startup

	// Redirect output to logfile
	FILE *file = fopen("oocd.log", "w");
	set_log_output(NULL, file);

	/* initialize commandline interface */
	cmd_ctx = setup_command_handler(NULL);

	if (util_init(cmd_ctx) != ERROR_OK)
		return 1;//EXIT_FAILURE;

	if (ioutil_init(cmd_ctx) != ERROR_OK)
		return 1;//EXIT_FAILURE;

	command_context_mode(cmd_ctx, COMMAND_CONFIG);
	command_set_output_handler(cmd_ctx, configuration_output_handler, NULL);

	if (parse_cmdline_args(cmd_ctx, argc, argv) != ERROR_OK)
		return EXIT_FAILURE;

	// set path to configuration file
	add_script_search_dir(OOCD_CONF_FILES_PATH);
	add_config_command("script "OOCD_CONF_FILE_PATH);

	ret = parse_config_file(cmd_ctx);
	if (ret != ERROR_OK) {
		LOG << "Error in openocd configuration!\nFor more detailed information refer to oocd.log" << endl;
		return EXIT_FAILURE;
	}

	ret = command_run_line(cmd_ctx, (char*)"init");
	if (ret != ERROR_OK) {
		LOG << "Error in openocd initialization!\nFor more detailed information refer to oocd.log" << endl;
		return EXIT_FAILURE;
	}

	// Find target cortex_a9 core 0
	target_a9 = get_target("omap4460.cpu");
	if (!target_a9) {
		LOG << "FATAL ERROR: Target omap4460.cpu not found!" << endl;
		return 1;
	}

	// Find target cortex_m3 core 0
	target_m3 = get_target("omap4460.m30");
	if (!target_m3) {
		LOG << "FATAL ERROR: Target omap4460.m30 not found!" << endl;
		return 1;
	}

	// Needed to read current pc
	arm_a9 = target_to_arm(target_a9);

	// Disable jtag polling
	jtag_poll_set_enabled (false);

	// Init timers
	for (int i=0; i<P_MAX_TIMERS; i++) {
		timers[i].inUse = false;
	}

	init_symbols();

	LOG << "OpenOCD 0.7.0 for FAIL* and Pandaboard initialized" << endl;

	/* === INITIALIZATION COMPLETE => MAIN LOOP === */

	/*
	 * Initial reboot
	 */
	oocdw_reboot();

	/*
	 * Switch to experiment for navigational instructions
	 */
	fail::simulator.startup();

	while (!oocdw_exection_finished) {

		/*
		 * At this point the device should always be in halted state
		 * So single-stepping is done loop-wise, here.
		 * For having functional timers anyways, we need to update
		 * in every loop iteration
		 */
		uint32_t trace_count = 1;
		while (single_step_requested) {
			int repeat = 5;
			int retval;

			freeze_timers();
			update_mmu_watch();
			unfreeze_timers();

			unfreeze_cycle_counter();
			while ((retval = target_step(target_a9, 1, 0, 1)) && (repeat--)) {
				LOG << "ERROR: Single-step could not be executed at instruction "
						<< trace_count << ":" << hex << getCurrentPC() << dec
						<< " at t=" << oocdw_read_cycle_counter()
						<< ". ERRORCODE: "<< retval << ". Retrying..." << endl;
				usleep(1000*300);
			}
			freeze_cycle_counter();
			if (!repeat) {
				LOG << "FATAL ERROR: Single-step could not be executed. "
						"Terminating..." << endl;
				exit(-1);
			}
			/*
			 * Because this is a micro main-loop, we need to update
			 * timers. This loop can be executed several times (e.g.
			 * in tracing mode), so timers would be neglected otherwise.
			 */
			update_timers();
			uint32_t pc = getCurrentPC();

			/*
			 * Reset request flag. Needs to be done before calling
			 * onBreakpoint, because otherwise it would reset a
			 * newly activated request after coroutine switch.
			 */
			single_step_requested = false;

			freeze_timers();
			fail::simulator.onBreakpoint(NULL, pc, fail::ANY_ADDR);
			unfreeze_timers();

			/*
			 * Get current memory access(es)
			 */
			if (single_step_watch_memory) {
				struct halt_condition mem_access;
				if (getCurrentMemAccess(&mem_access)) {
					freeze_timers();
					fail::simulator.onMemoryAccess(NULL,
							mem_access.address,
							mem_access.addr_len,
							mem_access.type == HALT_TYPE_WP_WRITE,
							pc);
					unfreeze_timers();
				}
			}

			trace_count++;
		}

		// Shortcut loop exit for tracing
		if (oocdw_exection_finished) {
			break;
		}

		/*
		 * In the following loop stage it is assumed, that the target is
		 * running, so execution needs to be resumed, if the target is
		 * halted.
		 * Before we resume, any potential MMU modifications must be
		 * programmed into the device
		 * As this programming should be done offline, we need to freeze the
		 * timers for the configuration period.
		 */
		freeze_timers();
		update_mmu_watch();
		unfreeze_timers();

		if (target_a9->state == TARGET_HALTED) {
			/*
			 * Execute single-step if horizontal hop was detected
			 */

			bool horizontal_step = false;

			// Compare current memory access event with set watchpoint
			struct halt_condition mem_access;
			if (getCurrentMemAccess(&mem_access)) {

				struct watchpoint *watchpoint = target_a9->watchpoints;

				// WPT_READ = 0, WPT_WRITE = 1, WPT_ACCESS = 2
				while (watchpoint) {
					if (((mem_access.type == HALT_TYPE_WP_READ)
						&& (watchpoint->rw == WPT_READ))
						|| ((mem_access.type == HALT_TYPE_WP_WRITE)
						&& (watchpoint->rw == WPT_WRITE))
						||	((mem_access.type == HALT_TYPE_WP_READWRITE)
						&& (watchpoint->rw == WPT_ACCESS)) ) {

						if (((mem_access.address < watchpoint->address)
								&& (mem_access.address + mem_access.addr_len >= watchpoint->address))
								|| ((mem_access.address >= watchpoint->address)
								&& (watchpoint->address + watchpoint->length >= mem_access.address))) {
							horizontal_step = true;
						}
					}
					watchpoint = watchpoint->next;
				}
			}

			// Compare current pc with set breakpoint (potential horizontal hop)
			uint32_t pc = getCurrentPC();
			struct breakpoint *breakpoint = target_a9->breakpoints;

			while (breakpoint) {
				if (((pc <= breakpoint->address) && ((pc + 4) >= breakpoint->address)) ||
						((breakpoint->address <= pc) && ((breakpoint->address + 4) >= pc))) {
					horizontal_step = true;
				}
				breakpoint = breakpoint->next;
			}

			if (horizontal_step) {
				LOG << "horizontal -> singlestep" << endl;
				unfreeze_cycle_counter();
				if (target_step(target_a9, 1, 0, 1)) {
					LOG << "FATAL ERROR: Single-step could not be executed!" << endl;
					exit (-1);
				}
				freeze_cycle_counter();
			}


			LOG << "Resume" << endl;
			unfreeze_cycle_counter();
			if (target_resume(target_a9, 1, 0, 1, 1)) {
				LOG << "FATAL ERROR: Target could not be resumed!" << endl;
				exit(-1);
			}
		}

		// Wait for target to halt
		while (target_a9->state != TARGET_HALTED) {
			// Polling needs to be done to detect target halt state changes.
			if (target_poll(target_a9)) {
				LOG << "FATAL ERROR: Error polling after resume!" << endl;
				exit(-1);
			}
			// Update timers, so waiting can be aborted
			update_timers();
		}

		// Check for halt and trigger event accordingly
		if (target_a9->state == TARGET_HALTED) {
			freeze_cycle_counter();

			uint32_t pc = getCurrentPC();

			// After coroutine-switch, dbg-reason might change, so it must
			// be stored
			enum target_debug_reason current_dr = target_a9->debug_reason;
			switch (current_dr) {
			case DBG_REASON_WPTANDBKPT:
				/* Fall through for handling of BP and WP*/
			case DBG_REASON_BREAKPOINT:
				if (mmu_recovery_needed) {
					// Check for correct pc
					if (pc != mmu_recovery_bp.address) {
						LOG << "FATAL ERROR: Something went wrong while handling mmu event" <<
								hex << pc << " " << mmu_recovery_bp.address << endl;
						exit(-1);
					}

					// Step, so mem access can be done
					unfreeze_cycle_counter();
					if (target_step(target_a9, 1, 0, 1)) {
						LOG << "FATAL ERROR: Single-step could not be executed!" << endl;
						exit (-1);
					}
					freeze_cycle_counter();
					// Reset mapping at mmu_recovery_address
					// write into memory at mmu_recovery_address to map page/section
					unmap_page_section(mmu_recovery_address);

					invalidate_tlb();

					// remove this BP
					oocdw_delete_halt_condition(&mmu_recovery_bp);

					mmu_recovery_needed = false;
				} else if (pc == sym_MmuFaultBreakpointHook) {
					uint32_t lr_abt;
					oocdw_read_reg(fail::RI_LR_ABT, &lr_abt);

					// ToDo: GP register read faster? DFAR also available in reg 3
					uint32_t dfar;
					oocdw_read_reg(fail::RI_DFAR, &dfar);

					/*
					 * Analyze instruction to get access width. Also the
					 * base-address may differ from this in multiple
					 * data access instructions with decrement option.
					 * As some Register values have changed in the handler, we
					 * first must find the original values.
					 * This is easy for a static abort handler (pushed to stack),
					 * but must be adjusted, if handler is adjusted
					 */

					// ToDo: As this is going to be executed very often, it
					// should be done on the target system

					uint32_t opcode;
					oocdw_read_from_memory(lr_abt - 8, 4, 1, (uint8_t*)(&opcode));

					uint32_t sp_abt;
					oocdw_read_reg(fail::RI_SP_ABT, &sp_abt);

					arm_regs_t regs;

					// Read registers from abort stack, as they are now modified
					oocdw_read_from_memory(sp_abt, 4, 4, (uint8_t*)(&regs.r[2]));

					for (int i = 0; i < 16; i++) {
						if (i >=2 && i <=5) {
							continue;
						}
						oocdw_read_reg(i, &(regs.r[i]));
						regs.r[15] += 8;
					}

					// ABT SPSR is usr cpsr
					oocdw_read_reg(fail::RI_SPSR_ABT, &regs.cpsr);

					// if abort triggert in non-user-mode, spsr is approproate spsr
					switch ((regs.cpsr & 0b11111)) {
					case 0b10001:	// FIQ
						oocdw_read_reg(fail::RI_SPSR_FIQ, &regs.spsr);
						break;
					case 0b10010:	// IRQ
						oocdw_read_reg(fail::RI_SPSR_IRQ, &regs.spsr);
						break;
					case 0b10011:	// SVC
						oocdw_read_reg(fail::RI_SPSR_SVC, &regs.spsr);
						break;
					case 0b10110:	// MON
						oocdw_read_reg(fail::RI_SPSR_MON, &regs.spsr);
						break;
					case 0b11011:	// UNDEFINED
						oocdw_read_reg(fail::RI_SPSR_UND, &regs.spsr);
						break;
					}

					arm_instruction_t op;

					decode_instruction(opcode, &regs, &op);

					/*
					 * Set BP on the instruction, which caused the abort,
					 * singlestep and recover previous mmu state
					 */
					/*
					 * ToDO: This might be inefficient, because often the event
					 * trigger will cause a system reset and so this handling
					 * is done for nothing, but we must not do anything after
					 * triggering a event.
					 */
					mmu_recovery_bp.address = lr_abt - 8;
					mmu_recovery_bp.addr_len = 4;
					mmu_recovery_bp.type = HALT_TYPE_BP;

					oocdw_read_reg(4,&mmu_recovery_address);

					// ToDO: If all BPs used, delete and memorize any of these
					// as we will for sure first halt in the recovery_bp, we can
					// then recover it
					oocdw_set_halt_condition(&mmu_recovery_bp);

					mmu_recovery_needed = true;

					if (op.flags & (OP_FLAG_READ | OP_FLAG_WRITE)) {
						fail::simulator.onMemoryAccess(NULL, op.mem_addr,
								op.mem_size, op.flags & OP_FLAG_WRITE, lr_abt - 8);
					} else {
						/* As some memory accesses can't be decoded, because we
						 * are executing data, this is going to be handled as a trap.
						 */
						fail::simulator.onTrap(NULL, fail::ANY_TRAP);
					}

				} else if (pc == sym_GenericTrapHandler) {
					freeze_timers();
					fail::simulator.onTrap(NULL, fail::ANY_TRAP);
					unfreeze_timers();
				} else {
					freeze_timers();
					LOG << "BP-EVENT " << hex << pc << dec << endl;
					fail::simulator.onBreakpoint(NULL, pc, fail::ANY_ADDR);
					unfreeze_timers();
				}

				if (current_dr == DBG_REASON_BREAKPOINT) {
					break;
				}
				/* Potential fall through (Breakpoint and Watchpoint)*/
			case DBG_REASON_WATCHPOINT:
			{
				struct halt_condition halt;

				/*
				 * Which watchpoint hit?
				 *
				 * Alternatives:
				 *  - Decode and analyze current instruction (getCurrentMemAccess)
				 *    => Did not always work correctly
				 *       This problem might be resolved by the off by 8-byte
				 *       commit (Change-Id: I1cfcb84af2abae7971869d2ce29d602648e2f020)
				 *
				 *  - Find Watchpoint in watchpoint list of openocd (getHaltingWatchpoint)
				 *    => This does not work, if there is a number of watchpoints
				 *       unequal to 1 (So we might better switch back to
				 *       decode & analyze, if this now works)
				 */
#if 0
				if (!getCurrentMemAccess(&halt)) {
					LOG << "FATAL ERROR: Can't determine memory-access address of halt cause" << endl;
					exit(-1);
				}
#else
				if (!getHaltingWatchpoint(&halt)) {
					LOG << "FATAL ERROR: Can't determine memory-access address of halt cause" << endl;
					exit(-1);
				}
#endif

				// Get meta information and fire MemoryAccess event
				int iswrite;
				switch (halt.type) {
				case HALT_TYPE_WP_READ:
					iswrite = 0;
					break;
				case HALT_TYPE_WP_WRITE:
					iswrite = 1;
					break;
				case HALT_TYPE_WP_READWRITE:
					LOG << "We should not get a READWRITE as trigger of"
							"a watchpoint!" << endl;
					iswrite = 1;
					break;
				default:
					LOG << "FATAL ERROR: Can't determine memory-access type of halt cause" << endl;
					exit(-1);
					break;
				}

				freeze_timers();
				LOG << "WATCHPOINT EVENT ADDR: " << hex << halt.address << dec
						<< " LENGTH: " << halt.addr_len << " TYPE: "
						<< (iswrite?'W':'R') << endl;
				fail::simulator.onMemoryAccess(NULL, halt.address, halt.addr_len, iswrite, pc);
				unfreeze_timers();
			}
				break;
			case DBG_REASON_SINGLESTEP:
				LOG << "FATAL ERROR: Single-step is handled in previous loop phase" << endl;
				exit (-1);
				break;
			case DBG_REASON_DBGRQ:
				// Do nothing
				break;
			default:
				LOG << "FATAL ERROR: Target halted in unexpected cpu state "
					<< current_dr << endl;
				exit (-1);
				break;
			}
		}
		// Update timers. Granularity will be coarse, because this is done after
		// polling the device
		update_timers();
	}

	/* === FINISHING UP === */
	unregister_all_commands(cmd_ctx, NULL);

	/* free commandline interface */
	command_done(cmd_ctx);

	adapter_quit();

	fclose(file);

	LOG << "finished" << endl;

	exit(oocdw_exCode);
}

void oocdw_finish(int exCode)
{
	oocdw_exection_finished = true;
	oocdw_exCode = exCode;
}

void oocdw_set_halt_condition(struct halt_condition *hc)
{
	assert((target_a9->state == TARGET_HALTED) && "Target not halted");
	assert((hc != NULL) && "No halt condition defined");


	if (hc->type == HALT_TYPE_BP) {							/* BREAKPOINT */
		LOG << "Adding BP " << hex << hc->address << dec << ":" << hc->addr_len << endl;
		if (!free_breakpoints) {
			LOG << "FATAL ERROR: No free breakpoints left" << endl;
			exit(-1);
		}
		if (breakpoint_add(target_a9, hc->address,
					hc->addr_len, BKPT_HARD)) {
			LOG << "FATAL ERROR: Breakpoint could not be set" << endl;
			exit(-1);
		}
		free_breakpoints--;

	} else if (hc->type == HALT_TYPE_SINGLESTEP) {			/* SINGLE STEP */
		// Just set flag, so single step will be executed in according
		// main loop stage
		single_step_requested = true;
	} else if ((hc->type == HALT_TYPE_WP_READ) ||
				(hc->type == HALT_TYPE_WP_WRITE) ||
				(hc->type == HALT_TYPE_WP_READWRITE)) {		/* WATCHPOINT */

		// Use MMU for watching ?
		if ((hc->addr_len > 4) || !free_watchpoints) {
			LOG << "Setting up MMU to watch memory range " << hex << hc->address
					<< ":" << hc->addr_len << dec << endl;

			struct mem_range mr;
			mr.address = hc->address;
			mr.width = hc->addr_len;

			mmu_watch_add_waiting_list.push_back(mr);
			return;
		}

		// Special case for tracing: Watch ANY_ADDR => Decode memory accesses
		// while tracing
		if (hc->address == fail::ANY_ADDR) {
			single_step_watch_memory = true;
			return;
		}

		// Use hardware watchpoint
		LOG << "Adding WP " << hex << hc->address << dec << ":" << hc->addr_len
				<< ":" << ((hc->type == HALT_TYPE_WP_READ)? "R" :
				(hc->type == HALT_TYPE_WP_WRITE)? "W" : "R/W") << endl;

		enum watchpoint_rw rw = WPT_ACCESS;
		if (hc->type == HALT_TYPE_WP_READ) {
			rw = WPT_READ;
		} else if (hc->type == HALT_TYPE_WP_WRITE) {
			rw = WPT_WRITE;
		} else if (hc->type == HALT_TYPE_WP_READWRITE) {
			rw = WPT_ACCESS;
		}

		// No masking => value = 0; mask = 0xffffffff
		// length const 1, because functional correct and smallest
		// hit surface
		if (watchpoint_add(target_a9, hc->address, hc->addr_len,
					rw, 0x0, 0xffffffff)) {
			LOG << "FATAL ERROR: Watchpoint could not be set" << endl;
			exit(-1);
		}
		free_watchpoints--;
	} else {
		LOG << "FATAL ERROR: Could not determine halt condition type" << endl;
		exit(-1);
	}
}

void oocdw_delete_halt_condition(struct halt_condition *hc)
{
	assert((target_a9->state == TARGET_HALTED) && "Target not halted");
	assert((hc != NULL) && "No halt condition defined");

	// Remove halt condition from pandaboard
	if (hc->type == HALT_TYPE_BP) {							/* BREAKPOINT */
		LOG << "Removing BP " << hex << hc->address << dec << ":"
				<< hc->addr_len << endl;
		breakpoint_remove(target_a9, hc->address);
		free_breakpoints++;
	} else if (hc->type == HALT_TYPE_SINGLESTEP) {			/* SINGLE STEP */
		// Do nothing. Single-stepping event hits one time and
		// extinguishes itself automatically
	} else if ((hc->type == HALT_TYPE_WP_READWRITE) ||
			(hc->type == HALT_TYPE_WP_READ) ||
			(hc->type == HALT_TYPE_WP_WRITE)) {				/* WATCHPOINT */

		struct mem_range mr;
		mr.address = hc->address;
		mr.width = hc->addr_len;

		// Watched by MMU?
		std::vector<struct mem_range>::iterator search;
		search = find_mem_range_in_vector(mr, mmu_watch);

		if (search != mmu_watch.end()) { // YES => Remove
			LOG << "Setting up MMU to NO LONGER watch memory range " << hex
					<< hc->address << ":" << hc->addr_len << dec << endl;
			mmu_watch_del_waiting_list.push_back(mr);
			return;
		} else { // NO => ERROR
			LOG << "FATAL ERROR: Memory range " << hex << hc->address << ":"
					<< hc->addr_len << dec << " not watched by MMU, so can't be"
					" removed."<< endl;
			exit(-1);
		}

		// Special case for tracing: Watch ANY_ADDR => Decode memory accesses
		// while tracing
		if (hc->address == fail::ANY_ADDR) {
			single_step_watch_memory = false;
			return;
		}

		// Remove hardware watchpoint
		watchpoint_remove(target_a9, hc->address);
		free_watchpoints++;
		LOG << hex << "Removing WP " << hc->address << ":" << hc->addr_len
				<< ":" << ((hc->type == HALT_TYPE_WP_READ) ? "R" :
				(hc->type == HALT_TYPE_WP_WRITE) ? "W" : "R/W") << dec << endl;
	}
}

bool oocdw_halt_target(struct target *target)
{
	if (target_poll(target)) {
		LOG << "FATAL ERROR: Target polling failed for target "
				<< target->cmd_name << endl;
		return false;
	}

	if (target->state == TARGET_RESET) {
		LOG << "FATAL ERROR: Target " << target->cmd_name << " could not be "
				"halted, because in reset mode" << endl;
	}

	if (target_halt(target)) {
		LOG << "FATAL ERROR: Could could not halt target " << target->cmd_name
				<< endl;
		return false;
	}

	/*
	 * Wait for target to actually stop.
	 */
	long long then = timeval_ms();
	if (target_poll(target)) {
		LOG << "FATAL ERROR: Target polling failed for target "
				<< target->cmd_name << endl;
		return false;
	}
	while (target->state != TARGET_HALTED) {
		if (target_poll(target)) {
			LOG << "FATAL ERROR: Target polling failed for target "
					<< target->cmd_name << endl;
			return false;
		}
		if (timeval_ms() > then + 1000) {
			LOG << "FATAL ERROR: Timeout waiting for halt of target "
					<< target->cmd_name << endl;
			return false;
		}
	}
	return true;
}

/*
 * As "reset halt" and "reset init" fail irregularly on the pandaboard, resulting in
 * a device freeze, from which only a manual reset can recover the state, a different
 * approach is used to reset the device and navigate to main entry.
 * A "reset run" command is executed, which does not cause a device freeze.
 * Afterwards the pandaboard is immediately halted, so the halt normally triggers
 * in the initialization phase. Afterwards a breakpoint is set on the target
 * instruction, so the device is set for navigation to the target dynamic instructions.
 *
 * 1) The indirection of navigating to the main entry is needed, because
 *    the first halt condition might be a watchpoint, which could also
 *    trigger in the binary loading phase.
 * 2) Because it is not guaranteed, that the immediate halt is triggered
 *    before main entry, a "safety loop" is executed before the main
 *    entry. It is therefore necessary to first navigate into this loop
 *    and then jump over it by modifying the program counter.
 */
void oocdw_reboot()
{
	int retval, fail_counter = 1;
	bool reboot_success = false;

	for (int i=0; i<P_MAX_TIMERS; i++) {
		timers[i].freezed = false;
		timers[i].active = false;
	}

	while (!reboot_success) {
		LOG << "Rebooting device" << endl;
		reboot_success = true;

		// If target is not halted, reset will result in freeze
		if (target_a9->state != TARGET_HALTED) {
			if (!oocdw_halt_target(target_a9)) {
				reboot_success = false;
				if (fail_counter++ > 4) {
					LOG << "FATAL ERROR: Rebooting not possible" << endl;
					exit(-1);
				}
				usleep(100*1000);
				continue;
			}
		}

		/*
		 * Clear all halt conditions
		 */
		breakpoint_clear_target(target_a9);
		watchpoint_clear_target(target_a9);

		/*
		 * Reset state structures
		 */
		free_watchpoints = 4;
		free_breakpoints = 6;
		mmu_recovery_needed = false;
		mmu_watch.clear();
		mmu_watch_add_waiting_list.clear();
		mmu_watch_del_waiting_list.clear();


		// Standard configuration: Sections only, all mapped
		for (int i = 0; i < 4096; i++) {
			if (mmu_conf[i].type == DESCRIPTOR_PAGE) {
				delete[] mmu_conf[i].second_lvl_mapped;
				mmu_conf[i].type = DESCRIPTOR_SECTION;
			}
			mmu_conf[i].mapped = true;
		}

		/*
		 * The actual reset command executed by OpenOCD jimtcl-engine
		 */
		retval = Jim_Eval(cmd_ctx->interp, "reset");

		retval = target_call_timer_callbacks_now();
		if (retval) {
			LOG << "target_call_timer_callbacks_now() Error" << endl;
			exit(-1);
		}

		struct target *target;
		for (target = all_targets; target; target = target->next)
			target->type->check_reset(target);

		usleep(750*1000);

		// Immediate halt after reset.
		if (!oocdw_halt_target(target_a9)) {
			reboot_success = false;
			if (fail_counter++ > 4) {
				LOG << "FATAL ERROR: Rebooting not possible" << endl;
				exit(-1);
			}
			usleep(100*1000);
			continue;
		}

		// Check if halted in safety loop
		uint32_t pc = getCurrentPC();

		if (pc < sym_SafetyLoopBegin || pc >= sym_SafetyLoopEnd) {
			LOG << "NOT IN LOOP!!! PC: " << hex << pc << dec << std::endl;

			// BP on entering main
			struct halt_condition hc;

			hc.address = sym_SafetyLoopEnd - 4;
			hc.addr_len = 4;
			hc.type = HALT_TYPE_BP;
			oocdw_set_halt_condition(&hc);

			if (target_resume(target_a9, 1, 0, 1, 1)) {
				LOG << "FATAL ERROR: Target could not be resumed" << endl;
				exit(-1);
			}

			long long then;
			then = timeval_ms();
			while (target_a9->state != TARGET_HALTED) {
				int retval = target_poll(target_a9);
				if (retval != ERROR_OK) {
					LOG << "FATAL ERROR: Target polling failed" << endl;
					exit(-1);
				}
				if (timeval_ms() > then + 2000) {
					LOG << "Error: Timeout waiting for main entry" << endl;
					reboot_success = false;
					if (fail_counter++ > 4) {
						LOG << "FATAL ERROR: Rebooting not possible" << endl;
						exit(-1);
					}
					oocdw_halt_target(target_a9);
					break;
				}
			}
			// Remove temporary
			oocdw_delete_halt_condition(&hc);
		} else {
			LOG << "Stopped in loop. PC: " << hex << pc << dec << std::endl;
		}

		if (reboot_success) {
			/*
			 * Jump over safety loop (set PC)
			 * This can be done, because we know that the following code does
			 * not use any register values, which could be set in the loop
			 */
			oocdw_write_reg(15, sym_SafetyLoopEnd);

			/*
			 * as we want to use the Cortex-M3 for access to ram, it should
			 * always be halted, so halt it after reset.
			 */
			if (target_m3->state != TARGET_HALTED) {
				if (!oocdw_halt_target(target_m3)) {
					exit(-1);
				}
			}
		}

		// Initially set BP for generic traps
		struct halt_condition hc;
		hc.type = HALT_TYPE_BP;
		hc.address = sym_GenericTrapHandler;
		hc.addr_len = 4;
		oocdw_set_halt_condition(&hc);

		// Initially set BP for mmu abort handler
		hc.type = HALT_TYPE_BP;
		hc.address = sym_MmuFaultBreakpointHook;
		hc.addr_len = 4;
		oocdw_set_halt_condition(&hc);
	}
}

/*
 * Register access as in target.c: COMMAND_HANDLER(handle_reg_command)
 */
static struct reg *get_reg_by_number(unsigned int num)
{
	struct reg *reg = NULL;

	struct reg_cache *cache = target_a9->reg_cache;
	unsigned int count = 0;
	while (cache) {
		unsigned i;
		for (i = 0; i < cache->num_regs; i++) {
			if (count++ == num) {
				reg = &cache->reg_list[i];
				break;
			}
		}
		if (reg) {
			break;
		}
		cache = cache->next;
	}

	return reg;
}

static void read_dpm_register(uint32_t reg_num, uint32_t *data)
{
	struct armv7a_common *armv7a = target_to_armv7a(target_a9);
	struct arm_dpm *dpm = armv7a->arm.dpm;
	int retval;
	retval = dpm->prepare(dpm);

	if (retval != ERROR_OK) {
		LOG << "Unable to prepare for reading dpm register" << endl;
	}

	if (reg_num == fail::RI_DFAR) {
		// MRC p15, 0, <Rt>, c6, c0, 0
		retval = dpm->instr_read_data_r0(dpm,
				ARMV4_5_MRC(15, 0, 0, 6, 0, 0),
				data);
	} else if (reg_num == fail::RI_DFSR) {
		// MRC p15, 0, <Rt>, c5, c0, 0
		retval = dpm->instr_read_data_r0(dpm,
				ARMV4_5_MRC(15, 0, 0, 5, 0, 0),
				data);
	} else {
		LOG << "FATAL ERROR: Reading dpm register with id " << reg_num
				<< " is not supported." << endl;
		exit (-1);
	}

	if (retval != ERROR_OK) {
		LOG << "Unable to read DFAR-Register" << endl;
	}

	dpm->finish(dpm);
}

static inline void read_processor_register(uint32_t reg_num, uint32_t *data)
{
	struct reg *reg;
	reg = get_reg_by_number(reg_num);

	if (reg->valid == 0) {
		reg->type->get(reg);
	}

	*data = *((uint32_t*)(reg->value));
}

void oocdw_read_reg(uint32_t reg_num, uint32_t *data)
{
	assert((target_a9->state == TARGET_HALTED) && "Target not halted");

	switch (reg_num) {
	case fail::RI_DFAR:
		/* fall through */
	case fail::RI_DFSR:
		read_dpm_register(reg_num, data);
		break;

	case fail::RI_R0:
		/* fall through */
	case fail::RI_R1:
		/* fall through */
	case fail::RI_R2:
		/* fall through */
	case fail::RI_R3:
		/* fall through */
	case fail::RI_R4:
		/* fall through */
	case fail::RI_R5:
		/* fall through */
	case fail::RI_R6:
		/* fall through */
	case fail::RI_R7:
		/* fall through */
	case fail::RI_R8:
		/* fall through */
	case fail::RI_R9:
		/* fall through */
	case fail::RI_R10:
		/* fall through */
	case fail::RI_R11:
		/* fall through */
	case fail::RI_R12:
		/* fall through */
	case fail::RI_R13:
		/* fall through */
	case fail::RI_R14:
		/* fall through */
	case fail::RI_R15:
		read_processor_register(reg_num, data);
		break;
	case fail::RI_R8_FIQ:
		read_processor_register(16, data);
		break;
	case fail::RI_R9_FIQ:
		read_processor_register(17, data);
		break;
	case fail::RI_R10_FIQ:
		read_processor_register(18, data);
		break;
	case fail::RI_R11_FIQ:
		read_processor_register(19, data);
		break;
	case fail::RI_R12_FIQ:
		read_processor_register(20, data);
		break;
	case fail::RI_SP_FIQ:
		read_processor_register(21, data);
		break;
	case fail::RI_LR_FIQ:
		read_processor_register(22, data);
		break;
	case fail::RI_SP_IRQ:
		read_processor_register(23, data);
		break;
	case fail::RI_LR_IRQ:
		read_processor_register(24, data);
		break;
	case fail::RI_SP_SVC:
		read_processor_register(25, data);
		break;
	case fail::RI_LR_SVC:
		read_processor_register(26, data);
		break;
	case fail::RI_SP_ABT:
		read_processor_register(27, data);
		break;
	case fail::RI_LR_ABT:
		read_processor_register(28, data);
		break;
	case fail::RI_SP_UND:
		read_processor_register(29, data);
		break;
	case fail::RI_LR_UND:
		read_processor_register(30, data);
		break;
	case fail::RI_CPSR:
		read_processor_register(31, data);
		break;
	case fail::RI_SPSR_FIQ:
		read_processor_register(32, data);
		break;
	case fail::RI_SPSR_IRQ:
		read_processor_register(33, data);
		break;
	case fail::RI_SPSR_SVC:
		read_processor_register(34, data);
		break;
	case fail::RI_SPSR_ABT:
		read_processor_register(35, data);
		break;
	case fail::RI_SPSR_UND:
		read_processor_register(36, data);
		break;
	case fail::RI_SP_MON:
		read_processor_register(37, data);
		break;
	case fail::RI_LR_MON:
		read_processor_register(38, data);
		break;
	case fail::RI_SPSR_MON:
		read_processor_register(39, data);
		break;
	default:
		LOG << "ERROR: Register with id " << reg_num << " unknown." << endl;
		break;
	}
}

// ToDo: Writing registers above R15 not yet needed, so writing was not
// implemented
void oocdw_write_reg(uint32_t reg_num, uint32_t data)
{
	assert((target_a9->state == TARGET_HALTED) && "Target not halted");

	switch (reg_num) {
	case fail::RI_R0:
		/* fall through */
	case fail::RI_R1:
		/* fall through */
	case fail::RI_R2:
		/* fall through */
	case fail::RI_R3:
		/* fall through */
	case fail::RI_R4:
		/* fall through */
	case fail::RI_R5:
		/* fall through */
	case fail::RI_R6:
		/* fall through */
	case fail::RI_R7:
		/* fall through */
	case fail::RI_R8:
		/* fall through */
	case fail::RI_R9:
		/* fall through */
	case fail::RI_R10:
		/* fall through */
	case fail::RI_R11:
		/* fall through */
	case fail::RI_R12:
		/* fall through */
	case fail::RI_R13:
		/* fall through */
	case fail::RI_R14:
		/* fall through */
	case fail::RI_R15:
	{
		struct reg *reg = get_reg_by_number(reg_num);

		reg->type->set(reg, (uint8_t*)(&data));
	}
		break;
	default:
		LOG << "ERROR: Register with id " << reg_num << " unknown for writing."
			<< endl;
		break;
	}
}


void oocdw_read_from_memory(uint32_t address, uint32_t chunk_size,
					uint32_t chunk_num, uint8_t *data)
{
	if (target_read_memory(target_a9, address, chunk_size, chunk_num, data)) {
		LOG << "FATAL ERROR: Reading from memory failed. Addr: " << hex
				<< address << dec << " chunk-size: " << chunk_size
				<< " chunk_num: " << chunk_num << endl;
		exit(-1);
	}
}

void oocdw_write_to_memory(uint32_t address, uint32_t chunk_size,
					uint32_t chunk_num, uint8_t const *data,
					bool cache_inval)
{
	struct target *write_target;
	if (cache_inval) {
		// A9 writes and invalidates
		write_target = target_a9;
	} else {
		// M3 writes and does not invalidate
		write_target = target_m3;
	}

	if (chunk_size > 4) {
		LOG << "FATAL ERROR: WRITING CHUNKS BIGGER THAN 4 BYTE NOT ALLOWED"
				<< endl;
		exit(-1);
	}

	if (target_write_phys_memory(write_target, address, chunk_size, chunk_num, data)) {
		LOG << "FATAL ERROR: Writing to memory failed." << endl;
		exit(-1);
	}
}

int oocdw_register_timer(void *this_ptr, p_timer_handler_t funct, uint64_t useconds,
							bool active, const char *id)
{
	for (int i=0; i<P_MAX_TIMERS; i++ ) {
		// find unused timer
		if (!timers[i].inUse) {
			timers[i].this_ptr = this_ptr;
			timers[i].funct = funct;
			timers[i].timeToFire = useconds;
			gettimeofday(&(timers[i].time_begin), NULL);
			timers[i].active = active;
			strcpy(timers[i].id, id);
			timers[i].inUse = true;
			return i;
		}
	}
	return -1;
}

bool oocdw_unregisterTimer(unsigned timerID)
{
	if (!timers[timerID].inUse) {
		return false;
	}
	timers[timerID].inUse = false;
	return true;
}

void oocdw_deactivate_timer(unsigned timer_index)
{
	timers[timer_index].active = false;
}

static void update_timers()
{
	for (int i=0; i<P_MAX_TIMERS; i++ ) {
		if (timers[i].inUse && timers[i].active) {
			struct timeval t_now, t_diff;
			gettimeofday(&t_now, NULL);
			timersub(&t_now, &timers[i].time_begin, &t_diff);

			uint64_t useconds_delta = t_diff.tv_sec * 1000000 + t_diff.tv_usec;

			if (timers[i].timeToFire <= useconds_delta) {
				LOG << "TIMER EVENT " << useconds_delta << " > " <<timers[i].timeToFire << endl;

				// Halt target to get defined halted state at experiment end
				oocdw_halt_target(target_a9);
				// Fire
				timers[i].funct(timers[i].this_ptr);
			}
		}
	}
}

static uint32_t getCurrentPC()
{
	return buf_get_u32(arm_a9->pc->value, 0, 32);
}

static bool getMemAccess(uint32_t opcode, struct halt_condition *access) {
	arm_regs_t regs;

	oocdw_read_reg(fail::RI_CPSR, &regs.cpsr);

	arm_instruction_t op;
	int retval = decode_instruction(opcode, &regs, &op);
	if (retval == 2) { // Instruction won't be executed
		return false;
	}
	if (retval) {
		LOG << "ERROR: Opcode " << hex << opcode << " at instruction "
				<< getCurrentPC() << dec << " could not be decoded" << endl;
		exit(-1);
	}

	if (op.flags & (OP_FLAG_READ | OP_FLAG_WRITE)) {
		for (int i = 0; i < 16; i++) {
			if ((1 << i) & op.regs_r) {
				oocdw_read_reg(i, &(regs.r[i]));
				if (i == 15) {
					regs.r[15] += 8;
				}
			}
			if ((i >= 10) && ((1 << i) & op.regs_r_fiq)) {
				oocdw_read_reg(i - 10 + fail::RI_R10_FIQ, &(regs.r[i]));
			}
		}

		if (decode_instruction(opcode, &regs, &op)) {
			LOG << "ERROR: Opcode " << hex << opcode << dec
					<< " could not be decoded" << endl;
			exit(-1);
		}

		access->address = op.mem_addr;
		access->addr_len = op.mem_size;
		access->type = (op.flags & OP_FLAG_READ) ? HALT_TYPE_WP_READ : HALT_TYPE_WP_WRITE;
		return true;
	}
	return false;
}

/*
 * Returns all memory access events of current instruction in
 * 0-terminated (0 in address field) array of max length.
 */
static bool getCurrentMemAccess(struct halt_condition *access)
{
	uint32_t pc = getCurrentPC();

	uint32_t opcode;
	oocdw_read_from_memory(pc, 4 , 1, (uint8_t*)(&opcode));

	return getMemAccess(opcode, access);
}

/*
 * If only one watchpoint is active, this checkpoint gets returned by
 * this function
 * Will be eliminated by disassembling and evaluating instruction
 */
static bool getHaltingWatchpoint(struct halt_condition *hc)
{
	struct watchpoint *watchpoint = target_a9->watchpoints;

	if (!watchpoint || watchpoint->next) {
		// Multiple watchpoints activated? No single answer possible
		return false;
	}

	hc->address = watchpoint->address;
	hc->addr_len = watchpoint->length;
	switch (watchpoint->rw) {
	case WPT_READ:
		hc->type = HALT_TYPE_WP_READ;
		break;
	case WPT_WRITE:
		hc->type = HALT_TYPE_WP_WRITE;
		break;
	case WPT_ACCESS:
		hc->type = HALT_TYPE_WP_READWRITE;
		break;
	default:
		LOG << "FATAL ERROR: Something went seriously wrong, here" << endl;
		break;
	}

	return true;
}


static uint32_t cc_overflow = 0;

void freeze_cycle_counter()
{
	struct armv7a_common *armv7a = target_to_armv7a(target_a9);
	struct arm_dpm *dpm = armv7a->arm.dpm;
	int retval;
	retval = dpm->prepare(dpm);

	if (retval != ERROR_OK) {
		LOG << "Unable to prepare for reading dpm register" << endl;
	}

	uint32_t data = 1;	// Counter 0, else shift left by counter-number

	/* Write to PMCNTENCLR */
	retval = dpm->instr_read_data_r0(dpm,
		ARMV4_5_MRC(15, 0, 0, 9, 12, 2),
		&data);

	if (retval != ERROR_OK) {
		LOG << "Unable to write PMCNTENCLR-Register" << endl;
	}

	dpm->finish(dpm);
}

void unfreeze_cycle_counter()
{
	struct armv7a_common *armv7a = target_to_armv7a(target_a9);
	struct arm_dpm *dpm = armv7a->arm.dpm;
	int retval;
	retval = dpm->prepare(dpm);

	if (retval != ERROR_OK) {
		LOG << "Unable to prepare for reading dpm register" << endl;
	}

	uint32_t data = 1;	// Counter 0, else shift left by counter-number

	/* Write to PMCNTENSET */
	retval = dpm->instr_read_data_r0(dpm,
		ARMV4_5_MRC(15, 0, 0, 9, 12, 1),
		&data);

	if (retval != ERROR_OK) {
		LOG << "Unable to write PMCNTENCLR-Register" << endl;
	}

	dpm->finish(dpm);
}


uint64_t oocdw_read_cycle_counter()
{
	struct armv7a_common *armv7a = target_to_armv7a(target_a9);
	struct arm_dpm *dpm = armv7a->arm.dpm;
	int retval;
	retval = dpm->prepare(dpm);

	if (retval != ERROR_OK) {
		LOG << "Unable to prepare for reading dpm register" << endl;
	}

	uint32_t data, flags;

	// PMCCNTR
	retval = dpm->instr_read_data_r0(dpm,
			ARMV4_5_MRC(15, 0, 0, 9, 13, 0),
			&data);

	if (retval != ERROR_OK) {
		LOG << "Unable to read PMCCNTR-Register" << endl;
	}

	// PMOVSR
	retval = dpm->instr_read_data_r0(dpm,
			ARMV4_5_MRC(15, 0, 0, 9, 12, 3),
			&flags);

	if (retval != ERROR_OK) {
		LOG << "Unable to read PMOVSR-Register" << endl;
	}

	if (flags & (1 << 31)) {
		cc_overflow++;

		flags ^= (1 << 31);

		retval = dpm->instr_write_data_r0(dpm,
				ARMV4_5_MCR(15, 0, 0, 9, 12, 3),
				flags);

		if (retval != ERROR_OK) {
			LOG << "Unable to read PMOVSR-Register" << endl;
		}
	}

	dpm->finish(dpm);

	return (uint64_t)data + ((uint64_t)cc_overflow << 32);
}

static void init_symbols()
{
	sym_GenericTrapHandler = elf_reader.getSymbol("GenericTrapHandler").getAddress();
	sym_MmuFaultBreakpointHook = elf_reader.getSymbol("MmuFaultBreakpointHook").getAddress();
	sym_SafetyLoopBegin = elf_reader.getSymbol("SafetyloopBegin").getAddress();
	sym_SafetyLoopEnd = elf_reader.getSymbol("SafetyloopEnd").getAddress();
	sym_addr_PageTableFirstLevel = elf_reader.getSymbol("A9TTB").getAddress();
	sym_addr_PageTableSecondLevel = elf_reader.getSymbol("A9TTB_L2").getAddress();
	LOG << hex << "SYMBOLS READ: " << endl <<
			"GenericTrapHandler     " << sym_GenericTrapHandler << endl <<
			"MmuFaultBreakpointHook " << sym_MmuFaultBreakpointHook << endl <<
			"SafetyLoopBegin        " << sym_SafetyLoopBegin << endl <<
			"SafetyLoopEnd          " << sym_SafetyLoopEnd << endl <<
			"A9TTB                  " << sym_addr_PageTableFirstLevel << endl <<
			"A9TTB_L2               " << sym_addr_PageTableSecondLevel << endl << dec;
	if (!sym_GenericTrapHandler
			|| !sym_MmuFaultBreakpointHook
			|| !sym_SafetyLoopBegin
			|| !sym_SafetyLoopEnd
			|| !sym_addr_PageTableFirstLevel
			|| !sym_addr_PageTableSecondLevel) {
		LOG << "FATAL ERROR: Not all relevant symbols were found" << endl;
		exit(-1);
	}
}

static void invalidate_tlb()
{
	struct armv7a_common *armv7a = target_to_armv7a(target_a9);
	struct arm_dpm *dpm = armv7a->arm.dpm;
	int retval;
	retval = dpm->prepare(dpm);

	if (retval != ERROR_OK) {
		LOG << "FATAL ERROR: Unable to prepare for reading dpm register" << endl;
		exit (-1);
	}

	retval = dpm->instr_write_data_r0(dpm,
		ARMV4_5_MCR(15, 0, 0, 8, 7, 0),
		0);

	if (retval != ERROR_OK) {
		LOG << "FATAL ERROR: Unable to invalidate TLB" << endl;
		exit (-1);
	}

	dpm->finish(dpm);
}

struct mmu_page_section {
	enum descriptor_e type;
	uint32_t index;
};

static void divide_into_pages_and_sections(const struct mem_range &in,
											std::vector<struct mmu_page_section> &out)
{
	struct mmu_page_section ps;

	uint32_t address = in.address;
	uint32_t width = in.width;

	// add pre-section pages
	while ((width > 0) && ((address % 0x100000) != 0)) {
		ps.index = ((address >> 20) * 256) + ((address >> 12) & 0xFF);
		ps.type = DESCRIPTOR_PAGE;
		out.push_back(ps);

		address += 0x1000;
		width -= 0x1000;
	}

	// add sections
	while (width >= 0x100000) {
		ps.index = address >> 20;
		ps.type = DESCRIPTOR_SECTION;
		out.push_back(ps);

		address += 0x100000;
		width -= 0x100000;
	}

	// add post-section pages
	while (width > 0) {
		ps.index = ((address >> 20) * 256) + ((address >> 12) & 0xFF);
		ps.type = DESCRIPTOR_PAGE;
		out.push_back(ps);

		address += 0x1000;
		width -= 0x1000;
	}
}

static void force_page_alignment_mem_range (std::vector<struct mem_range> &in,
											std::vector<struct mem_range> &out)
{
	std::vector<struct mem_range>::iterator it;
	for (it = in.begin(); it != in.end(); it++) {
		struct mem_range tmp;

		if ((it->address % 0x1000) != 0) {
			tmp.address = it->address - (it->address % 0x1000);
			tmp.width = it->width + (it->address % 0x1000);
		} else {
			tmp.address = it->address;
			tmp.width = it->width;
		}

		if ((tmp.width % 0x1000) != 0) {
			tmp.width += 0x1000 - (tmp.width % 0x1000);
		}

		out.push_back(tmp);
	}
}

static bool compareByAddress(const struct mem_range &a, const struct mem_range &b)
{
	return a.address < b.address;
}

static void merge_mem_ranges (std::vector<struct mem_range> &in_out)
{
	// sort ascending by address
	std::sort(in_out.begin(), in_out.end(), compareByAddress);

	std::vector<struct mem_range>::iterator it, next_it;
	it = in_out.begin();
	next_it = it; next_it++;
	while (it != in_out.end() && next_it != in_out.end()) {
		if ((it->address + it->width) >= next_it->address) {
			uint32_t additive_width = (next_it->address - it->address) + next_it->width;
			if (additive_width > it->width) {
				it->width = additive_width;
			}
			in_out.erase(next_it);
			next_it = it; next_it++;
		} else {
			it++; next_it++;
		}
	}
}

/*
 * Aggregate writes for faster MMU configuration
 */
static void execute_buffered_writes(std::vector<std::pair<uint32_t, uint32_t> > &buf) {
	std::vector<std::pair<uint32_t, uint32_t> >::iterator it_buf = buf.begin(), it_next;

	it_next = it_buf; it_next++;
	if (it_buf != buf.end()) {
		uint32_t start_address = it_buf->first;
		uint32_t data [256];
		int data_idx = 0;
		for (; it_buf != buf.end(); it_buf++) {
			data[data_idx++] = it_buf->second;
			if (it_next == buf.end() || (it_buf->first + 4) != it_next->first || data_idx == 256) {
				LOG << "Writing " << data_idx << " buffered items to address " << hex << start_address << dec << endl;
				oocdw_write_to_memory(start_address, 4, data_idx, (unsigned char*)(data), true);
				if (it_next == buf.end()) {
					break;
				}
				data_idx = 0;
				start_address = it_next->first;
			}
			it_next++;
		}
	}
}

static bool update_mmu_watch_add()
{
	bool invalidate = false;

	std::vector<struct mem_range>::iterator it;
	for (it = mmu_watch_add_waiting_list.begin(); it != mmu_watch_add_waiting_list.end(); it++) {
		// already watched?
		std::vector<struct mem_range>::iterator search;
		search = find_mem_range_in_vector(*it, mmu_watch);
		if (search != mmu_watch.end()) {
			LOG << hex << "FATAL ERROR: Memory Range already watched. Address: "
					<< it->address << ", width: " << it->width << dec << endl;
			exit(-1);
		}

		// memorise exact range to watch
		mmu_watch.push_back(*it);
	}
	mmu_watch_add_waiting_list.clear();

	std::vector<struct mem_range> mmu_watch_aligned;
	force_page_alignment_mem_range(mmu_watch, mmu_watch_aligned);

	// Merge existing and new configuration and merge direct neighbours
	merge_mem_ranges(mmu_watch_aligned);

	std::vector<std::pair<uint32_t, uint32_t> >buffer_for_chunked_write;

	// We don't calculate a diff of previous and new configuration, because the
	// descriptor structures will tell us, if a targeted part was already unmapped
	// In this case we just do nothing

	for (it = mmu_watch_aligned.begin(); it != mmu_watch_aligned.end(); it++) {
		std::vector<struct mmu_page_section> candidates;
		divide_into_pages_and_sections(*it, candidates);

		std::vector<struct mmu_page_section>::iterator cand_it;
		for (cand_it = candidates.begin(); cand_it != candidates.end(); cand_it++) {
			struct mmu_first_lvl_descr *first_lvl;
			if (cand_it->type == DESCRIPTOR_PAGE) {
				first_lvl = &mmu_conf[cand_it->index/256];
				if (first_lvl->type == DESCRIPTOR_SECTION) {
					if (!first_lvl->mapped) {
						continue;
					}
					// Set 1st lvl descriptor to link to 2nd-lvl table
					uint32_t pageTableAddress = sym_addr_PageTableSecondLevel
							+ ((cand_it->index - (cand_it->index % 256)) * 4);
					uint32_t descr_content = pageTableAddress | 0x1e9;

					uint32_t target_address = sym_addr_PageTableFirstLevel
							+ ((cand_it->index / 256) * 4);
					LOG << "Writing to " << hex << target_address << dec << endl;
					oocdw_write_to_memory(target_address , 4, 1,
							(unsigned char*)(&descr_content), true);

					invalidate = true;
					first_lvl->second_lvl_mapped = new bool[256];

					for (int i = 0; i < 256; i++) {
						first_lvl->second_lvl_mapped[i] = true;
						descr_content = (((cand_it->index - (cand_it->index % 256)) + i) << 12) | 0x32;
						buffer_for_chunked_write.push_back(std::pair<uint32_t, uint32_t>(
								pageTableAddress + (i * 4),
								descr_content));
					}
					first_lvl->type = DESCRIPTOR_PAGE;
				}

				// as the type of the first_lvl descriptor should have changed, this if-condition
				// will also be executed, if the previous one was and if the section was not
				// already unmapped
				if (first_lvl->type == DESCRIPTOR_PAGE) {
					if (first_lvl->second_lvl_mapped[cand_it->index % 256]) {
						uint32_t descr_content = (cand_it->index << 12) | 0x30;
						buffer_for_chunked_write.push_back(std::pair<uint32_t, uint32_t>(
							sym_addr_PageTableSecondLevel + (cand_it->index * 4),
							descr_content));
						first_lvl->second_lvl_mapped[cand_it->index % 256] = false;
						invalidate = true;
					}
				}
			} else if (cand_it->type == DESCRIPTOR_SECTION) {

				first_lvl = &mmu_conf[cand_it->index];
				if (first_lvl->type == DESCRIPTOR_PAGE) {
					// Bring up to Section descriptor
					first_lvl->type = DESCRIPTOR_SECTION;
					// This information is not entirely valid, but it will be used
					// by the next if-clause to unmap the whole section.
					// Whatever configuration was made on page-lvl won't be used any more
					first_lvl->mapped = true;
					delete[] first_lvl->second_lvl_mapped;
					// all the rest is handled in the next if condition
				}

				if (first_lvl->type == DESCRIPTOR_SECTION) {
					if (first_lvl->mapped) {
						uint32_t descr_content = (cand_it->index << 20) | 0xc00;
						uint32_t descr_addr = sym_addr_PageTableFirstLevel + (cand_it->index * 4);

						buffer_for_chunked_write.push_back(
							std::pair<uint32_t, uint32_t>(descr_addr,descr_content));

						first_lvl->mapped = false;
						invalidate = true;
					}
				}
			}
		}
	}

	execute_buffered_writes(buffer_for_chunked_write);

	return invalidate;
}

static void get_range_diff(	std::vector<struct mem_range> &before,
							std::vector<struct mem_range> &after,
							std::vector<struct mem_range> &diff)
{
	// sort both before and after vector ascending by address
	std::sort(before.begin(), before.end(), compareByAddress);
	std::sort(after.begin(), after.end(), compareByAddress);

	/*
	 * There are now 5 possible cases for every entry in <before>:
	 *
	 * 			1)			2)			3)			4)			5)
	 * Before:	|------|	|------|	|------|	|------|	|------|
	 * After:				|------|	|----|	  	  |----|	  |--|
	 */

	std::vector<struct mem_range>::iterator it_before;
	std::vector<struct mem_range>::iterator it_after = after.begin();
	for (it_before = before.begin(); it_before != before.end(); it_before++) {

		// case 1
		if (it_after == after.end() || (it_after->address > (it_before->address + it_before->width))) {
			diff.push_back(*it_before);
		} else if (it_after->address == it_before->address) {
			if (it_after->width == it_before->width) {			// case 2
				continue;
			} else if (it_after->width < it_before->width) {	// case 3
				struct mem_range tmp;
				tmp.address = it_after->address + it_after->width;
				tmp.width = it_before->width - it_after->width;
				diff.push_back(tmp);
			} else {
				LOG << "FATAL ERROR: Unexpected result in mmu diff calculation" << endl;
				exit(-1);
			}
		} else if (it_after->address > it_before->address) {
			uint32_t address_diff = it_after->address - it_before->address;
			if ((it_after->width + address_diff) == it_before->width) {		// case 4
				struct mem_range tmp;
				tmp.address = it_before->address;
				tmp.width = address_diff;
				diff.push_back(tmp);
			} else if ((it_after->width + address_diff) < it_before->width) {	// case 5
				struct mem_range tmp_a, tmp_b;
				tmp_a.address = it_before->address;
				tmp_a.width = address_diff;
				tmp_b.address = it_after->address + it_after->width;
				tmp_b.width = it_before->width - address_diff - it_after->width;
				diff.push_back(tmp_a);
				diff.push_back(tmp_b);
			} else {
				LOG << "FATAL ERROR: Unexpected result in mmu diff calculation" << endl;
				exit(-1);
			}
		} else {
			LOG << "FATAL ERROR: Unexpected result in mmu diff calculation" << endl;
			exit(-1);
		}
	}
}

static bool update_mmu_watch_remove()
{
	bool invalidate = false;

	std::vector<struct mem_range> mmu_watch_aligned_before;
	force_page_alignment_mem_range(mmu_watch, mmu_watch_aligned_before);

	std::vector<struct mem_range>::iterator it;
	for (it = mmu_watch_del_waiting_list.begin(); it != mmu_watch_del_waiting_list.end(); it++) {
		// Remove from mmu_watch
		std::vector<struct mem_range>::iterator search;
		search = find_mem_range_in_vector(*it, mmu_watch);
		if (search == mmu_watch.end()) {
			LOG << hex << "FATAL ERROR: MMU could not be configured to no longer watch address: "
					<< it->address << ", width: " << it->width << dec << endl;
			exit(-1);
		}
		mmu_watch.erase(search);
	}
	mmu_watch_del_waiting_list.clear();

	std::vector<struct mem_range> mmu_watch_aligned_after;
	force_page_alignment_mem_range(mmu_watch, mmu_watch_aligned_after);

	std::vector<struct mem_range> mmu_watch_aligned_diff;
	get_range_diff(mmu_watch_aligned_before, mmu_watch_aligned_after, mmu_watch_aligned_diff);

	std::vector<std::pair<uint32_t, uint32_t> >buffer_for_chunked_write;

	// Remove difference from mmu conf
	for (it = mmu_watch_aligned_diff.begin(); it != mmu_watch_aligned_diff.end(); it++) {
		std::vector<struct mmu_page_section> candidates;
		divide_into_pages_and_sections(*it, candidates);

		std::vector<struct mmu_page_section>::iterator cand_it;
		for (cand_it = candidates.begin(); cand_it != candidates.end(); cand_it++) {
			struct mmu_first_lvl_descr *first_lvl;
			if (cand_it->type == DESCRIPTOR_PAGE) {
				first_lvl = &mmu_conf[cand_it->index/256];
				if (first_lvl->type == DESCRIPTOR_SECTION) {
					if (first_lvl->mapped) {
						// ToDo: Throw error? This is unexpected!
						continue;
					}
					// Set 1st lvl descriptor to link to 2nd-lvl table
					uint32_t pageTableAddress = sym_addr_PageTableSecondLevel + ((cand_it->index - (cand_it->index % 256)) * 4);
					uint32_t descr_content = pageTableAddress | 0x1e9;
					uint32_t target_address = sym_addr_PageTableFirstLevel + ((cand_it->index / 256) * 4);
					LOG << "Writing to " << hex << target_address << dec << endl;
					oocdw_write_to_memory(target_address, 4, 1, (unsigned char*)(&descr_content), true);
					invalidate = true;

					first_lvl->second_lvl_mapped = new bool[256];

					// prepare chunk
					for (int i = 0; i < 256; i++) {
						first_lvl->second_lvl_mapped[i] = false;
						descr_content = (((cand_it->index - (cand_it->index % 256)) + i) << 12) | 0x30;
						buffer_for_chunked_write.push_back(std::pair<uint32_t, uint32_t>(
							pageTableAddress + (i * 4),
							descr_content));
					}
					first_lvl->type = DESCRIPTOR_PAGE;
				}

				if (first_lvl->type == DESCRIPTOR_PAGE) {
					if (!first_lvl->second_lvl_mapped[cand_it->index % 256]) {
						uint32_t descr_content = (cand_it->index << 12) | 0x32;
						buffer_for_chunked_write.push_back(std::pair<uint32_t, uint32_t>(
								sym_addr_PageTableSecondLevel + (cand_it->index * 4),
								descr_content));
						first_lvl->second_lvl_mapped[cand_it->index % 256] = true;
					}
				}
			} else if (cand_it->type == DESCRIPTOR_SECTION) {

				first_lvl = &mmu_conf[cand_it->index];
				if (first_lvl->type == DESCRIPTOR_PAGE) {
					// Bring up to Section descriptor
					first_lvl->type = DESCRIPTOR_SECTION;
					// This information is not entirely valid, but it will be used
					// by the next if-clause to map the whole section.
					// Whatever configuration was made on page-lvl won't be used any more
					first_lvl->mapped = false;
					delete[] first_lvl->second_lvl_mapped;
					// all the rest is handled in the next if condition
				}

				if (first_lvl->type == DESCRIPTOR_SECTION) {
					if (!first_lvl->mapped) {
						uint32_t descr_content = (cand_it->index << 20) | 0xc02;
						uint32_t descr_addr = sym_addr_PageTableFirstLevel + (cand_it->index * 4);
						LOG << "Write to " << hex << descr_addr << dec << endl;
						oocdw_write_to_memory(descr_addr, 4, 1, (unsigned char*)(&descr_content), true);
						LOG << "Writing entry at " << hex << descr_addr << " content: " << descr_content << dec << endl;
						first_lvl->mapped = true;
					}
				}
			}
		}
	}

	// execute buffered writes
	execute_buffered_writes(buffer_for_chunked_write);
	exit(0);

	return invalidate;
}

static void update_mmu_watch()
{
	//LOG << "UPDATE MMU" << endl;

	bool invalidate = false;

	/*
	 * ADDITION
	 */
	if (mmu_watch_add_waiting_list.size() > 0) {
		invalidate = update_mmu_watch_add();
	}

	/*
	 * REMOVAL
	 */
	if (mmu_watch_del_waiting_list.size() > 0) {
		if (update_mmu_watch_remove()) {
			invalidate = true;
		}
	}

	if (invalidate) {
		invalidate_tlb();
	}
}

static void unmap_page_section(uint32_t address)
{
	uint32_t data;
	oocdw_read_from_memory(address, 4, 1, (unsigned char*)(&(data)));

	// clear 2 lsb
	data &= 0xfffffffc;

	oocdw_write_to_memory(address, 4, 1, (unsigned char*)(&(data)), true);
}

static std::vector<struct mem_range>::iterator
find_mem_range_in_vector(struct mem_range &elem, std::vector<struct mem_range> &vec)
{
	std::vector<struct mem_range>::iterator search;
	for (search = vec.begin(); search != vec.end(); search++) {
		if ((elem.address == search->address) && (elem.width == search->width)) {
			break;
		}
	}
	return search;
}

static struct timeval freeze_begin;

static void freeze_timers()
{
	// Set freeze begin time and active timers to be freezed, so
	// on unfreeze the bygone time can be subtracted from them
	gettimeofday(&freeze_begin, NULL);
	for (int i = 0; i < P_MAX_TIMERS; i++) {
		if (timers[i].inUse) {
			timers[i].freezed = true;
		}
	}
}

static void unfreeze_timers()
{
	// Calculate time frozen and subtract from frozen timers
	struct timeval freeze_end, freeze_delta;
	gettimeofday(&freeze_end, NULL);
	timersub(&freeze_end, &freeze_begin, &freeze_delta);

	for (int i = 0; i < P_MAX_TIMERS; i++) {
		if (timers[i].inUse && timers[i].freezed) {
			struct timeval tmp = timers[i].time_begin;
			timeradd(&tmp, &freeze_delta, &timers[i].time_begin);

			timers[i].freezed = false;
		}
	}
}
