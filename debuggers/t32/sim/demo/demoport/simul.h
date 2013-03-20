
/**************************************************************************

	Basic types and includes
	
**************************************************************************/

#ifdef WIN32

#ifndef _WINDOWS_
#include <windows.h>
#endif

typedef __int64 simulTime;
typedef unsigned __int64 simulWord64;

#define SIMULAPI __cdecl

#else

#include "stdarg.h"

#ifdef NSC32000
typedef QUAD simulTime; 
typedef QUAD simulWord64;
#else
typedef long long simulTime;
typedef unsigned long long simulWord64;
#endif

#define SIMULAPI

#endif


typedef unsigned int simulWord32;

#define SIMUL_VERSION		1

#ifdef SIMUL_ARCHITECTURE_64BIT
#define SIMUL_ARCHITECTURE_WIDTH	8
typedef simulWord64 simulWord;
#else
#define SIMUL_ARCHITECTURE_WIDTH	4
typedef simulWord32 simulWord;
#endif

typedef void * simulProcessor;
typedef void * simulPtr;


/**************************************************************************

	Internal callback structure (DO NOT ACCESS THIS STRUCTURE)
	
**************************************************************************/

typedef struct
{
    int endianess;
    int addresswidth;
    int datawidth;
    int processortype;
    int id;
    int resvd1;
    int resvd2;
    int resvd3;

    void (SIMULAPI *printf)();
    void (SIMULAPI *warning)();
    void (SIMULAPI *stop)();
    void (SIMULAPI *update)();
    void * (SIMULAPI *alloc)();
    void (SIMULAPI *free)();
    void * (SIMULAPI *registerCommandCallback)();
    void * (SIMULAPI *registerResetCallback)();
    void * (SIMULAPI *registerExitCallback)();

    void (SIMULAPI *getClockFrequency)();
    void (SIMULAPI *setClockFrequency)();
    void (SIMULAPI *getClockCycle)();
    void (SIMULAPI *setClockCycle)();
    void (SIMULAPI *getTime)();
    void (SIMULAPI *getClock)();
    void (SIMULAPI *stall)();
    void (SIMULAPI *startTimer)();
    void (SIMULAPI *stopTimer)();
    void (SIMULAPI *stopAllTimer)();
    void * (SIMULAPI *registerTimerCallback)();

    int (SIMULAPI *readMemory32)();
    int (SIMULAPI *readMemory64)();
    int (SIMULAPI *writeMemory32)();
    int (SIMULAPI *writeMemory64)();
    void * (SIMULAPI *registerBusReadCallback)();
    void * (SIMULAPI *registerBusWriteCallback)();
    void (SIMULAPI *relocateBusCallback)();

    int (SIMULAPI *getPort32)();
    int (SIMULAPI *getPort64)();
    int (SIMULAPI *setPort32)();
    int (SIMULAPI *setPort64)();
    void * (SIMULAPI *registerPortChangeCallback)();

    int (SIMULAPI *createSharedResource)();
    int (SIMULAPI *readSharedResource)();
    int (SIMULAPI *writeSharedResource)();

    void * (SIMULAPI *openFile)();
    void (SIMULAPI *closeFile)();
    int (SIMULAPI *readFile)();
    int (SIMULAPI *writeFile)();
    int (SIMULAPI *readlineFile)();
    int (SIMULAPI *writelineFile)();
    long (SIMULAPI *seekFile)();

    int (SIMULAPI *readTerminal)();
    int (SIMULAPI *writeTerminal)();
    int (SIMULAPI *stateTerminal)();
    void * (SIMULAPI *registerTerminalCallback)();

    void * (SIMULAPI *registerGoCallback)();
    void * (SIMULAPI *registerBreakCallback)();
}
simulProcInfo;


/**************************************************************************

	Callback structure
	
**************************************************************************/

#define SIMUL_CALLBACK_INIT		0x0000
#define SIMUL_CALLBACK_BUSREAD		0x0001
#define SIMUL_CALLBACK_BUSWRITE		0x0002
#define SIMUL_CALLBACK_PORTCHANGE	0x0004
#define SIMUL_CALLBACK_TIMER		0x0010
#define SIMUL_CALLBACK_EXIT		0x0020
#define SIMUL_CALLBACK_RESET		0x0040
#define SIMUL_CALLBACK_TERMINAL		0x0080
#define SIMUL_CALLBACK_COMMAND		0x0100
#define SIMUL_CALLBACK_GO		0x0200
#define SIMUL_CALLBACK_BREAK		0x0400

