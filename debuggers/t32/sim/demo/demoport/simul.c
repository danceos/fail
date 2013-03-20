
#include "simul.h"


/**************************************************************************

	Model entry
	
**************************************************************************/

int SIMULAPI SIMUL_Interface(simulProcessor processor, simulCallbackStruct * cbs)
{
    cbs->x.init.version = SIMUL_VERSION;
#if SIMUL_ARCHITECTURE_WIDTH == 8
    cbs->x.init.argpword = cbs->x.init.argpword64;
    cbs->x.init.argpaddress = cbs->x.init.argpaddress64;
#else
    cbs->x.init.argpword = cbs->x.init.argpword32;
    cbs->x.init.argpaddress = cbs->x.init.argpaddress32;
#endif
    return SIMUL_Init(processor, cbs);
}


/**************************************************************************

	Generic configuration functions
	
**************************************************************************/

void SIMULAPI SIMUL_Printf(simulProcessor processor, const char *format, ...)
{
    va_list         ap;

    va_start(ap, format);
    (*((simulProcInfo *) processor)->printf) (processor, format, ap);
    va_end(ap);
}


void SIMULAPI SIMUL_Warning(simulProcessor processor, const char *format, ...)
{
    va_list         ap;

    va_start(ap, format);
    (*((simulProcInfo *) processor)->warning) (processor, format, ap);
    va_end(ap);
}


void SIMULAPI SIMUL_Stop(simulProcessor processor)
{
    (*((simulProcInfo *) processor)->stop) (processor);
}


void SIMULAPI SIMUL_Update(simulProcessor processor, int flags)
{
    (*((simulProcInfo *) processor)->update) (processor, flags);
}


void * SIMULAPI SIMUL_Alloc(simulProcessor processor, int size)
{
    return (*((simulProcInfo *) processor)->alloc)(processor, size);
}


void SIMULAPI SIMUL_Free(simulProcessor processor, void * ptr)
{
    (*((simulProcInfo *) processor)->free)(processor, ptr);
}


int SIMULAPI SIMUL_GetEndianess(simulProcessor processor)
{
    return ((simulProcInfo *) processor)->endianess;
}


void * SIMULAPI SIMUL_RegisterCommandCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private)
{
    return (*((simulProcInfo *) processor)->registerCommandCallback) (processor, func, private, SIMUL_ARCHITECTURE_WIDTH);
}


void * SIMULAPI SIMUL_RegisterResetCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private)
{
    return (*((simulProcInfo *) processor)->registerResetCallback)(processor, func, private);
}


void * SIMULAPI SIMUL_RegisterExitCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private)
{
    return (*((simulProcInfo *) processor)->registerExitCallback)(processor, func, private);
}


void * SIMULAPI SIMUL_RegisterGoCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private)
{
    return (*((simulProcInfo *) processor)->registerGoCallback)(processor, func, private);
}


void * SIMULAPI SIMUL_RegisterBreakCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private)
{
    return (*((simulProcInfo *) processor)->registerBreakCallback)(processor, func, private);
}


/**************************************************************************

	Clocks and Timers
	
**************************************************************************/

void SIMULAPI SIMUL_GetClockFrequency(simulProcessor processor, int id, simulWord64 * pfrequency)
{
    (*((simulProcInfo *) processor)->getClockFrequency)(processor, id, pfrequency);
}


void SIMULAPI SIMUL_SetClockFrequency(simulProcessor processor, int id, simulWord64 * pfrequency)
{
    (*((simulProcInfo *) processor)->setClockFrequency)(processor, id,  pfrequency);
}


void SIMULAPI SIMUL_GetClockCycle(simulProcessor processor, int id, simulTime * ptime)
{
    (*((simulProcInfo *) processor)->getClockCycle)(processor, id, ptime);
}


void SIMULAPI SIMUL_SetClockCycle(simulProcessor processor, int id, simulTime * ptime)
{
    (*((simulProcInfo *) processor)->setClockCycle)(processor, id, ptime);
}


void SIMULAPI SIMUL_GetTime(simulProcessor processor, simulTime * ptime)
{
    (*((simulProcInfo *) processor)->getTime)(processor, ptime);
}


void SIMULAPI SIMUL_GetClock(simulProcessor processor, int timerid, simulTime * ptime)
{
    (*((simulProcInfo *) processor)->getClock) (processor, timerid, ptime);
}


void SIMULAPI SIMUL_Stall(simulProcessor processor, int mode, simulTime * ptime)
{
    (*((simulProcInfo *) processor)->stall) (processor, mode, ptime);
}


void SIMULAPI SIMUL_StartTimer(simulProcessor processor, void * timerid, int mode, simulTime * ptime)
{
    (*((simulProcInfo *) processor)->startTimer)(processor, timerid, mode, ptime);
}


