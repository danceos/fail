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
}

#include <cassert>
#include <ostream>
#include <sys/time.h>
#include "config/VariantConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/SALConfig.hpp"
#include "sal/arm/ArmArchitecture.hpp"

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
 * State variable for propagation of a coming horizontal hop.
 * For correct execution of a horizontal hop, an additional
 * single-step needs to be executed before resuming target
 * execution.
 * Variable is set while setting new halt_conditions on basis of
 * current instruction and its memory access(es).
 */
static bool horizontal_step = false;

/*
 * As the normal loop execution is
 * resume - wait for halt - trigger events and set next halt condition(s),
 * single-stepping is different. It is accomplished by setting this state
 * variable.
 */
static bool single_step_requested = false;

// Timer related structures
#define P_MAX_TIMERS 32
struct timer{
    bool inUse;      			// Timer slot is in-use (currently registered).
    uint64_t  timeToFire; 		// Time to fire next in microseconds
    struct timeval time_begin;  // Timer value at activation of timer
    bool active;     			// 0=inactive, 1=active.
    p_timer_handler_t funct;  	// A callback function for when the
								//   timer fires.
    void *this_ptr;            	// The this-> pointer for C++ callbacks
                            	//   has to be stored as well.
#define PMaxTimerIDLen 32
    char id[PMaxTimerIDLen]; 	// String ID of timer.
};

struct timer timers[P_MAX_TIMERS];

fail::Logger LOG("OpenOCD", false);

