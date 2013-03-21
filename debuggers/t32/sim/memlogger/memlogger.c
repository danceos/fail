
#include "simul.h"
#include <string.h>
/**************************************************************************

	Local port structure

**************************************************************************/

typedef struct
{
    simulWord infoBase;
    int bustype;
    int data;
}
MemLog_t;

static int SIMULAPI readCB(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
   simulWord address = cbs->x.bus.address;
   MemLog_t * memlog = (MemLog_t*)private;
   cbs->x.bus.data = memlog->data;
   simulWord width = cbs->x.bus.width;
   SIMUL_Printf(processor, "MEM Read *0x%x - 0x%x\n", address, memlog->data);

  simulWord data = memlog->data;
   simulWord writeAccess = 0;
   simulWord iadr = memlog->infoBase;
   SIMUL_WriteMemory(processor, memlog->bustype, &iadr, 4, SIMUL_MEMORY_HIDDEN, &address);
   iadr += 4;
   SIMUL_WriteMemory(processor, memlog->bustype, &iadr, 4, SIMUL_MEMORY_HIDDEN, &data);
   iadr += 4;
   SIMUL_WriteMemory(processor, memlog->bustype, &iadr, 4, SIMUL_MEMORY_HIDDEN, &width);
   iadr += 4;
   SIMUL_WriteMemory(processor, memlog->bustype, &iadr, 4, SIMUL_MEMORY_HIDDEN, &writeAccess);

   return SIMUL_MEMORY_OK;
}


static int SIMULAPI writeCB(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
   simulWord data = cbs->x.bus.data;
   simulWord address = cbs->x.bus.address;
   MemLog_t * memlog = (MemLog_t*)private;
   memlog->data = cbs->x.bus.data;
   simulWord width = cbs->x.bus.width;
   SIMUL_Printf(processor, "MEM Write *0x%x - 0x%x\n", address, data);

   simulWord writeAccess = 1;
   simulWord iadr = memlog->infoBase;
   SIMUL_WriteMemory(processor, memlog->bustype, &iadr, 4, SIMUL_MEMORY_HIDDEN, &address);
   iadr += 4;
   SIMUL_WriteMemory(processor, memlog->bustype, &iadr, 4, SIMUL_MEMORY_HIDDEN, &data);
   iadr += 4;
   SIMUL_WriteMemory(processor, memlog->bustype, &iadr, 4, SIMUL_MEMORY_HIDDEN, &width);
   iadr += 4;
   SIMUL_WriteMemory(processor, memlog->bustype, &iadr, 4, SIMUL_MEMORY_HIDDEN, &writeAccess);


   return SIMUL_MEMORY_OK;
}



static int SIMULAPI PortReset(simulProcessor processor, simulCallbackStruct * cbs, simulPtr private)
{
    return SIMUL_RESET_OK;
}


/**************************************************************************

	Entry point of the Model

**************************************************************************/

int SIMULAPI SIMUL_Init(simulProcessor processor, simulCallbackStruct * cbs)
{
    MemLog_t       *pmemlog;

    strcpy(cbs->x.init.modelname, __DATE__ " MemLogger");

    pmemlog = (MemLog_t *) SIMUL_Alloc(processor, sizeof(MemLog_t));

//   if (cbs->x.init.argc != 3) {
//	SIMUL_Warning(processor, "parameters: <address> <portnumber>");
//	return SIMUL_INIT_FAIL;
//    }
    pmemlog->bustype = cbs->x.init.argpbustype[1];
    pmemlog->infoBase = 0x60000000; // placed at "external RAM"

    simulWord from, to;
    from = 0x20002074;
    to   = 0x20002077;
    SIMUL_RegisterResetCallback(processor, PortReset, (simulPtr) pmemlog);

    SIMUL_RegisterBusWriteCallback(processor, writeCB, (simulPtr) pmemlog, pmemlog->bustype, &from, &to);
    SIMUL_RegisterBusReadCallback(processor, readCB, (simulPtr) pmemlog, pmemlog->bustype, &from, &to);
    return SIMUL_INIT_OK;
}