void SIMULAPI SIMUL_StopTimer(simulProcessor processor, void * timerid)
{
    (*((simulProcInfo *) processor)->stopTimer)(processor, timerid);
}


void SIMULAPI SIMUL_StopAllTimer(simulProcessor processor)
{
    (*((simulProcInfo *) processor)->stopTimer) (processor);
}


void * SIMULAPI SIMUL_RegisterTimerCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private)
{
    return (*((simulProcInfo *) processor)->registerTimerCallback)(processor, func, private);
}



/**************************************************************************

	Buses and Memories
	
**************************************************************************/

int SIMULAPI SIMUL_ReadMemory(simulProcessor processor, int bustype, simulWord * paddress, int width, int cycletype, simulWord * pdata)
{
#if SIMUL_ARCHITECTURE_WIDTH == 8
    return (*((simulProcInfo *) processor)->readMemory64) (processor, bustype, paddress, width, cycletype, pdata);
#else
    return (*((simulProcInfo *) processor)->readMemory32) (processor, bustype, paddress, width, cycletype, pdata);
#endif
}


int SIMULAPI SIMUL_WriteMemory(simulProcessor processor, int bustype, simulWord * paddress, int width, int cycletype, simulWord * pdata)
{
#if SIMUL_ARCHITECTURE_WIDTH == 8
    return (*((simulProcInfo *) processor)->writeMemory64) (processor, bustype, paddress, width, cycletype, pdata);
#else
    return (*((simulProcInfo *) processor)->writeMemory32) (processor, bustype, paddress, width, cycletype, pdata);
#endif
}


void * SIMULAPI SIMUL_RegisterBusWriteCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private, int bustype, simulWord * paddressFrom, simulWord * paddressTo)
{
    return (*((simulProcInfo *) processor)->registerBusWriteCallback)(processor, func, private, bustype, paddressFrom, paddressTo, SIMUL_ARCHITECTURE_WIDTH);
}


void * SIMULAPI SIMUL_RegisterBusReadCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private, int bustype, simulWord * paddressFrom, simulWord * paddressTo)
{
    return (*((simulProcInfo *) processor)->registerBusReadCallback)(processor, func, private, bustype, paddressFrom, paddressTo, SIMUL_ARCHITECTURE_WIDTH);
}


void SIMULAPI SIMUL_RelocateBusCallback(simulProcessor processor, void * callbackid, int bustype, simulWord * paddressFrom, simulWord * paddressTo)
{
    (*((simulProcInfo *) processor)->relocateBusCallback)(processor, callbackid, bustype, paddressFrom, paddressTo, SIMUL_ARCHITECTURE_WIDTH);
}


void SIMULAPI SIMUL_InsertWord(simulProcessor processor, simulWord * ptarget , int wordwidth, simulWord * paddress, int width, simulWord * pdata)
{
    int             offset;

    if (width >= wordwidth) {
	*ptarget = *pdata;
    } else if (width == 8) {
	if (((simulProcInfo *) processor)->endianess) {
	    offset = wordwidth - 8 - ((*paddress << 3) & (wordwidth - 1));
	} else {
	    offset = (*paddress << 3) & (wordwidth - 1);
	}
	*ptarget = (*ptarget & ~(0xff << offset)) | ((*pdata & 0xff) << offset);
    } else if (width == 16) {
	if (((simulProcInfo *) processor)->endianess) {
	    offset = wordwidth - 16 - ((*paddress << 3) & (wordwidth - 1));
	} else {
	    offset = (*paddress << 3) & (wordwidth - 1);
	}
	*ptarget = (*ptarget & ~(0xffff << offset)) | ((*pdata & 0xffff) << offset);
#if SIMUL_ARCHITECTURE_WIDTH == 8
    } else if (width == 32) {
	if (((simulProcInfo *) processor)->endianess) {
	    offset = wordwidth - 32 - ((*paddress << 3) & (wordwidth - 1));
	} else {
	    offset = (*paddress << 3) & (wordwidth - 1);
	}
	*ptarget = (*ptarget & ~(0xffffffff << offset)) | ((*pdata & 0xffffffff) << offset);
#endif
    }
}



