#include <iostream>
#include <unistd.h>
#include <fstream>

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "sal/SALInst.hpp"
#include "sal/Listener.hpp"
#include "sal/Memory.hpp"
#include "config/FailConfig.hpp"
#include "../plugins/tracing/TracingPlugin.hpp"
#include "../plugins/serialoutput/SerialOutputLogger.hpp"


// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_EVENT_MEMREAD) \
|| !defined(CONFIG_EVENT_GUESTSYS) || !defined(CONFIG_EVENT_INTERRUPT) \
|| !defined(CONFIG_EVENT_IOPORT) || !defined(CONFIG_EVENT_JUMP) || !defined(CONFIG_EVENT_MEMREAD) \
|| !defined(CONFIG_EVENT_MEMWRITE) || !defined(CONFIG_EVENT_TRAP) || !defined(CONFIG_SR_REBOOT) \
|| !defined(CONFIG_SR_SAVE) || !defined(CONFIG_SR_RESTORE) || !defined(CONFIG_SUPPRESS_INTERRUPTS)
  #error This experiment needs: all CONFIG_EVENT_*, all CONFIG_SR_* and the CONFIG_SUPPRESS_INTERRUPTS flag. Enable these in the configuration.
#endif

using namespace std;
using namespace fail;

bool RegressionTest::run()
{
	unsigned instrAddr_at_save = 0;
	BaseListener *ev;

	//Result-File
	fstream file ("regression-test.results", ios::out | ios::trunc);
	file << "experiment start" << endl;

	//Wait for correct start point
	GuestListener g;

	while (simulator.addListenerAndResume(&g) == &g) {
		if (g.getData() == 'A') {
			file << "Found start-point with signal: " << g.getData() << endl;
			file << "GuestListener-Test SUCCESSFUL." << endl;
			break;
		}
	}

	//Watchdog-Timer
	TimerListener watchdog(100000);
	simulator.addListener(&watchdog);

	//Save state
	file << "Saving state." << endl;
	simulator.save("regression-save");
	instrAddr_at_save = simulator.getCPU(0).getInstructionPointer();

	//Start Plugins
	TracingPlugin tp;
	ofstream of("regression-trace.results");
	tp.setOstream(&of);
	simulator.addFlow(&tp);

	SerialOutputLogger so(0x3F8);
	simulator.addFlow(&so);

	//BPListener
	BPSingleListener bp_test(REGRESSION_FUNC_BP);
	BPSingleListener mem_read_test(REGRESSION_FUNC_MEM_READ);
	BPSingleListener mem_write_test(REGRESSION_FUNC_MEM_WRITE);
	BPSingleListener trap_test(REGRESSION_FUNC_TRAP);
	BPSingleListener jump_test(REGRESSION_FUNC_JUMP);
	BPSingleListener interrupt_test(REGRESSION_FUNC_INTERRUPT);

	simulator.addListener(&bp_test);
	simulator.addListener(&mem_read_test);
	simulator.addListener(&mem_write_test);
	simulator.addListener(&trap_test);
	simulator.addListener(&jump_test);
	simulator.addListener(&interrupt_test);

	while (true) {
		simulator.removeListener(&watchdog);
		simulator.addListener(&watchdog);
		ev = simulator.resume();

		if (ev == &bp_test) {
			file << "Breakpoint-Test start." << endl;

			BPSingleListener mainbp(ANY_ADDR);
			mainbp.setCounter(1000);

			BPRangeListener bprange(REGRESSION_FUNC_LOOP_DONE, REGRESSION_FUNC_LOOP_DONE);
			BPSingleListener breakcounter(REGRESSION_FUNC_LOOP_DONE);
			simulator.addListener(&breakcounter);

			int count = 0;

			while (true) {

				BaseListener* ev = simulator.resume();

				if (ev == &breakcounter || ev == &bprange) {

					count++;
					//First 5 times test BPSingleListener
					if (count < 5) {
						simulator.addListener(&breakcounter);
					//Next 5 times test BPRangeListener
					} else if (count < 10) {
						simulator.addListener(&bprange);
					//At 10 run of loop start BPSingleListener, BPRangeListener, mainListener
					//which waits 1000 instructions.
					} else if (count == 10) {
						simulator.addListener(&breakcounter);
						simulator.addListener(&bprange);
						simulator.addListener(&mainbp);
					//If mainListener fires not first the test failes.
					}else if (count >= 10) {
						file << "Breakpoint-Test FAILED."<< endl;
						break;
					}
					//If mainListener fires first the test success.
				} else if (ev == &mainbp) {
					file << "Breakpoint-Test SUCCESSFUL." <<endl;
					break;
				} else if (ev == &watchdog) {
					file << "Breakpoint-Test FAILED --> Watchdog fired. Timeout!" << endl;
				}
			}
			file << "Breakpoint-Test end" << endl;
			simulator.removeListener(&breakcounter);
			simulator.removeListener(&bprange);
			simulator.removeListener(&mainbp);

		} else if (ev == &mem_read_test) {

			file << "Memory-Read-Test start." << endl;

			MemReadListener memread(REGRESSION_VAR_MTEST_READ);
			simulator.addListener(&memread);
			simulator.resume();

			file << "Memaddr-Read: " << hex << memread.getTriggerAddress() << dec << endl;

			if (memread.getTriggerAddress() == REGRESSION_VAR_MTEST_READ) {
				file << "Memory-Read-Test SUCCESSFUL." << endl;
			} else {
				file << "Memory-Read-Test FAILED." << endl;
			}

			file << "Memory-Read-Test end." << endl;
			simulator.removeListener(&memread);

		} else if (ev == &mem_write_test) {

			file << "Memory-Write-Test start." << endl;

			MemWriteListener memwrite(REGRESSION_VAR_MTEST_WRITE);
			simulator.addListener(&memwrite);
			simulator.resume();

			file << "Memaddr-WRITE: " << hex << memwrite.getTriggerAddress() << dec << endl;

			if (memwrite.getTriggerAddress() == REGRESSION_VAR_MTEST_WRITE) {
				file << "Memory-Write-Test SUCCESSFUL." << endl;
			} else {
				file << "Memory-Write-Test FAILED." << endl;
			}

			file << "Memory-Write-Test end." << endl;


		} else if (ev == &trap_test) {

			file << "Trap-Test start" << endl;

			TrapListener trap(ANY_TRAP);
			simulator.addListener(&trap);
			simulator.resume();
			file << "Trap found: " << trap.getTriggerNumber() << endl;

			file << "Trap-Test end" << endl;

			simulator.removeListener(&trap);

			//After a division-error trap ecos halts the cpu
			//Because of this a restore to start point is needed

			simulator.restore("regression-save");

			if (simulator.getCPU(0).getInstructionPointer() == instrAddr_at_save) {
				file << "Save-/Restore-Test SUCCESSFUL." << endl;
			} else {
				file << "Save-/Restore-Test FAILED. The instructionpointer after restore is \
				different to the ionstructionpointer after save! " << endl;
				file << "Instructionpointer after save: " << instrAddr_at_save << endl;
				file << "Instructionpointer after restore: " << \
				simulator.getCPU(0).getInstructionPointer() << endl;
			}

			// Reboot test

			file << "Reboot-Test start" << endl;

			BPSingleListener bpReboot(ANY_ADDR);

			simulator.addListener(&bpReboot);
			simulator.resume();

			long beforeReboot = bpReboot.getTriggerInstructionPointer();

			file << "Before Reboot-Addr: 0x" << hex << beforeReboot << dec << endl;

			simulator.reboot();

			bpReboot.setWatchInstructionPointer(beforeReboot);
			simulator.addListener(&bpReboot);
			simulator.resume();

			long afterReboot = bpReboot.getTriggerInstructionPointer();

			file << "After Reboot-Addr: 0x" << hex << afterReboot << dec << endl;

			if (beforeReboot == afterReboot) {
				file << "Reboot-Test SUCCESSFUL." << endl;
			} else {
				file << "Reboot-Test FAILED." << endl;
			}

			file << "Reboot-Test end" << endl;
			simulator.removeListener(&bpReboot);

			file << "Serial-Output: " << so.getOutput() << endl;

			file.flush();
			of.flush();

			simulator.terminate();

		} else if (ev == &jump_test) {
			file << "Jump-Test start" << endl;

			JumpListener jump(ANY_INSTR);
			simulator.addListener(&jump);
			simulator.resume();

			file << "Jump-Instruction found: " << jump.getOpcode() << endl;
			file << "current Instruction-Pointer: 0x" << hex <<\
			simulator.getCPU(0).getInstructionPointer() << dec << endl;

			file << "Jump-Test end" << endl;

			simulator.removeListener(&jump);

		} else if (ev == &interrupt_test) {

			file << "Interrupt-Test start" << endl;

			InterruptListener interrupt(32);
			simulator.addListener(&interrupt);
			ev = simulator.resume();

			if (ev == &interrupt) {
				file << "Interrupt-Test SUCCESSFUL. Interruptnum: " << interrupt.getTriggerNumber() \
				<< endl;
			}

			file << "Interrupt-Test end" << endl;
		} else if (ev == &watchdog) {
			file << "Watchdog fired. Timeout!" << endl;
		}
	}

	file << "experiment end" << endl;

	return true;

}