typedef struct
{
    int version;
    char * modelname;
    char * commandline;
    int argc;
    char ** argp;
    int * argpint;
    simulWord * argpword;
    simulWord32 * argpword32;
    simulWord64 * argpword64;
    int * argpport;
    int * argpbustype;
    simulWord * argpaddress;
    simulWord32 * argpaddress32;
    simulWord64 * argpaddress64;
    simulTime * argptime;
}
simulParamCallbackStruct;

typedef struct
{
    simulWord address;
    simulWord data;
    int width;
    int cycletype;
    int clocks;
}
simulBusCallbackStruct;

typedef struct
{
    simulWord32 address;
    simulWord32 data;
    int width;
    int cycletype;
    int clocks;
}
simulBusCallbackStruct32;

typedef struct
{
    simulWord64 address;
    simulWord64 data;
    int width;
    int cycletype;
    int clocks;
}
simulBusCallbackStruct64;

typedef struct
{
    simulWord newdata;
    simulWord olddata;
}
simulPortCallbackStruct;

typedef struct
{
    simulWord32 newdata;
    simulWord32 olddata;
}
simulPortCallbackStruct32;

typedef struct
{
    simulWord64 newdata;
    simulWord64 olddata;
}
simulPortCallbackStruct64;

typedef struct
{
    int data;
}
simulTerminalCallbackStruct;

typedef struct
{
    int type;
    simulTime time;
    union {
	simulParamCallbackStruct init;
	simulParamCallbackStruct command;
	simulBusCallbackStruct bus;
	simulBusCallbackStruct32 bus32;
	simulBusCallbackStruct64 bus64;
	simulPortCallbackStruct port;
	simulPortCallbackStruct32 port32;
	simulPortCallbackStruct64 port64;
	simulTerminalCallbackStruct terminal;
    } x;
}
simulCallbackStruct;


typedef int (SIMULAPI * simulCallbackFunctionPtr)(simulProcessor processor, simulCallbackStruct *, simulPtr private);

#ifdef	__cplusplus
extern "C" {
#endif


/**************************************************************************

	Model entry definition
	
**************************************************************************/

#define SIMUL_INIT_OK			1
#define SIMUL_INIT_FAIL			-1

extern int SIMULAPI SIMUL_Init(simulProcessor processor, simulCallbackStruct * cbs);



/**************************************************************************

	Generic configuration functions
	
**************************************************************************/

extern void SIMULAPI SIMUL_Printf(simulProcessor processor, const char *format, ...);

extern void SIMULAPI SIMUL_Warning(simulProcessor processor, const char *format, ...);

extern void SIMULAPI SIMUL_Stop(simulProcessor processor);

extern void SIMULAPI SIMUL_Update(simulProcessor processor, int flags);

extern void * SIMULAPI SIMUL_Alloc(simulProcessor processor, int size);

extern void SIMULAPI SIMUL_Free(simulProcessor processor, void * ptr);


#define SIMUL_ENDIANESS_LITTLE		0
#define SIMUL_ENDIANESS_BIG		1

extern int SIMULAPI SIMUL_GetEndianess(simulProcessor processor);


#define SIMUL_COMMAND_OK		1
#define SIMUL_COMMAND_FAIL		-1

extern void * SIMULAPI SIMUL_RegisterCommandCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private);


#define SIMUL_RESET_OK			1

extern void * SIMULAPI SIMUL_RegisterResetCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private);


#define SIMUL_EXIT_OK			1

extern void * SIMULAPI SIMUL_RegisterExitCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private);


#define SIMUL_GO_OK			1

extern void * SIMULAPI SIMUL_RegisterGoCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private);


#define SIMUL_BREAK_OK			1

extern void * SIMULAPI SIMUL_RegisterBreakCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private);



/**************************************************************************

	Clocks and Timers
	
**************************************************************************/

#define SIMUL_TIMER_OK			1