void SIMULAPI SIMUL_ExtractWord(simulProcessor processor, simulWord * psource, int wordwidth, simulWord * paddress, int width, simulWord * pdata)
{
    int offset;
    
    if (width >= wordwidth) {
	*pdata = *psource;
    } else if (width == 8) {
	if (((simulProcInfo *) processor)->endianess) {
	    offset = wordwidth - 8 - ((*paddress << 3) & (wordwidth - 1));
	} else {
	    offset = (*paddress << 3) & (wordwidth - 1);
	}
	*pdata = (*psource >> offset) & 0xff;
    } else if (width == 16) {
	if (((simulProcInfo *) processor)->endianess) {
	    offset = wordwidth - 16 - ((*paddress << 3) & (wordwidth - 1));
	} else {
	    offset = (*paddress << 3) & (wordwidth - 1);
	}
	*pdata = (*psource >> offset) & 0xffff;
#if SIMUL_ARCHITECTURE_WIDTH == 8
    } else if (width == 32) {
	if (((simulProcInfo *) processor)->endianess) {
	    offset = wordwidth - 32 - ((*paddress << 3) & (wordwidth - 1));
	} else {
	    offset = (*paddress << 3) & (wordwidth - 1);
	}
	*pdata = (*psource >> offset) & 0xffffffff;
#endif
    }
}


void SIMULAPI SIMUL_SaveWord(simulProcessor processor, void * ptarget, int width, simulWord * pdata)
{
    if (((simulProcInfo *) processor)->endianess) {
	switch (width>>3) {
#if SIMUL_ARCHITECTURE_WIDTH == 8
	case 8:
	    ((unsigned char *) ptarget)[0] = *pdata >> 56;
	    ((unsigned char *) ptarget)[1] = *pdata >> 48;
	    ((unsigned char *) ptarget)[2] = *pdata >> 40;
	    ((unsigned char *) ptarget)[3] = *pdata >> 32;
	    ((unsigned char *) ptarget)[4] = *pdata >> 24;
	    ((unsigned char *) ptarget)[5] = *pdata >> 16;
	    ((unsigned char *) ptarget)[6] = *pdata >> 8;
	    ((unsigned char *) ptarget)[7] = *pdata;
	    break;
#endif
	case 4:
	    ((unsigned char *) ptarget)[0] = *pdata >> 24;
	    ((unsigned char *) ptarget)[1] = *pdata >> 16;
	    ((unsigned char *) ptarget)[2] = *pdata >> 8;
	    ((unsigned char *) ptarget)[3] = *pdata;
	    break;
	case 2:
	    ((unsigned char *) ptarget)[0] = *pdata >> 8;
	    ((unsigned char *) ptarget)[1] = *pdata;
	    break;
	default:
	    ((unsigned char *) ptarget)[0] = *pdata;
	    break;
	}
    } else {
	switch (width>>3) {
#if SIMUL_ARCHITECTURE_WIDTH == 8
	case 8:
	    ((unsigned char *) ptarget)[7] = *pdata >> 56;
	    ((unsigned char *) ptarget)[6] = *pdata >> 48;
	    ((unsigned char *) ptarget)[5] = *pdata >> 40;
	    ((unsigned char *) ptarget)[4] = *pdata >> 32;
#endif
	case 4:
	    ((unsigned char *) ptarget)[3] = *pdata >> 24;
	    ((unsigned char *) ptarget)[2] = *pdata >> 16;
	case 2:
	    ((unsigned char *) ptarget)[1] = *pdata >> 8;
	default:
	    ((unsigned char *) ptarget)[0] = *pdata;
	}
    }
}


void SIMULAPI SIMUL_LoadWord(simulProcessor processor, void * psource, int width, simulWord * pdata)
{
    if (((simulProcInfo *) processor)->endianess) {
	switch (width>>3) {
#if SIMUL_ARCHITECTURE_WIDTH == 8
	case 8:
	    {
		simulWord       low, high;
		low = (((unsigned char *) psource)[7]) | (((unsigned char *) psource)[6] << 8)
		    | (((unsigned char *) psource)[5] << 16) | (((unsigned char *) psource)[4] << 24);
		high = (((unsigned char *) psource)[3]) | (((unsigned char *) psource)[2] << 8)
		    | (((unsigned char *) psource)[1] << 16) | (((unsigned char *) psource)[0] << 24);
		*pdata = low | (high << 32);
	    }
	    break;
#endif
	case 4:
	    *pdata = (((unsigned char *) psource)[3]) | (((unsigned char *) psource)[2] << 8)
		| (((unsigned char *) psource)[1] << 16) | (((unsigned char *) psource)[0] << 24);
	    break;
	case 2:
	    *pdata = (((unsigned char *) psource)[1]) | (((unsigned char *) psource)[0] << 8);
	    break;
	default:
	    *pdata = ((unsigned char *) psource)[0];
	}
    } else {
	switch (width>>3) {
#if SIMUL_ARCHITECTURE_WIDTH == 8
	case 8:
	    {
		simulWord       low, high;
		low = (((unsigned char *) psource)[0]) | (((unsigned char *) psource)[1] << 8)
		    | (((unsigned char *) psource)[2] << 16) | (((unsigned char *) psource)[3] << 24);
		high = (((unsigned char *) psource)[4]) | (((unsigned char *) psource)[5] << 8)
		    | (((unsigned char *) psource)[6] << 16) | (((unsigned char *) psource)[7] << 24);
		*pdata = low | (high << 32);
	    }
	    break;
#endif
	case 4:
	    *pdata = (((unsigned char *) psource)[0]) | (((unsigned char *) psource)[1] << 8)
		| (((unsigned char *) psource)[2] << 16) | (((unsigned char *) psource)[3] << 24);
	    break;
	case 2:
	    *pdata = (((unsigned char *) psource)[0]) | (((unsigned char *) psource)[1] << 8);
	    break;
	default:
	    *pdata = ((unsigned char *) psource)[0];
	}
    }
}



