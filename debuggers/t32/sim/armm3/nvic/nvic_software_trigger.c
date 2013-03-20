#include "nvic.h"

/*************** Software Trigger Interrupt Register				***************/

int STIR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int arrayidx;
	simulWord32 reg = 0;
	int wakeUpEvent = 0;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus,&reg);

	arrayidx = (reg & 0x1FF) + 16;

	if (arrayidx < 256)
	{
		ChangePending(arrayidx,1);
		Interrupt(processor,IntCtrl,wakeUpEvent);
	}

	return SIMUL_MEMORY_OK;
}