/** FORWARD DECLARATIONS **/
static void update_timers();
static struct watchpoint *getHaltingWatchpoint();
static void getCurrentMemAccesses(struct halt_condition *accesses);
static uint32_t getCurrentPC();
static struct reg *get_reg_by_number(unsigned int num);

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

	/* Start the executable meat that can evolve into thread in future. */
	//ret = openocd_thread(argc, argv, cmd_ctx);

	if (parse_cmdline_args(cmd_ctx, argc, argv) != ERROR_OK)
		return EXIT_FAILURE;

	/*if (server_preinit() != ERROR_OK)
	  return EXIT_FAILURE;*/

	// set path to configuration file
	add_script_search_dir(OOCD_CONF_FILES_PATH);
	add_config_command("script "OOCD_CONF_FILE_PATH);

	ret = parse_config_file(cmd_ctx);
	if (ret != ERROR_OK) {
		LOG << "Error in openocd configuration!\nFor more detailed information refer to oocd.log" << endl;
		return EXIT_FAILURE;
	}

	// Servers (gdb/telnet) are not being activated
	/*  ret = server_init(cmd_ctx);
		if (ERROR_OK != ret)
		return EXIT_FAILURE;*/

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

	arm_a9 = target_to_arm(target_a9);
	jtag_poll_set_enabled (false);

	LOG << "OpenOCD 0.7.0 for Fail* and Pandaboard initialized" << endl;

	for (int i=0; i<P_MAX_TIMERS; i++) {
		timers[i].inUse = false;
	}

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
		while (single_step_requested) {
			if (target_step(target_a9, 1, 0, 1)) {
				LOG << "FATAL ERROR: Single-step could not be executed" << endl;
				return 1;
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

			fail::simulator.onBreakpoint(NULL, pc, fail::ANY_ADDR);
		}

		/*
		 * In the following loop stage it is assumed, that the target is
		 * running, so execution needs to be resumed, if the target is
		 * halted.
		 */
		if (target_a9->state == TARGET_HALTED) {
			LOG << "Resume" << endl;
			if (target_resume(target_a9, 1, 0, 1, 1)) {
				LOG << "FATAL ERROR: Target could not be resumed!" << endl;
				return 1;
			}
		}

		// Wait for target to halt
		while (target_a9->state != TARGET_HALTED) {
			// Polling needs to be done to detect target halt state changes.
			if (target_poll(target_a9)) {
				LOG << "FATAL ERROR: Error polling after resume!" << endl;
				return 1;
			}
			// Update timers, so waiting can be aborted
			update_timers();
		}

		// Check for halt and trigger event accordingly
		if (target_a9->state == TARGET_HALTED) {

			uint32_t pc = getCurrentPC();

			// After coroutine-switch, dbg-reason might change, so it must
			// be stored
			enum target_debug_reason current_dr = target_a9->debug_reason;
			switch (current_dr) {
			case DBG_REASON_WPTANDBKPT:
				/* Fall through */
			case DBG_REASON_BREAKPOINT:
				fail::simulator.onBreakpoint(NULL, pc, fail::ANY_ADDR);
				if (current_dr == DBG_REASON_BREAKPOINT) {
					break;
				}
				/* Potential fall through */
			case DBG_REASON_WATCHPOINT:
			{
				// ToDo: Replace with calls of every current memory access
				struct watchpoint *wp = getHaltingWatchpoint();
				if (!wp) {
					// ToDo: Determine address by interpreting instruction and register contents
					LOG << "FATAL ERROR: Can't determine memory-access address of halt cause" << endl;
					return 1;
				}

				int iswrite;
				switch (wp->rw) {
					case WPT_READ:
						iswrite = 0;
						break;
					case WPT_WRITE:
						iswrite = 1;
						break;
					case WPT_ACCESS:
						// ToDo: Can't tell if read or write
						iswrite = 1;
						break;
					default:
						LOG << "FATAL ERROR: Can't determine memory-access type of halt cause" << endl;
						return 1;
						break;
				}

				fail::simulator.onMemoryAccess(NULL, wp->address, wp->length, iswrite, pc);
			}
				break;
			case DBG_REASON_SINGLESTEP:
				LOG << "FATAL ERROR: Single-step is handled in previous loop phase" << endl;
				return 1;
				break;
			default:
				LOG << "FATAL ERROR: Target halted in unexpected cpu state!" << endl;
				break;
			}

			/*
			 * Execute single-step if horizontal hop was detected
			 */
			if (target_a9->state == TARGET_HALTED && horizontal_step) {
				if (target_step(target_a9, 1, 0, 1)) {
					LOG << "FATAL ERROR: Single-step could not be executed!" << endl;
					return 1;
				}
				// Reset horizontal hop flag
				horizontal_step = false;
			}
		}

		// Update timers. Granularity will be coarse, because this is done after polling the device
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



void oocdw_set_halt_condition(struct halt_condition *hc)
{
	assert((target_a9->state == TARGET_HALTED) && "Target not halted");
	assert((hc != NULL) && "No halt condition defined");

	horizontal_step = false;

	if (hc->type == HALT_TYPE_BP) {
		if (breakpoint_add(target_a9, hc->address,
					hc->addr_len, BKPT_HARD)) {
			LOG << "FATAL ERROR: Breakpoint could not be set" << endl;
			exit(-1);
		}

		// Compare current pc with set breakpoint (potential horizontal hop)
		if (hc->address == getCurrentPC()) {
			horizontal_step = true;
		}
	} else if (hc->type == HALT_TYPE_SINGLESTEP) {
		single_step_requested = true;
	} else {
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

		// Compare current memory access events with set watchpoint
		// (potential horizontal hop)
		struct halt_condition mem_accesses [4];
		getCurrentMemAccesses(mem_accesses);
		int i = 0;
		while (mem_accesses[i].address) {
			// Look for overlapping similar memory access
			if (mem_accesses[i].type == hc->type) {
				if (mem_accesses[i].address < hc->address) {
					if (mem_accesses[i].address + mem_accesses[i].addr_len >= hc->address) {
						horizontal_step = true;
					}
				} else {
					if (hc->address + hc->addr_len >= mem_accesses[i].address) {
						horizontal_step = true;
					}
				}
			}
			i++;
		}
	}
}

void oocdw_delete_halt_condition(struct halt_condition *hc)
{
	assert((target_a9->state == TARGET_HALTED) && "Target not halted");
	assert((hc != NULL) && "No halt condition defined");

	// Remove halt condition from pandaboard
	if (hc->type == HALT_TYPE_BP) {
		breakpoint_remove(target_a9, hc->address);
	} else if (hc->type == HALT_TYPE_SINGLESTEP) {
		// Do nothing. Single-stepping event hits one time and
		// extinguishes itself automatically
	} else if ((hc->type == HALT_TYPE_WP_READWRITE) ||
			(hc->type == HALT_TYPE_WP_READ) ||
			(hc->type == HALT_TYPE_WP_WRITE)) {
		watchpoint_remove(target_a9, hc->address);
	}
}

void oocdw_halt_target()
{
	if (target_halt(target_a9)) {
		LOG << "FATAL ERROR: Target could not be halted" << endl;
		exit(-1);
	}

	/*
	 * Wait for target to actually stop.
	 */
	long long then = timeval_ms();
	if (target_poll(target_a9)) {
		LOG << "FATAL ERROR: Target polling failed" << endl;
		exit(-1);
	}
	while (target_a9->state != TARGET_HALTED) {
		if (target_poll(target_a9)) {
			LOG << "FATAL ERROR: Target polling failed" << endl;
			exit(-1);
		}
		if (timeval_ms() > then + 1000) {
			LOG << "FATAL ERROR: Timeout waiting for target halt" << endl;
			exit(-1);
		}
	}
}

// ToDo: read from elf-file
#define SAFETYLOOP_BEGIN 	0x83000084
#define SAFETYLOOP_END		0x830000a0

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
	int retval, reboot_success = 0, fail_counter = 1;

	while (!reboot_success) {
		LOG << "Rebooting device" << endl;
		reboot_success = 1;

        // If target is not halted, reset will result in freeze
        if (target_a9->state != TARGET_HALTED) {
            oocdw_halt_target();
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

		usleep(700*1000);

		// Immediate halt after reset.
		oocdw_halt_target();

		uint32_t pc = buf_get_u32(arm_a9->pc->value, 0, 32);
		if (pc < SAFETYLOOP_BEGIN || pc > SAFETYLOOP_END) {
			LOG << "NOT IN LOOP!!! PC: " << hex << pc << dec << std::endl;

			// BP on entering main
			struct halt_condition hc;

			// ToDo: Non static MAIN ENTRY
			hc.address = SAFETYLOOP_END;
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
				// ToDo: Adjust timeout
				if (timeval_ms() > then + 2000) {
					LOG << "Error: Timeout waiting for main entry" << endl;
					reboot_success = 0;
					if (fail_counter++ > 4) {
						LOG << "FATAL ERROR: Rebooting not possible" << endl;
						exit(-1);
					}
					oocdw_halt_target();
					break;
				}
			}
			// Remove temporary
			oocdw_delete_halt_condition(&hc);
		} else {
			LOG << "Stopped in loop. PC: " << hex << pc << dec << std::endl;
		}

		if (reboot_success) {
			// Jump over safety loop (set PC)
			oocdw_write_reg(15, SAFETYLOOP_END + 0x4);
		}
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
		if (reg)
			break;
		cache = cache->next;
	}

	return reg;
}

