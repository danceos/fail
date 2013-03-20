#include "nvic.h"

/*************** IDs 			***************/

int ID_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0x0;

	switch ((cbs->x.bus.address-(IntCtrl->baseaddress+PID4_OFFSET))&0xFF)
	{
		case 0x00: 	reg = IntCtrl->regs.pid4; break;
		case 0x04:	reg = IntCtrl->regs.pid5; break;
		case 0x08:	reg = IntCtrl->regs.pid6; break;
		case 0x0C:	reg = IntCtrl->regs.pid7; break;
		case 0x10:	reg = IntCtrl->regs.pid0; break;
		case 0x14:	reg = IntCtrl->regs.pid1; break;
		case 0x18:	reg = IntCtrl->regs.pid2; break;
		case 0x1C:	reg = IntCtrl->regs.pid3; break;
		case 0x20:	reg = IntCtrl->regs.cid0; break;
		case 0x24:	reg = IntCtrl->regs.cid1; break;
		case 0x28:	reg = IntCtrl->regs.cid2; break;
		case 0x2C:	reg = IntCtrl->regs.cid3; break;
	}
	cbs->x.bus.clocks = 1;
	return BusWrite(&cbs->x.bus, &reg);
}


/*************** GPIO 			***************/

#ifdef _DEBUG
int GPIO_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0x0;

	switch ((cbs->x.bus.address-(IntCtrl->baseaddress+GPIO_OFFSET))&0xF)
	{
		case 0x00: 	
			reg = IntCtrl->regs.gpio; 
			break;
		case 0x04:	
			reg = 0; 
			break;
		case 0x08:	
			reg = 0; 
			break;
	}
	cbs->x.bus.clocks = 1;
	return BusWrite(&cbs->x.bus, &reg);
}

int GPIO_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0x0;

	BusRead(&cbs->x.bus, &reg);

	switch ((cbs->x.bus.address-(IntCtrl->baseaddress+GPIO_OFFSET))&0xF)
	{
		case 0x04:	
			IntCtrl->regs.gpio |= reg;
			break;
		case 0x08:	
			IntCtrl->regs.gpio &= ~reg;
			break;
	}
	cbs->x.bus.clocks = 1;

	return SIMUL_MEMORY_OK;
}

#endif
