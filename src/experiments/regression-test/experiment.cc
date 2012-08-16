#include <iostream>
#include <unistd.h>

#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "sal/SALInst.hpp"
#include "sal/Listener.hpp"
#include "util/Logger.hpp"
#include "config/FailConfig.hpp"
#include "sal/bochs/BochsRegister.hpp"

//ToDo:
// more flow-control by BreakpointListener
// life-counter (counter over BreakpointListener) monitor whether a test takes too much "time"
// more tests + test of plugins

// Check if configuration dependencies are satisfied:
#if !defined(CONFIG_EVENT_BREAKPOINTS) || !defined(CONFIG_EVENT_MEMREAD) \
	|| !defined(CONFIG_EVENT_MEMWRITE) || !defined(CONFIG_SR_SAVE) || !defined(CONFIG_SR_RESTORE) \
	|| !defined(CONFIG_SR_REBOOT) || !defined(CONFIG_EVENT_TRAP)
  #error This experiment needs: breakpoints, memread, memwrite. Enable these in the configuration.
#endif

using namespace std;
using namespace fail;

bool RegressionTest::run()
{
	BPSingleListener bpPather(ANY_ADDR);
	int count = 0;
	
	Logger log("Regression-Test", false);
	log << "experiment start" << endl;

//Breakpoint-Test
	log << "Breakpoint-Test start." << endl;
	
	BPSingleListener mainbp(ANY_ADDR); 
	mainbp.setCounter(1000);
	
	BPRangeListener bprange(REGRESSION_FUNC_LOOP_DONE, REGRESSION_FUNC_LOOP_DONE);
	BPSingleListener breakcounter(REGRESSION_FUNC_LOOP_DONE);
	simulator.addListener(&breakcounter);


	while(true){
		
		BaseListener* ev = simulator.resume();
		
		if(ev == &breakcounter || ev == &bprange) {
			
			count++;
			//First 5 times test BPSingleListener
			if(count < 5){
				log << "Loop-Single-Test!" << endl;
				simulator.addListener(&breakcounter);
			//Next 5 times test BPRangeListener
			}else if(count < 10){
				log << "Loop-Range-Test!" << endl;
				simulator.addListener(&bprange);
			//At 10 run of loop start BPSingleListener, BPRangeListener, mainListener 
			//which waits 1000 instructions. 
			}else if(count == 10){
				log << "loop-limit reached..." << endl;
				simulator.addListener(&breakcounter);
				simulator.addListener(&bprange);
				simulator.addListener(&mainbp);
			//If mainListener fires not first the test failes.
			}else if(count >= 10){
				log << "Breakpoint-Test FAILED."<< endl;
				break;
			}
			//If mainListener fires first the test success.
		}else if(ev == &mainbp) {
			log << "Breakpoint-Test SUCCESS." <<endl;
			break;
		}
	}
	//simulator.clearListeners(this);
	log << "Breakpoint-Test end" << endl;
	
//Memory test TODO..
/*
	log << "Memory-Test start." << endl;
	
	MemReadListener memread(REGRESSION_FUNC_MTEST_READ);
	MemWriteListener memwrite(REGRESSION_FUNC_MTEST_READ);
	
	simulator.resume();
	B D
	log << "Memaddr: " << hex <<memread.getTriggerAddress() << dec <<endl;
	
	
	log << "Memory-Test end." << endl;
*/


//Jump test

	log << "Jump-Test start" << endl;
	
	bpPather.setWatchInstructionPointer(REGRESSION_FUNC_JUMP);
	simulator.addListener(&bpPather);
	simulator.resume();
	log << "Begin of Jump-Function found." << endl;

	JumpListener jump(ANY_INSTR);
	simulator.addListener(&jump);
	simulator.resume();
	log << "Jump-Instruction found: " << jump.getOpcode() << endl;
	log << "current Instruction-Pointer: 0x" << hex <<\
	simulator.getRegisterManager().getInstructionPointer() << dec << endl;
	
	
	log << "Jump-Test end" << endl;


//Interrupt test

	log << "Interrupt-Test start" << endl;
	
	InterruptListener interrupt(32);
	simulator.addListener(&interrupt);
	simulator.resume();
	
	log << "Interrupt-Test SUCCESS" << endl;
	
	log << "Interrupt-Test end" << endl;


// Save / Restore test
	
	log << "Save-/Restore-Test start" << endl;
	
	simulator.save("regression-save");
	
	long beforeRestore = simulator.getRegisterManager().getInstructionPointer();
	log << "InstructionPointer before restore : 0x" << hex << beforeRestore << dec << " DEZ: "<< beforeRestore << endl;
	
	simulator.restore("regression-save");
	
	long afterRestore = simulator.getRegisterManager().getInstructionPointer();
	log << "InstructionPointer after restore: 0x" << hex << afterRestore << dec << " DEZ: "<< afterRestore << endl;
	
	if(beforeRestore == afterRestore) {
		log << "Save-/Restore-Test SUCCESS." << endl;
	}else {
		log << "Save-/Restore-Test FAILED." << endl;
	}
	
	log << "Save-/Restore-Test end" << endl;

// Reboot test

	log << "Reboot-Test start" << endl;

	BPSingleListener bpReboot(ANY_ADDR);
	
	simulator.addListener(&bpReboot);
	simulator.resume();

	long beforeReboot = bpReboot.getTriggerInstructionPointer();
	
	log << "Before Reboot-Addr: 0x" << hex << beforeReboot << dec << endl;
	
	bpReboot.setWatchInstructionPointer(beforeReboot);
	simulator.addListener(&bpReboot);
	
	simulator.reboot(); 
	
	long afterReboot = bpReboot.getTriggerInstructionPointer();
	
	log << "After Reboot-Addr: 0x" << hex << beforeReboot << dec << endl;
	
	if (beforeReboot == afterReboot){
		log << "Reboot-Test SUCCESS." << endl;
	}else {
		log << "Reboot-Test FAILED." << endl;
	}

	log << "Reboot-Test end" << endl;

// Trap test
	
	log << "Trap-Test start" << endl;
	
	bpPather.setWatchInstructionPointer(REGRESSION_FUNC_TRAP);
	simulator.addListener(&bpPather);
	simulator.resume();
	log << "Begin of Trap-Function found." << endl;
	
	TrapListener trap(ANY_TRAP);
	simulator.addListener(&trap);
	simulator.resume();
	log << "Trap found." << endl;
	
	log << "Trap-Test end" << endl;

	return true;
}
