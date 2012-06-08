#include <cstdlib>

#include "icm/icmCpuManager.hpp"
#include "flakyMemory.hpp"
#include "beforeInstruction.hpp"

// enable relaxed scheduling for maximum performance
#define SIM_ATTRS (ICM_ATTR_RELAXED_SCHED)


icmProcessorObject createPlatform(
    const char *application,
    bool gdb,
    bool flaky=false,
    Addr lowAddr=0x00,
    Addr highAddr=0xffffffff
) {
    // select library components
    const char *vlnvRoot = 0; // when null use default library
    const char *vlnvRoot2 ="/srv/scratch/sirozipp/build/lib/" ; 
    const char *model = icmGetVlnvString(vlnvRoot,
                                         "arm.ovpworld.org",
                                         "processor",
                                         "armm",
                                         "1.0",
                                         "model");
    const char *semihosting = icmGetVlnvString(vlnvRoot, "arm.ovpworld.org", "semihosting", "armNewlib", "1.0", "model");
    
    // set attributes for CPU model
    icmAttrListObject icmAttr;
    icmAttr.addAttr("endian",        "little");
    icmAttr.addAttr("compatibility", "nopBKPT");
    icmAttr.addAttr("variant",       "Cortex-M3");
    icmAttr.addAttr("UAL",           "1");
    
    icmProcessorObject processor(
        "cpu-Cortex-M3",             // CPU name
        "armm",             // CPU type
        0,                  // CPU cpuId
        0,                  // CPU model flags
        32,                 // address bits
        model,              // model file
        "modelAttrs",       // morpher attributes
        gdb?ICM_ATTR_DEFAULT:SIM_ATTRS,   // simulation attributes
        &icmAttr,                  // user-defined attributes
        semihosting,        // semi-hosting file
        "modelAttrs"        // semi-hosting attributes
    );
    
//    if (flaky) {
        createFlakyMem(processor, lowAddr, highAddr, vlnvRoot2);
 //   }
    
    if (gdb) {
        processor.debugThisProcessor();
    }
    
    // load the processor object file
    processor.loadLocalMemory(application, true, true, true);
    
    return processor;
}


// Main simulation routine
int main(int argc, char ** argv) {
    const char *application = "application.elf";
    bool gdb = false;
    Uns32 portNum = (Uns32)-1;
    
    if(argc >= 2) {
        application = argv[1];
        if(argc >= 3) {
            gdb = true;
            portNum = (Uns32)atoi(argv[2]);
        }
    }
    
    icmPlatform platform(
        "fiPlatformCpp",
        ICM_VERBOSE | ICM_STOP_ON_CTRLC,
        gdb ? "rsp" : 0,
        gdb ? portNum : 0
    );
    icmProcessorObject processor = createPlatform(application, gdb);
    
    simulateUntilBP(processor);
    
    return 0;
}
