
#include "simul.h"


/**************************************************************************

	Offsets and width of the control registers
	
**************************************************************************/

#define DEMOPORT_WIDTH		4
#define DEMOPORT_DATA1REG	0
#define DEMOPORT_DATA2REG	1*DEMOPORT_WIDTH
#define DEMOPORT_DATA3REG	2*DEMOPORT_WIDTH
#define DEMOPORT_DATA5REG	3*DEMOPORT_WIDTH
#define DEMOPORT_DATA6REG	3*DEMOPORT_WIDTH+1
#define DEMOPORT_DATA6CREG	3*DEMOPORT_WIDTH+2
#define DEMOPORT_TIMER1REG	4*DEMOPORT_WIDTH
#define DEMOPORT_TIMER2REG	5*DEMOPORT_WIDTH
#define DEMOPORT_TIMER3REG	6*DEMOPORT_WIDTH
#define DEMOPORT_RAM1BASE	0x200


/**************************************************************************

	Local port structure
	
**************************************************************************/

typedef struct
{
    int bustype;
    simulWord startaddress;
    int baseport;

    simulWord port2data;

    simulWord port4data;
    void * port4readcallback;
    void * port4writecallback;

    void * port5readhandle;
    void * port5writehandle;
    simulWord port5data;
    
    int port6txint;
    int port6rxint;
    int port6txintpending;
    int port6rxintpending;

    void * timer1id;
    
    simulWord timer2value;

    void * timer3id;
    int timer3running;
    simulWord timer3startvalue;
    simulTime timer3startclock;
}
demoPort;


/**************************************************************************

	Data1 demonstrates a port which is directly connected to simulator ports.
	The callback is called for reads and writes to the port.
	
**************************************************************************/

static int SIMULAPI PortWriteData1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    SIMUL_SetPort(processor, demoportptr->baseport + DEMOPORT_DATA1REG * 8, cbs->x.bus.width, &cbs->x.bus.data);
    cbs->x.bus.clocks += 4;		/* add 4 waitstates */
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReadData1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    SIMUL_GetPort(processor, demoportptr->baseport + DEMOPORT_DATA1REG * 8, cbs->x.bus.width, &cbs->x.bus.data);
    cbs->x.bus.clocks += 4;		/* add 4 waitstates */
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortResetData1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulWord       data;
    demoPort       *demoportptr = (demoPort *) private;

    data = -1;
    SIMUL_SetPort(processor, demoportptr->baseport + DEMOPORT_DATA1REG * 8, DEMOPORT_WIDTH * 8, &data);

    return SIMUL_RESET_OK;
}


