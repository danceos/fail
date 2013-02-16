/**
 * FailT32 -- Fault Injection on the Lauterbach Trace32 System
 *
 * 1. Invoke t32 executable with appropriate Lauterbach Skript
 *  - Script has to load binary
 *  - and let system run until entry point
 *  -> Then we have a readily configured system
 *
 * @author Martin Hoffmann <hoffmann@cs.fau.de>
 * @date 15.02.2013
 */

#include <t32.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>

#include "config/VariantConfig.hpp"
#include "sal/SALInst.hpp"


using namespace std;

/* Default T32 error handler */
void err(int ernum){
	if(err != 0){
		cerr << "Error: " << err << endl;
		//exit(-1);
	}
}


/* Here we go... */
int main(int argc, char** argv){

  // Evaluate arguments

  // Setup connection to Lauterbach
  cout << "Lauterbach remote connection" << endl;
	int error;
	char hostname[] = "localhost";
	char packlen[] = "1024";
	char port[] = "20010";

	cout << "[T32] Connecting to " << hostname << ":" << port << " Packlen: " << packlen << endl;
	T32_Config("NODE=", hostname);
	T32_Config("PACKLEN=", packlen);
	T32_Config("PORT=", port);

	cout << "[T32] Init." << endl;
	err(T32_Init());

	cout << "[T32] Attach." << endl;
	err(T32_Attach(T32_DEV_ICD));


  // Let the SimulatorController do the dirty work.
  fail::simulator.startup();

  // Here, we come back after any experiment called a resume
  // Start execution of the SUT.
  // The experiments/traces hopefully set some Breakpoints, we can react on.
  // We may also provide a timeout, if a TimerListener was set wanted.

 /*
   while(1) {
        // Start execution (with next timeout, in any)

        // Wait for debugger to stop.

        // Evaluate state.

        // Call appropriate callback of the SimulatorController.

    }
  */
  cout << "[T32 Backend] After startup" << endl;
  return 0;
}



