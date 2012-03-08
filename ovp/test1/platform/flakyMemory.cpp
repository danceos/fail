#include "flakyMemory.hpp"


void createFlakyMem(
    icmProcessorObject processor,
    Addr lowAddr,
    Addr highAddr,
    const char *vlnvRoot
) {
    
    icmBusObject *mainBus;
    icmBusObject *interBus;
    icmBusObject *mmcBus;
    icmMmcObject *mmc1;
    icmMemoryObject *memory1;
    icmMemoryObject *memory2;
    icmMemoryObject *memory3;
    
#if 0
    // get location of mmc object file:
    const char *flakyMem = icmGetVlnvString(
        vlnvRoot,
        "ovpworld.org",
        "mmc",
        "flakyMemory",
        "1.0",
        "model"
    );
#endif 
    // create full MMC
    mmc1 = new icmMmcObject("mmc1", "/srv/scratch/sirozipp/build/lib/libflaky.so", "modelAttrs", 0, False);
    
    // create the processor bus
    mainBus = new icmBusObject("bus1", 32);
    
    // create the intermediate bus
    interBus = new icmBusObject("bus2", 32);
    
    // create the bus connecting the mmc to the memory
    mmcBus = new icmBusObject("bus3", 32);
    
    // connect mmc direct to processor ports
    processor.connect(*mainBus, *mainBus);
    
    // connect master port of MMC to bus
    mmc1->connect(*interBus, "sp1", False);
    mmc1->connect(*mmcBus, "mp1", True);
    
    if (lowAddr != 0x00) {
        memory1 = new icmMemoryObject("mem1", ICM_PRIV_RWX, lowAddr-1);
        memory1->connect("memp1", *mainBus, 0x00);
    }
    memory2 = new icmMemoryObject("mem2", ICM_PRIV_RWX, highAddr-lowAddr);
    memory2->connect("memp2", *mmcBus, lowAddr);
    if (highAddr != 0xffffffff) {
        memory3 = new icmMemoryObject("mem3", ICM_PRIV_RWX, 0xffffffff-(highAddr+1));
        memory3->connect("memp3", *mainBus, highAddr+1);
    }
    
    mainBus->newBridge(*interBus, "br1", "sp2", "mp2", lowAddr, highAddr, lowAddr);
    
    // show the bus connections
    mainBus->printConnections();
    interBus->printConnections();
    mmcBus->printConnections();
}
