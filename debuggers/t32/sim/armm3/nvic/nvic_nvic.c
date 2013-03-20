#include "nvic.h"

/*************** Irq x Set Enable Register						***************/

int IRQSETENR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQSETENR_OFFSET) >> 2;
	int arrayidx = (idx << 5) + 16;
	simulWord32 reg = 0;
	int i;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus,&reg);

	for (i=0;i<32;i++)
	{
		if (reg & (1<<i)) ChangeEnable(arrayidx+i,1);
	}

	Interrupt(processor,IntCtrl,0);

	return SIMUL_MEMORY_OK;
}

/*************** Irq x Clear Enable Register     				***************/

int IRQCLRENR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQCLRENR_OFFSET) >> 2;
	int arrayidx = (idx << 5) + 16;
	simulWord32 reg = 0;
	int i;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus,&reg);

	for (i=0;i<32;i++)
	{
		if (reg & (1<<i)) ChangeEnable(arrayidx+i,0);
	}

	Interrupt(processor,IntCtrl,0);

	return SIMUL_MEMORY_OK;
}

/*************** Irq x Set Pending Register      				***************/

int IRQSETPER_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQSETPER_OFFSET) >> 2;
	int arrayidx = (idx << 5) + 16;
	simulWord32 reg = 0;
	int wakeUpEvent = 0;
	int i;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus,&reg);

	for (i=0;i<32;i++)
	{
		if (reg & (1<<i)) ChangePending(arrayidx+i,1);
	}

	Interrupt(processor,IntCtrl,wakeUpEvent);

	return SIMUL_MEMORY_OK;
}

/*************** Irq x Clear Pending Register    				***************/

int IRQCLRPER_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQCLRPER_OFFSET) >> 2;
	int arrayidx = (idx << 5) + 16;
	simulWord32 reg = 0;
	int wakeUpEvent = 0;
	int i;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus,&reg);

	for (i=0;i<32;i++)
	{
		if (reg & (1<<i)) ChangePending(arrayidx+i,0);
	}

	Interrupt(processor,IntCtrl,wakeUpEvent);

	return SIMUL_MEMORY_OK;
}

/*************** Irq x Priority Register          				***************/

int IRQPR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQPR_OFFSET) >> 2;
	int arrayidx = (idx << 2) + 16;
	simulWord32 reg = 0;
	int i;

	cbs->x.bus.clocks = 1;

	for (i=0;i<4;i++) reg |= IntCtrl->IrqPriority[arrayidx+i] << (i << 3);

	BusRead(&cbs->x.bus,&reg);

	for (i=0;i<4;i++) IntCtrl->IrqPriority[arrayidx+i] = (unsigned char)(reg >> (i << 3));

	return SIMUL_MEMORY_OK;
}

/*************** Irq x Set Enable Register						***************/

int IRQSETENR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQSETENR_OFFSET) >> 2;
	int arrayidx = (idx << 5) + 16;
	simulWord32 reg = 0;
	int i;

	cbs->x.bus.clocks = 1;

	if (arrayidx<(7 * 32))
	{
		for (i=0;i<32;i++) reg |= (IntCtrl->IrqEnable[arrayidx+i])?(1<<i):(0);
	}
	else
	{
		for (i=0;i<16;i++) reg |= (IntCtrl->IrqEnable[arrayidx+i])?(1<<i):(0);
	}

	BusWrite(&cbs->x.bus,&reg);
	return SIMUL_MEMORY_OK;
}

/*************** Irq x Clear Enable Register     				***************/

int IRQCLRENR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQCLRENR_OFFSET) >> 2;
	int arrayidx = (idx << 5) + 16;
	simulWord32 reg = 0;
	int i;

	cbs->x.bus.clocks = 1;

	if (arrayidx<(7 * 32))
	{
		for (i=0;i<32;i++) reg |= (IntCtrl->IrqEnable[arrayidx+i])?(1<<i):(0);
	}
	else
	{
		for (i=0;i<16;i++) reg |= (IntCtrl->IrqEnable[arrayidx+i])?(1<<i):(0);
	}

	BusWrite(&cbs->x.bus,&reg);
	return SIMUL_MEMORY_OK;
}

/*************** Irq x Set Pending Register      				***************/

int IRQSETPER_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQSETPER_OFFSET) >> 2;
	int arrayidx = (idx << 5) + 16;
	simulWord32 reg = 0;
	int i;

	cbs->x.bus.clocks = 1;

	if (arrayidx<(7 * 32))
	{
		for (i=0;i<32;i++) reg |= (IntCtrl->IrqPending[arrayidx+i])?(1<<i):(0);
	}
	else
	{
		for (i=0;i<16;i++) reg |= (IntCtrl->IrqPending[arrayidx+i])?(1<<i):(0);
	}

	BusWrite(&cbs->x.bus,&reg);
	return SIMUL_MEMORY_OK;
}

/*************** Irq x Clear Pending Register    				***************/

int IRQCLRPER_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQCLRPER_OFFSET) >> 2;
	int arrayidx = (idx << 5) + 16;
	simulWord32 reg = 0;
	int i;

	cbs->x.bus.clocks = 1;

	if (arrayidx<(7 * 32))
	{
		for (i=0;i<32;i++) reg |= (IntCtrl->IrqPending[arrayidx+i])?(1<<i):(0);
	}
	else
	{
		for (i=0;i<16;i++) reg |= (IntCtrl->IrqPending[arrayidx+i])?(1<<i):(0);
	}

	BusWrite(&cbs->x.bus,&reg);
	return SIMUL_MEMORY_OK;
}

/*************** Irq x Active Bit Register       				***************/

int IRQABR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQABR_OFFSET) >> 2;
	int arrayidx = (idx << 5) + 16;
	simulWord32 reg = 0;
	int i;

	cbs->x.bus.clocks = 1;

	if (arrayidx<(7 * 32))
	{
		for (i=0;i<32;i++) reg |= (IntCtrl->IrqActive[arrayidx+i])?(1<<i):(0);
	}
	else
	{
		for (i=0;i<16;i++) reg |= (IntCtrl->IrqActive[arrayidx+i])?(1<<i):(0);
	}

	BusWrite(&cbs->x.bus,&reg);
	return SIMUL_MEMORY_OK;
}

/*************** Irq x Priority Register          				***************/

int IRQPR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = ((cbs->x.bus.address & ~0x3) - IntCtrl->baseaddress - IRQPR_OFFSET) >> 2;
	int arrayidx = (idx << 2) + 16;
	simulWord32 reg = 0;
	int i;

	cbs->x.bus.clocks = 1;

	for (i=0;i<4;i++) reg |= ((simulWord32)((unsigned char)IntCtrl->IrqPriority[arrayidx+i])) << (i << 3);

	BusWrite(&cbs->x.bus,&reg);
	return SIMUL_MEMORY_OK;
}