void oocdw_read_reg(uint32_t reg_num, uint32_t *data)
{
	assert((target_a9->state == TARGET_HALTED) && "Target not halted");

	switch (reg_num) {
	case fail::RI_DFAR:
	{
		struct armv7a_common *armv7a = target_to_armv7a(target_a9);
		struct arm_dpm *dpm = armv7a->arm.dpm;
		int retval;
		retval = dpm->prepare(dpm);

		if (retval != ERROR_OK) {
			LOG << "Unable to prepare for reading dpm register" << endl;
		}

		retval = dpm->instr_read_data_r0(dpm,
				ARMV4_5_MRC(15, 0, 0, 6, 0, 0),
				data);
		if (retval != ERROR_OK) {
			LOG << "Unable to read DFAR-Register" << endl;
		}

		dpm->finish(dpm);
	}
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
	{
		struct reg *reg = get_reg_by_number(reg_num);

		if (reg->valid == 0) {
			reg->type->get(reg);
		}

		*data = *((uint32_t*)(reg->value));
	}
	break;
	default:
		LOG << "ERROR: Register with id " << reg_num << " unknown." << endl;
		break;
	}
}

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
		LOG << "ERROR: Register with id " << reg_num << " unknown." << endl;
		break;
	}
}

void oocdw_finish(int exCode)
{
	oocdw_exection_finished = true;
	oocdw_exCode = exCode;
}

void oocdw_read_from_memory(uint32_t address, uint32_t chunk_size,
					uint32_t chunk_num, uint8_t *data)
{
	if (target_read_memory(target_a9, address, chunk_size, chunk_num, data)) {
		LOG << "FATAL ERROR: Reading from memory failed." << endl;
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

	if (target_write_memory(write_target, address, chunk_size, chunk_num, data)) {
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
			struct timeval t_now;
			gettimeofday(&t_now, NULL);
			uint64_t useconds_delta = (t_now.tv_sec - timers[i].time_begin.tv_sec) * 1000000 + t_now.tv_usec - timers[i].time_begin.tv_usec;

			if (timers[i].timeToFire <= useconds_delta) {
				// Halt target to get defined halted state at experiment end
				oocdw_halt_target();
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

/*
 * Returns all memory access events of current instruction in
 * 0-terminated (0 in address field) array of max length.
 * TODO implement analysis of current instruction in combination
 * with register contents
 */
static void getCurrentMemAccesses(struct halt_condition *accesses)
{
	// ToDo: Get all 1-byte memory access events of current
	//		 instruction. For now return empty array.
	accesses[0].address = 0;
}

/*
 * If only one watchpoint is active, this checkpoint gets returned by
 * this function
 */
static struct watchpoint *getHaltingWatchpoint()
{
	struct watchpoint *watchpoint = target_a9->watchpoints;

	if (!watchpoint || watchpoint->next) {
		// Multiple watchpoints activated? No single answer possible
		return NULL;
	}
	return watchpoint;
}