#define SIMUL_CLOCK_CORE		0
#define SIMUL_CLOCK_BUS			1
#define SIMUL_CLOCK_POWERPC_TIMEBASE	2


extern void SIMULAPI SIMUL_GetClockFrequency(simulProcessor processor, int clockid, simulWord64 * pfrequency);

extern void SIMULAPI SIMUL_SetClockFrequency(simulProcessor processor, int clockid, simulWord64 * pfrequency);

extern void SIMULAPI SIMUL_GetClockCycle(simulProcessor processor, int clockid, simulTime * ptime);

extern void SIMULAPI SIMUL_SetClockCycle(simulProcessor processor, int clockid, simulTime * ptime);

extern void SIMULAPI SIMUL_GetTime(simulProcessor processor, simulTime * ptime);

extern void SIMULAPI SIMUL_GetClock(simulProcessor processor, int clockid, simulTime * ptime);

#define SIMUL_STALL_ABS		0x000
#define SIMUL_STALL_REL		0x100
#define SIMUL_STALL_CLOCKS	0x400

extern void SIMULAPI SIMUL_Stall(simulProcessor processor, int mode, simulTime * ptime);


/* timer modes */

#define SIMUL_TIMER_ABS		0x000
#define SIMUL_TIMER_REL		0x100
#define SIMUL_TIMER_SINGLE	0x000
#define SIMUL_TIMER_REPEAT	0x200
#define SIMUL_TIMER_CLOCKS	0x400


extern void SIMULAPI SIMUL_StartTimer(simulProcessor processor, void * timerid, int mode, simulTime * ptime);

extern void SIMULAPI SIMUL_StopTimer(simulProcessor processor, void * timerid);

extern void SIMULAPI SIMUL_StopAllTimer(simulProcessor processor);

extern void * SIMULAPI SIMUL_RegisterTimerCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private);



/**************************************************************************

	Buses and Memories
	
**************************************************************************/

/* return values for bus callback functions */

#define SIMUL_MEMORY_CONTINUE		0
#define SIMUL_MEMORY_OK			1
#define SIMUL_MEMORY_FAIL		-1

/* cycle types */

#define SIMUL_MEMORY_HIDDEN		0x00
#define SIMUL_MEMORY_FETCH		0x02
#define SIMUL_MEMORY_DATA		0x04
#define SIMUL_MEMORY_DMA		0x08


#define SIMUL_MEMORY_DEFAULT		0

#define SIMUL_MEMORY_POWERPC_SPR	1
#define SIMUL_MEMORY_POWERPC_DCR	2

#define SIMUL_MEMORY_ARM_CPX		1

#define SIMUL_MEMORY_SH_INTACK		255
#define SIMUL_MEMORY_M68K_INTACK	255


extern int SIMULAPI SIMUL_ReadMemory(simulProcessor processor, int bustype, simulWord * paddress, int width, int cycletype, simulWord * pdata);

extern int SIMULAPI SIMUL_WriteMemory(simulProcessor processor, int bustype, simulWord * paddress, int width, int cycletype, simulWord * pdata);


extern void * SIMULAPI SIMUL_RegisterBusReadCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private, int bustype, simulWord * paddressFrom, simulWord * paddressTo);

extern void * SIMULAPI SIMUL_RegisterBusWriteCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private, int bustype, simulWord * paddressFrom, simulWord * paddressTo);

extern void SIMULAPI SIMUL_RelocateBusCallback(simulProcessor processor, void * callbackid, int bustype, simulWord * paddressFrom, simulWord * paddressTo);


extern void SIMULAPI SIMUL_InsertWord(simulProcessor processor, simulWord * ptarget, int wordwidth, simulWord * paddress, int width, simulWord * pdata);

extern void SIMULAPI SIMUL_ExtractWord(simulProcessor processor, simulWord * psource, int wordwidth, simulWord * paddress, int width, simulWord * pdata);


extern void SIMULAPI SIMUL_SaveWord(simulProcessor processor, void * ptarget, int width, simulWord * pdata);

extern void SIMULAPI SIMUL_LoadWord(simulProcessor processor, void * psource, int width, simulWord * pdata);



/**************************************************************************

	Ports
	
**************************************************************************/

/* predefined core ports */

