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
#include <stdio.h>
#include <vector>
#include "config/VariantConfig.hpp"
#include "sal/SALInst.hpp"
#include "optionparser.h"
#include "optionparser_ext.hpp"

#include "T32Connector.hpp"
using namespace std;

static fail::T32Connector t32;

//!< Program options
enum  optionIndex { UNKNOWN, HELP, RUN, T32HOST, PORT, PACKLEN };
const option::Descriptor usage[] =
{
 {UNKNOWN, 0,"" , ""    ,Arg::None, "USAGE: fail-client [options]\n\n"
                                            "Options:" },
 {HELP,    0,"" , "help",Arg::None, "  --help  \tPrint usage and exit." },
 {RUN,    0,"r", "run",Arg::Required, "  --run, -r  \tLauterbach script to startup system." },
 {T32HOST,    0,"t", "trace32-host",Arg::Required, "  --trace32-host, -t <hostname>  \tHostname/IP Address of the Trace32 instance. (default: localhost)" },
 {PORT,    0,"p", "port",Arg::Required, "  --port <NUM>, -p <NUM> \tTCP Port. (default: 20010)" },
 {PACKLEN,    0,"l", "packet-length",Arg::Required, "  --packet-length, -l <NUM> \tPacket length. (default: 1024)" },
 {0,0,0,0,0,0}
};

/* Here we go... */
int main(int argc, char** argv){

  // Evaluate arguments
  argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
  option::Stats stats(usage, argc, argv);
  option::Option options[stats.options_max], buffer[stats.buffer_max];
  option::Parser parse(usage, argc, argv, options, buffer);

  if (parse.error()){
    cerr << "Error parsing arguments." << endl;
    return 1;
  }

  if ( options[HELP] ) // || argc == 0 || options[RUN].count() == 0) // to enforce -s
  {
    int columns = getenv("COLUMNS")? atoi(getenv("COLUMNS")) : 80;
    option::printUsage(fwrite, stdout, usage, columns);
    return 0;
  }

  if(options[RUN].count()){
    t32.setScript(options[RUN].first()->arg);
  }

  if(options[T32HOST].count()){
    t32.setHostname(options[T32HOST].first()->arg);
  }

  if(options[PORT].count()){
    t32.setPort(options[PORT].first()->arg);
  }

  if(options[PACKLEN].count()){
    t32.setPacketlength(options[PACKLEN].first()->arg);
  }

  // Initialize T32
  if(t32.startup() == false){
    cout << "Could not connect to Lauterbach :(" << endl;
    return -1;
  }


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



