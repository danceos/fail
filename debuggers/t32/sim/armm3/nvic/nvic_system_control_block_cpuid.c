#include "nvic.h"

/*************** CPUID Base Register              				***************/

int CPUIDR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	cbs->x.bus.clocks = 1;
	return BusWrite(&cbs->x.bus, &IntCtrl->regs.cpuidr);
}

/*************** Features    				***************/

int Feature_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0x0;

	switch ((cbs->x.bus.address-(IntCtrl->baseaddress+PFR0_OFFSET))&0xFF)
	{
		case 0x00: 	reg = IntCtrl->regs.pfr0; break;
		case 0x04:	reg = IntCtrl->regs.pfr1; break;
		case 0x08:	reg = IntCtrl->regs.dfr0; break;
		case 0x0C:	reg = IntCtrl->regs.afr0; break;
		case 0x10:	reg = IntCtrl->regs.mmfr0; break;
		case 0x14:	reg = IntCtrl->regs.mmfr1; break;
		case 0x18:	reg = IntCtrl->regs.mmfr2; break;
		case 0x1C:	reg = IntCtrl->regs.mmfr3; break;
		case 0x20:	reg = IntCtrl->regs.isar0; break;
		case 0x24:	reg = IntCtrl->regs.isar1; break;
		case 0x28:	reg = IntCtrl->regs.isar2; break;
		case 0x2C:	reg = IntCtrl->regs.isar3; break;
		case 0x30:	reg = IntCtrl->regs.isar4; break;
	}
	cbs->x.bus.clocks = 1;
	return BusWrite(&cbs->x.bus, &reg);
}