static void PortInitData1(simulProcessor processor, demoPort * demoportptr)
{
    simulWord       from, to;

    SIMUL_RegisterResetCallback(processor, PortResetData1, (simulPtr) demoportptr);

    from = demoportptr->startaddress + DEMOPORT_DATA1REG;
    to = demoportptr->startaddress + DEMOPORT_DATA1REG + DEMOPORT_WIDTH - 1;

    SIMUL_RegisterBusWriteCallback(processor, PortWriteData1, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    SIMUL_RegisterBusReadCallback(processor, PortReadData1, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);
}




/**************************************************************************

	Data2 demonstrates a port which holds its data local.
	The callback is called for reads and writes to the port.
	Only the correct bussize is allowed.
	
**************************************************************************/

static int SIMULAPI PortWriteData2(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    if (cbs->x.bus.width != DEMOPORT_WIDTH * 8)
	return SIMUL_MEMORY_FAIL;
    demoportptr->port2data = cbs->x.bus.data;
    cbs->x.bus.clocks = 0;	/* 0-cycle access */
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReadData2(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    if (cbs->x.bus.width != DEMOPORT_WIDTH * 8)
	return SIMUL_MEMORY_FAIL;
    cbs->x.bus.data = demoportptr->port2data;
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortResetData2(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    demoportptr->port2data = 0;

    return SIMUL_RESET_OK;
}


static void PortInitData2(simulProcessor processor, demoPort * demoportptr)
{
    simulWord       from, to;

    demoportptr->port2data = 0;

    SIMUL_RegisterResetCallback(processor, PortResetData2, (simulPtr) demoportptr);

    from = demoportptr->startaddress + DEMOPORT_DATA2REG;
    to = demoportptr->startaddress + DEMOPORT_DATA2REG + DEMOPORT_WIDTH - 1;

    SIMUL_RegisterBusWriteCallback(processor, PortWriteData2, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    SIMUL_RegisterBusReadCallback(processor, PortReadData2, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);
}




/**************************************************************************

	Data3 demonstrates a port which holds global data (same for multiple instances).
	The callback is called for reads and writes to the port.
	All bussizes are supported.
	
**************************************************************************/

static simulWord Port3Data;

static int SIMULAPI PortWriteData3(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    SIMUL_InsertWord(processor, &Port3Data, DEMOPORT_WIDTH*8, &cbs->x.bus.address, cbs->x.bus.width, &cbs->x.bus.data);
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReadData3(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    SIMUL_ExtractWord(processor, &Port3Data, DEMOPORT_WIDTH*8, &cbs->x.bus.address, cbs->x.bus.width, &cbs->x.bus.data);
    return SIMUL_MEMORY_OK;
}


static void PortInitData3(simulProcessor processor, demoPort * demoportptr)
{
    simulWord       from, to;

    from = demoportptr->startaddress + DEMOPORT_DATA3REG;
    to = demoportptr->startaddress + DEMOPORT_DATA3REG + DEMOPORT_WIDTH - 1;

    SIMUL_RegisterBusWriteCallback(processor, PortWriteData3, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    SIMUL_RegisterBusReadCallback(processor, PortReadData3, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);
}


    
/**************************************************************************

	Data4 demonstrates a port with a dynamic address.
	The value written to the port is the address of the port.
	The lower 8 address bits of the address are zero.
	The callback is called for reads and writes to the port.
	
**************************************************************************/

static int SIMULAPI PortWriteData4(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;
    simulWord       from, to;

    SIMUL_InsertWord(processor, &demoportptr->port4data, DEMOPORT_WIDTH * 8, &cbs->x.bus.address, cbs->x.bus.width, &cbs->x.bus.data);
    demoportptr->port4data &= ~0xff;

    from = demoportptr->port4data;
    to = demoportptr->port4data + DEMOPORT_WIDTH - 1;

    SIMUL_RelocateBusCallback(processor, demoportptr->port4readcallback, demoportptr->bustype, &from, &to);

    SIMUL_RelocateBusCallback(processor, demoportptr->port4writecallback, demoportptr->bustype, &from, &to);

    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReadData4(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    SIMUL_ExtractWord(processor, &demoportptr->port4data, DEMOPORT_WIDTH*8, &cbs->x.bus.address, cbs->x.bus.width, &cbs->x.bus.data);
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortResetData4(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulWord       from, to;
    demoPort       *demoportptr = (demoPort *) private;

    demoportptr->port4data = (demoportptr->startaddress & ~0xff) + 0x100;	/* initial address after reset */

    from = demoportptr->port4data;
    to = demoportptr->port4data + DEMOPORT_WIDTH - 1;

    SIMUL_RelocateBusCallback(processor, demoportptr->port4readcallback, demoportptr->bustype, &from, &to);

    SIMUL_RelocateBusCallback(processor, demoportptr->port4writecallback, demoportptr->bustype, &from, &to);

    return SIMUL_RESET_OK;
}


static void PortInitData4(simulProcessor processor, demoPort * demoportptr)
{
    simulWord       from, to;

    demoportptr->port4data = (demoportptr->startaddress & ~0xff) + 0x100;	/* initial address */

    SIMUL_RegisterResetCallback(processor, PortResetData4, (simulPtr) demoportptr);

    from = demoportptr->port4data;
    to = demoportptr->port4data + DEMOPORT_WIDTH - 1;

    demoportptr->port4writecallback = SIMUL_RegisterBusWriteCallback(processor, PortWriteData4, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    demoportptr->port4readcallback = SIMUL_RegisterBusReadCallback(processor, PortReadData4, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);
}



/**************************************************************************

	Data5 demonstrates a port which writes or reads binary data to/from file.
	The callback is called for reads and writes to the port.
	All bussizes are supported.
	
**************************************************************************/

static int SIMULAPI PortWriteData5(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    unsigned char   bdata;
    simulWord       wdata;
    demoPort       *demoportptr = (demoPort *) private;

    SIMUL_InsertWord(processor, &wdata, 8, &cbs->x.bus.address, cbs->x.bus.width, &cbs->x.bus.data);
    bdata = wdata & 0xff;

    if (SIMUL_WriteFile(processor, demoportptr->port5writehandle, &bdata, 1) <= 0)
	return SIMUL_MEMORY_FAIL;

    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReadData5(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    unsigned char   bdata;
    demoPort       *demoportptr = (demoPort *) private;

    if (cbs->x.bus.cycletype) {		/* destructive read only done when CPU/DMA makes access */
	if (!demoportptr->port5readhandle)
	    return SIMUL_MEMORY_FAIL;
	if (SIMUL_ReadFile(processor, demoportptr->port5readhandle, &bdata, 1) <= 0)
	    return SIMUL_MEMORY_FAIL;
	demoportptr->port5data = bdata;
    }

    SIMUL_ExtractWord(processor, &demoportptr->port5data, 8, &cbs->x.bus.address, cbs->x.bus.width, &cbs->x.bus.data);
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortResetData5(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    if (demoportptr->port5readhandle)
	SIMUL_SeekFile(processor, demoportptr->port5readhandle, 0l, SIMUL_FILE_SEEKABS);
    demoportptr->port5data = 0;

    return SIMUL_RESET_OK;
}


static void PortInitData5(simulProcessor processor, demoPort * demoportptr)
{
    simulWord       from, to;

    demoportptr->port5readhandle = SIMUL_OpenFile(processor, "~~~/portread.dat", SIMUL_FILE_READ);
    demoportptr->port5writehandle = SIMUL_OpenFile(processor, "~~~/portwrit.dat", SIMUL_FILE_WRITE|SIMUL_FILE_CREATE);

    demoportptr->port5data = 0;

    SIMUL_RegisterResetCallback(processor, PortResetData5, (simulPtr) demoportptr);

    from = demoportptr->startaddress + DEMOPORT_DATA5REG;
    to = demoportptr->startaddress + DEMOPORT_DATA5REG;

    SIMUL_RegisterBusWriteCallback(processor, PortWriteData5, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    SIMUL_RegisterBusReadCallback(processor, PortReadData5, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);
}



/**************************************************************************

	Terminal1 demonstrates a port which is connected to terminal window #1.
	Terminal1 is the data port, Terminal1C the status and control port.
	The callback is called for reads and writes to the port.
	Bits 6+7 or control enable the interrupts.
	The interrupt drives directly the CPU. For normal applications this
	should go thru an interrupt controller.
	When unloading the model we write a message to the terminal.
	
**************************************************************************/

static int SIMULAPI PortWriteTerminal1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulWord       data;
    demoPort       *demoportptr = (demoPort *) private;

    demoportptr->port6txintpending = 0;
    if (!((demoportptr->port6txintpending && demoportptr->port6txint)
	  || (demoportptr->port6rxintpending && demoportptr->port6rxint))) {
	data = 0;
	SIMUL_SetPort(processor, SIMUL_PORT_INTERRUPT, 1, &data);
    }
    if (SIMUL_WriteTerminal(processor, 1, (int) (cbs->x.bus.data & 0xff)) > 0) {
	demoportptr->port6txintpending = 1;
	if (((demoportptr->port6txintpending && demoportptr->port6txint)
	  || (demoportptr->port6rxintpending && demoportptr->port6rxint))) {
	    data = 1;
	    SIMUL_SetPort(processor, SIMUL_PORT_INTERRUPT, 1, &data);
	}
    }
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReadTerminal1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulWord       data;
    demoPort       *demoportptr = (demoPort *) private;

    if (cbs->x.bus.cycletype) {	/* destructive read only done when CPU/DMA
				 * makes access */
	cbs->x.bus.data = SIMUL_ReadTerminal(processor, 1);
	demoportptr->port6rxintpending = 0;
	if (!((demoportptr->port6txintpending && demoportptr->port6txint)
	  || (demoportptr->port6rxintpending && demoportptr->port6rxint))) {
	    data = 0;
	    SIMUL_SetPort(processor, SIMUL_PORT_INTERRUPT, 1, &data);
	}
	return SIMUL_MEMORY_OK;
    }
    return SIMUL_MEMORY_FAIL;
}


static int SIMULAPI PortWriteTerminal1C(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulWord       data;
    demoPort       *demoportptr = (demoPort *) private;

    demoportptr->port6txint = !!(cbs->x.bus.data & 0x80);
    demoportptr->port6rxint = !!(cbs->x.bus.data & 0x40);

    if (((demoportptr->port6txintpending && demoportptr->port6txint)
	 || (demoportptr->port6rxintpending && demoportptr->port6rxint))) {
	data = 1;
    } else {
	data = 0;
    }
    SIMUL_SetPort(processor, SIMUL_PORT_INTERRUPT, 1, &data);

    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReadTerminal1C(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    cbs->x.bus.data = SIMUL_StateTerminal(processor, 1);
    cbs->x.bus.data |= (demoportptr->port6txint << 7) | (demoportptr->port6rxint << 6);

    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReceiveTerminal1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulWord       data;
    demoPort       *demoportptr = (demoPort *) private;

    demoportptr->port6rxintpending = 1;

    if (((demoportptr->port6txintpending && demoportptr->port6txint)
	 || (demoportptr->port6rxintpending && demoportptr->port6rxint))) {
	data = 1;
	SIMUL_SetPort(processor, SIMUL_PORT_INTERRUPT, 1, &data);
    }

    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortResetTerminal1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    demoportptr->port6txint = 0;
    demoportptr->port6rxint = 0;

    demoportptr->port6txintpending = 0;
    demoportptr->port6rxintpending = 0;

    return SIMUL_RESET_OK;
}


static int SIMULAPI PortExitTerminal1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    int             i;
    demoPort       *demoportptr = (demoPort *) private;
    static const char exitmessage[] = "\r\nunloaded.\r\n";
    
    for (i = 0; exitmessage[i]; i++)
	SIMUL_WriteTerminal(processor, 1, exitmessage[i]);

    return SIMUL_EXIT_OK;
}


static void PortInitTerminal1(simulProcessor processor, demoPort * demoportptr)
{
    simulWord       from, to;

    demoportptr->port6txint = 0;
    demoportptr->port6rxint = 0;

    demoportptr->port6txintpending = 0;
    demoportptr->port6rxintpending = 0;

    SIMUL_RegisterResetCallback(processor, PortResetTerminal1, (simulPtr) demoportptr);

    SIMUL_RegisterExitCallback(processor, PortExitTerminal1, (simulPtr) demoportptr);

    from = demoportptr->startaddress + DEMOPORT_DATA6REG;
    to = demoportptr->startaddress + DEMOPORT_DATA6REG;

    SIMUL_RegisterBusWriteCallback(processor, PortWriteTerminal1, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    SIMUL_RegisterBusReadCallback(processor, PortReadTerminal1, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);


    from = demoportptr->startaddress + DEMOPORT_DATA6CREG;
    to = demoportptr->startaddress + DEMOPORT_DATA6CREG;

    SIMUL_RegisterBusWriteCallback(processor, PortWriteTerminal1C, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    SIMUL_RegisterBusReadCallback(processor, PortReadTerminal1C, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);
    
    SIMUL_RegisterTerminalCallback(processor, PortReceiveTerminal1, (simulPtr) demoportptr, 1);
}



/**************************************************************************

	Ram1 demonstrates multiport RAM between different CPUs.
	The callback is called for reads and writes to the port.
	All bussizes are supported.
	
**************************************************************************/

static int SIMULAPI PortWriteRam1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    int             offset, len;
    unsigned char   data[8];

    len = cbs->x.bus.width / 8;
    offset = cbs->x.bus.address & 0xff;

    SIMUL_SaveWord(processor, data, cbs->x.bus.width, &cbs->x.bus.data);

    if (SIMUL_WriteSharedResource(processor, offset, len, data) < 0)
        return SIMUL_MEMORY_FAIL;
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReadRam1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    int             offset, len;
    unsigned char   data[8];

    len = cbs->x.bus.width / 8;
    offset = cbs->x.bus.address & 0xff;

    if (SIMUL_ReadSharedResource(processor, offset, len, data) < 0)
	return SIMUL_MEMORY_FAIL;

    SIMUL_LoadWord(processor, data, cbs->x.bus.width, &cbs->x.bus.data);
    return SIMUL_MEMORY_OK;
}


static void PortInitRam1(simulProcessor processor, demoPort * demoportptr)
{
    simulWord       from, to;

    SIMUL_CreateSharedResource(processor, 256);

    from = demoportptr->startaddress + DEMOPORT_RAM1BASE;
    to = demoportptr->startaddress + DEMOPORT_RAM1BASE + 0xff - 1;

    SIMUL_RegisterBusWriteCallback(processor, PortWriteRam1, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    SIMUL_RegisterBusReadCallback(processor, PortReadRam1, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);
}

    

/**************************************************************************

	Timer1 demonstrates a timer that increments every <n> clockticks.
	The callback is called for each increment.
	The timervalue is directly kept in simulated memory, no read callback.
	A copy of the timer bits is also available in the ports.
	Bit 0 of the timer is send to port #0xf0 (to trigger Timer2).
	The command "SIM SETTIMER1 <clocks>" can set the period of the timer.
	
**************************************************************************/

static int SIMULAPI PortWriteTimer1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    SIMUL_SetPort(processor, demoportptr->baseport+DEMOPORT_TIMER1REG*8, cbs->x.bus.width, &cbs->x.bus.data);
    SIMUL_SetPort(processor, 0xf0, 1, &cbs->x.bus.data);

    return SIMUL_MEMORY_CONTINUE;	/* to update also the memory location */
}


static int SIMULAPI PortIncTimer1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;
    simulWord       data, address;

    address = demoportptr->startaddress + DEMOPORT_TIMER1REG;

    SIMUL_ReadMemory(processor, demoportptr->bustype, &address, DEMOPORT_WIDTH*8, SIMUL_MEMORY_HIDDEN, &data);
    data++;

    /* note: this write will cause the PortWriteTimer1 callback to be called */

    SIMUL_WriteMemory(processor, demoportptr->bustype, &address, DEMOPORT_WIDTH*8, SIMUL_MEMORY_HIDDEN, &data);
    
    return SIMUL_TIMER_OK;
}


static int SIMULAPI PortCommandTimer1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulTime time;
    demoPort       *demoportptr = (demoPort *) private;
    
    if (!strcmp(cbs->x.command.argp[0], "SETTIMER1")) {
	if (cbs->x.command.argc != 2 || cbs->x.command.argpint[1] <= 0) {
	    SIMUL_Warning(processor, "usage: SETTIMER1 <clocks>");
	    return SIMUL_COMMAND_FAIL;
	}
	time = cbs->x.command.argpint[1];
	SIMUL_StartTimer(processor, demoportptr->timer1id, SIMUL_TIMER_REPEAT | SIMUL_TIMER_REL | SIMUL_TIMER_CLOCKS, &time);
    }

    return SIMUL_COMMAND_OK;
}


static int SIMULAPI PortResetTimer1(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulTime       time;
    demoPort       *demoportptr = (demoPort *) private;

    time = 100;
    SIMUL_StartTimer(processor, demoportptr->timer1id, SIMUL_TIMER_REPEAT | SIMUL_TIMER_REL | SIMUL_TIMER_CLOCKS, &time);

    return SIMUL_RESET_OK;
}


static void PortInitTimer1(simulProcessor processor, demoPort * demoportptr)
{
    simulTime       time;
    simulWord       from, to;

    SIMUL_RegisterResetCallback(processor, PortResetTimer1, (simulPtr) demoportptr);

    SIMUL_RegisterCommandCallback(processor, PortCommandTimer1, (simulPtr) demoportptr);

    from = demoportptr->startaddress + DEMOPORT_TIMER1REG;
    to = demoportptr->startaddress + DEMOPORT_TIMER1REG + DEMOPORT_WIDTH - 1;

    SIMUL_RegisterBusWriteCallback(processor, PortWriteTimer1, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    demoportptr->timer1id = SIMUL_RegisterTimerCallback(processor, PortIncTimer1, (simulPtr) demoportptr);

    time = 100;
    SIMUL_StartTimer(processor, demoportptr->timer1id, SIMUL_TIMER_REPEAT | SIMUL_TIMER_REL | SIMUL_TIMER_CLOCKS, &time);
}



/**************************************************************************

	Timer2 demonstrates a timer that counts rising edges of port #0xf0.
	Any write access resets the timer to 0.
	The read callback does not take care about the buswidth and
	will return wrong values for wrong bussizes (but it is faster).
	
**************************************************************************/

static int SIMULAPI PortWriteTimer2(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    demoportptr->timer2value = 0;
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReadTimer2(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    cbs->x.bus.data = demoportptr->timer2value;
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortChangeTimer2(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    if (cbs->x.port.newdata)
	demoportptr->timer2value++;

    return SIMUL_PORT_OK;
}


static int SIMULAPI PortResetTimer2(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    demoportptr->timer2value = 0;

    return SIMUL_RESET_OK;
}


static void PortInitTimer2(simulProcessor processor, demoPort * demoportptr)
{
    simulWord       from, to;

    demoportptr->timer2value = 0;

    SIMUL_RegisterResetCallback(processor, PortResetTimer2, (simulPtr) demoportptr);

    from = demoportptr->startaddress + DEMOPORT_TIMER2REG;
    to = demoportptr->startaddress + DEMOPORT_TIMER2REG + DEMOPORT_WIDTH - 1;

    SIMUL_RegisterBusWriteCallback(processor, PortWriteTimer2, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    SIMUL_RegisterBusReadCallback(processor, PortReadTimer2, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    SIMUL_RegisterPortChangeCallback(processor, PortChangeTimer2, (simulPtr) demoportptr, 0xf0, 1);
}



/**************************************************************************

	Timer3 demonstrates a timer that decrements every clocktick.
	A write to the port starts the timer.
	The timer send an interrupt to the CPU when the value reaches 0.
	The callback is only called when the value reaches 0.
	The read callback calculates the expected value.
	
**************************************************************************/

static int SIMULAPI PortWriteTimer3(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulTime       clocks;
    demoPort       *demoportptr = (demoPort *) private;

    if (!demoportptr->timer3id)
	return SIMUL_MEMORY_FAIL;

    demoportptr->timer3running = 1;
    demoportptr->timer3startvalue = cbs->x.bus.data;
    SIMUL_GetClock(processor, 0, &demoportptr->timer3startclock);

    clocks = demoportptr->timer3startvalue;

    SIMUL_StartTimer(processor, demoportptr->timer3id, SIMUL_TIMER_REL | SIMUL_TIMER_CLOCKS, &clocks);

    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortReadTimer3(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulTime       clocks;
    demoPort       *demoportptr = (demoPort *) private;

    if (!demoportptr->timer3running) {
	cbs->x.bus.data = 0;
	return SIMUL_MEMORY_OK;
    }

    SIMUL_GetClock(processor, 0, &clocks);
    clocks = clocks-demoportptr->timer3startclock;

    cbs->x.bus.data = demoportptr->timer3startvalue - (simulWord) clocks;
    return SIMUL_MEMORY_OK;
}


static int SIMULAPI PortCallTimer3(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    simulWord       data;
    demoPort       *demoportptr = (demoPort *) private;

    demoportptr->timer3running = 0;

    /* set the interrupt line */

    data = 1;
    SIMUL_SetPort(processor, SIMUL_PORT_INTERRUPT, 1, &data);

    return SIMUL_TIMER_OK;
}


static int SIMULAPI PortResetTimer3(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    demoPort       *demoportptr = (demoPort *) private;

    SIMUL_StopTimer(processor, demoportptr->timer3id);
    demoportptr->timer3running = 0;

    return SIMUL_RESET_OK;
}


static void PortInitTimer3(simulProcessor processor, demoPort * demoportptr)
{
    simulWord       from, to;

    demoportptr->timer3running = 0;

    SIMUL_RegisterResetCallback(processor, PortResetTimer3, (simulPtr) demoportptr);

    from = demoportptr->startaddress + DEMOPORT_TIMER3REG;
    to = demoportptr->startaddress + DEMOPORT_TIMER3REG + DEMOPORT_WIDTH - 1;

    SIMUL_RegisterBusWriteCallback(processor, PortWriteTimer3, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    SIMUL_RegisterBusReadCallback(processor, PortReadTimer3, (simulPtr) demoportptr, demoportptr->bustype, &from, &to);

    demoportptr->timer3id = SIMUL_RegisterTimerCallback(processor, PortCallTimer3, (simulPtr) demoportptr);
}



/**************************************************************************

	Entry point of the Model

**************************************************************************/

int SIMULAPI SIMUL_Init(simulProcessor processor, simulCallbackStruct * cbs)
{
    demoPort       *demoportptr;

    strcpy(cbs->x.init.modelname, __DATE__ "   Demo Port");

    demoportptr = (demoPort *) SIMUL_Alloc(processor, sizeof(demoPort));

    if (cbs->x.init.argc != 3) {
	SIMUL_Warning(processor, "parameters: <address> <portnumber>");
	return SIMUL_INIT_FAIL;
    }
    demoportptr->bustype = cbs->x.init.argpbustype[1];
    demoportptr->startaddress = cbs->x.init.argpaddress[1];

    demoportptr->baseport = cbs->x.init.argpport[2];

    PortInitData1(processor, demoportptr);
    PortInitData2(processor, demoportptr);
    PortInitData3(processor, demoportptr);
    PortInitData4(processor, demoportptr);
    PortInitData5(processor, demoportptr);
    PortInitTerminal1(processor, demoportptr);
    PortInitRam1(processor, demoportptr);

    PortInitTimer1(processor, demoportptr);
    PortInitTimer2(processor, demoportptr);
    PortInitTimer3(processor, demoportptr);

    return SIMUL_INIT_OK;
}
