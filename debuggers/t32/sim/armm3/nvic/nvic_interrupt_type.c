#include "nvic.h"

/*************** Interrupt Control Type Register				***************/

int ICTR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;

	reg = 0;

	if (IntCtrl->ExternalIrqPortsCount >= 0 && IntCtrl->ExternalIrqPortsCount <= 32)
	{
		reg = 0;
	}
	else if (IntCtrl->ExternalIrqPortsCount >= 33 && IntCtrl->ExternalIrqPortsCount <= 64)
	{
		reg = 1;
	}
	else if (IntCtrl->ExternalIrqPortsCount >= 65 && IntCtrl->ExternalIrqPortsCount <= 96)
	{
		reg = 2;
	}
	else if (IntCtrl->ExternalIrqPortsCount >= 97 && IntCtrl->ExternalIrqPortsCount <= 128)
	{
		reg = 3;
	}
	else if (IntCtrl->ExternalIrqPortsCount >= 129 && IntCtrl->ExternalIrqPortsCount <= 160)
	{
		reg = 4;
	}
	else if (IntCtrl->ExternalIrqPortsCount >= 161 && IntCtrl->ExternalIrqPortsCount <= 192)
	{
		reg = 5;
	}
	else if (IntCtrl->ExternalIrqPortsCount >= 193 && IntCtrl->ExternalIrqPortsCount <= 224)
	{
		reg = 6;
	}
	else if (IntCtrl->ExternalIrqPortsCount >= 225 && IntCtrl->ExternalIrqPortsCount <= 256)
	{
		reg = 7;
	}

	return BusWrite(&cbs->x.bus, &reg);
}
