#include "nvic.h"

/*************** SysTick Control and Status Register			***************/

int STCSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
  IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;

	reg = IntCtrl->regs.stcsr & (REG_STCSR_COUNTFLAG | REG_STCSR_TICKINT | REG_STCSR_ENABLE | REG_STCSR_CLKSOURCE);

	if (cbs->x.bus.cycletype!=SIMUL_MEMORY_HIDDEN)
	{
		IntCtrl->regs.stcsr &= ~REG_STCSR_COUNTFLAG;
	}

	return BusWrite(&cbs->x.bus, &reg);
}

int STCSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;

	reg = IntCtrl->regs.stcsr;
	BusRead(&cbs->x.bus,&reg);

	if (reg ^ IntCtrl->regs.stcsr)
	{
		if ((IntCtrl->regs.stcsr & REG_STCSR_ENABLE) ^ (reg & REG_STCSR_ENABLE))
		{
			if (reg & REG_STCSR_ENABLE)
			{
				/* enable timer */
				simulTime time = 1;
				SIMUL_StartTimer(processor, IntCtrl->timer, SIMUL_TIMER_REPEAT | SIMUL_TIMER_CLOCKS, &time);
				IntCtrl->timerrun = 1;

				IntCtrl->regs.stcvr = IntCtrl->regs.strvr;
			}
			else
			{
				/* disable timer */
				SIMUL_StopTimer(processor, IntCtrl->timer);
				IntCtrl->timerrun = 0;
			}
		}

		IntCtrl->regs.stcsr = (IntCtrl->regs.stcsr & ~(REG_STCSR_TICKINT | REG_STCSR_ENABLE | REG_STCSR_COUNTFLAG)) | (reg & (REG_STCSR_TICKINT | REG_STCSR_ENABLE | REG_STCSR_COUNTFLAG)) | REG_STCSR_CLKSOURCE;
	}

	return SIMUL_MEMORY_OK;
}

/*************** SysTick Reload Value Register					***************/

int STRVR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;

	reg = IntCtrl->regs.strvr & 0xFFFFFF;

	return BusWrite(&cbs->x.bus, &reg);
}

int STRVR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus,&reg);

	IntCtrl->regs.strvr = reg & 0xFFFFFF;

	return SIMUL_MEMORY_OK;
}

/*************** SysTick Current Value Register					***************/

int STCVR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;

	reg = IntCtrl->regs.stcvr & 0xFFFFFF;

	return BusWrite(&cbs->x.bus, &reg);
}

int STCVR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	cbs->x.bus.clocks = 1;

	IntCtrl->regs.stcvr = 0;
	IntCtrl->regs.stcvr &= ~REG_STCSR_COUNTFLAG;

	return SIMUL_MEMORY_OK;
}

/*************** SysTick Calibration Value Register				***************/

int STCLVR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;

	reg = IntCtrl->regs.stclvr;

	return BusWrite(&cbs->x.bus, &reg);
}