#define SIMUL_PORT_RESET		-1
#define SIMUL_PORT_INTERRUPT		-2

#define SIMUL_PORT_PPC_INT		-2

#define SIMUL_PORT_ARM_IRQ		-2
#define SIMUL_PORT_ARM_FIQ		-3

#define SIMUL_PORT_SH_IRL0		-2
#define SIMUL_PORT_SH_IRL1		-3
#define SIMUL_PORT_SH_IRL2		-4
#define SIMUL_PORT_SH_IRL3		-5

#define SIMUL_PORT_MIPS_NMI		-2
#define SIMUL_PORT_MIPS_INT0		-3
#define SIMUL_PORT_MIPS_INT1		-4
#define SIMUL_PORT_MIPS_INT2		-5
#define SIMUL_PORT_MIPS_INT3		-6
#define SIMUL_PORT_MIPS_INT4		-7
#define SIMUL_PORT_MIPS_INT5		-8

#define SIMUL_PORT_M68K_IRL0		-2
#define SIMUL_PORT_M68K_IRL1		-3
#define SIMUL_PORT_M68K_IRL2		-4

#define SIMUL_PORT_C166_IRL0		-2
#define SIMUL_PORT_C166_IRL1		-3
#define SIMUL_PORT_C166_IRL2		-4
#define SIMUL_PORT_C166_IRL3		-5
#define SIMUL_PORT_C166_NMI		-6

/* return values for port callback functions */

#define SIMUL_PORT_OK			1


extern int SIMULAPI SIMUL_GetPort(simulProcessor processor, int offset, int width, simulWord * pdata);

extern int SIMULAPI SIMUL_SetPort(simulProcessor processor, int offset, int width, simulWord * pdata);

extern void * SIMULAPI SIMUL_RegisterPortChangeCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private, int offset, int width);



/**************************************************************************

	Shared Resources
	
**************************************************************************/

extern int SIMULAPI SIMUL_CreateSharedResource(simulProcessor processor, int size);

extern int SIMULAPI SIMUL_ReadSharedResource(simulProcessor processor, int offset, int len, void * pdata);

extern int SIMULAPI SIMUL_WriteSharedResource(simulProcessor processor, int offset, int len, void * pdata);



/**************************************************************************

	File I/O
	
**************************************************************************/

/* file modes */

#define SIMUL_FILE_READ		0x001
#define SIMUL_FILE_WRITE	0x002
#define SIMUL_FILE_CREATE	0x010
#define SIMUL_FILE_APPEND	0x020
#define SIMUL_FILE_BINARY	0x000
#define SIMUL_FILE_ASCII	0x100

extern void * SIMULAPI SIMUL_OpenFile(simulProcessor processor, const char * filename, int mode);

extern void SIMULAPI SIMUL_CloseFile(simulProcessor processor, void * file);

extern int SIMULAPI SIMUL_ReadFile(simulProcessor processor, void * file, void * pdata, int length);

extern int SIMULAPI SIMUL_WriteFile(simulProcessor processor, void * file, void * pdata, int length);

extern int SIMULAPI SIMUL_ReadlineFile(simulProcessor processor, void * file, void * pdata, int length);

extern int SIMULAPI SIMUL_WritelineFile(simulProcessor processor, void * file, void * pdata);

#define SIMUL_FILE_SEEKABS	0
#define SIMUL_FILE_SEEKREL	1
#define SIMUL_FILE_SEEKEND	2

extern long SIMULAPI SIMUL_SeekFile(simulProcessor processor, void * file, long pos, int mode);




/**************************************************************************

	Terminal I/O
	
**************************************************************************/

/* status bits */

#define SIMUL_STATE_RXREADY	0x001
#define SIMUL_STATE_TXREADY	0x002
#define SIMUL_STATE_NOTEXISTING	0x010

extern int SIMULAPI SIMUL_ReadTerminal(simulProcessor processor, int id);

extern int SIMULAPI SIMUL_WriteTerminal(simulProcessor processor, int id, int ch);

extern int SIMULAPI SIMUL_StateTerminal(simulProcessor processor, int id);

extern void * SIMUL_RegisterTerminalCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private, int id);


/**************************************************************************

	End of Include
	
**************************************************************************/

#ifdef	__cplusplus
}
#endif