/**************************************************************************

	Ports
	
**************************************************************************/

int SIMULAPI SIMUL_GetPort(simulProcessor processor, int offset, int width, simulWord * pdata)
{
#if SIMUL_ARCHITECTURE_WIDTH == 8
    return (*((simulProcInfo *) processor)->getPort64) (processor, offset, width, pdata);
#else
    return (*((simulProcInfo *) processor)->getPort32) (processor, offset, width, pdata);
#endif
}


int SIMULAPI SIMUL_SetPort(simulProcessor processor, int offset, int width, simulWord * pdata)
{
#if SIMUL_ARCHITECTURE_WIDTH == 8
    return (*((simulProcInfo *) processor)->setPort64) (processor, offset, width, pdata);
#else
    return (*((simulProcInfo *) processor)->setPort32) (processor, offset, width, pdata);
#endif
}


void * SIMULAPI SIMUL_RegisterPortChangeCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private, int offset, int width)
{
    return (*((simulProcInfo *) processor)->registerPortChangeCallback)(processor, func, private, offset, width, SIMUL_ARCHITECTURE_WIDTH);
}



/**************************************************************************

	Shared Resources
	
**************************************************************************/

int SIMULAPI SIMUL_CreateSharedResource(simulProcessor processor, int size)
{
    return (*((simulProcInfo *) processor)->createSharedResource) (processor, size);
}


int SIMULAPI SIMUL_ReadSharedResource(simulProcessor processor, int offset, int len, void * pdata)
{
    return (*((simulProcInfo *) processor)->readSharedResource) (processor, offset, len, pdata);
}


int SIMULAPI SIMUL_WriteSharedResource(simulProcessor processor, int offset, int len, void * pdata)
{
    return (*((simulProcInfo *) processor)->writeSharedResource) (processor, offset, len, pdata);
}



/**************************************************************************

	File I/O
	
**************************************************************************/

void * SIMULAPI SIMUL_OpenFile(simulProcessor processor, const char * filename, int mode)
{
    return (*((simulProcInfo *) processor)->openFile) (processor, filename, mode);
}


void SIMULAPI SIMUL_CloseFile(simulProcessor processor, void * file)
{
    (*((simulProcInfo *) processor)->closeFile) (processor, file);
}


int SIMULAPI SIMUL_ReadFile(simulProcessor processor, void * file, void * pdata, int length)
{
    return (*((simulProcInfo *) processor)->readFile) (processor, file, pdata, length);
}

int SIMULAPI SIMUL_WriteFile(simulProcessor processor, void * file, void * pdata, int length)
{
    return (*((simulProcInfo *) processor)->writeFile) (processor, file, pdata, length);
}


int SIMULAPI SIMUL_ReadlineFile(simulProcessor processor, void * file, void * pdata, int length)
{
    return (*((simulProcInfo *) processor)->readlineFile) (processor, file, pdata, length);
}

int SIMULAPI SIMUL_WritelineFile(simulProcessor processor, void * file, void * pdata)
{
    return (*((simulProcInfo *) processor)->writelineFile) (processor, file, pdata);
}


long SIMULAPI SIMUL_SeekFile(simulProcessor processor, void * file, long pos, int mode)
{
    return (*((simulProcInfo *) processor)->seekFile) (processor, file, pos, mode);
}



/**************************************************************************

	Terminal I/O
	
**************************************************************************/

int SIMULAPI SIMUL_StateTerminal(simulProcessor processor, int id)
{
    return (*((simulProcInfo *) processor)->stateTerminal) (processor, id);
}

int SIMULAPI SIMUL_ReadTerminal(simulProcessor processor, int id)
{
    return (*((simulProcInfo *) processor)->readTerminal) (processor, id);
}

int SIMULAPI SIMUL_WriteTerminal(simulProcessor processor, int id, int ch)
{
    return (*((simulProcInfo *) processor)->writeTerminal) (processor, id, ch);
}

void * SIMUL_RegisterTerminalCallback(simulProcessor processor, simulCallbackFunctionPtr func, simulPtr private, int id)
{
    return (*((simulProcInfo *) processor)->registerTerminalCallback) (processor, func, private, id, SIMUL_ARCHITECTURE_WIDTH);
}


