/***
* Cortex-M3 NVIC Simulator (C) Lauterbach 2010
* 08.03.2010  Sylwester Garncarek
***/

#include "nvic.h"
#include <string.h>

int setbit(simulWord32 v)
{
	int i;
	for (i=0;i<32;i++)
	{
		if (v & (1 << i)) return i;
	}

	return 0;
}

unsigned long NVIC_reg_offsets[] =
{
CMD_OFFSET,ICTR_OFFSET,STCSR_OFFSET,STRVR_OFFSET,STCVR_OFFSET,STCLVR_OFFSET,IRQSETENR_OFFSET,IRQCLRENR_OFFSET,IRQSETPER_OFFSET,IRQCLRPER_OFFSET,
IRQABR_OFFSET,IRQPR_OFFSET,CPUIDR_OFFSET,ICSR_OFFSET,VTOR_OFFSET,AIRCR_OFFSET,SCR_OFFSET,CCR_OFFSET,SHPR_OFFSET,SHCSR_OFFSET,CFSR_OFFSET,
HFSR_OFFSET,DFSR_OFFSET,MMAR_OFFSET,BFAR_OFFSET,AFSR_OFFSET,PFR0_OFFSET,PFR1_OFFSET,DFR0_OFFSET,AFR0_OFFSET,MMFR0_OFFSET,MMFR1_OFFSET,MMFR2_OFFSET,
MMFR3_OFFSET,ISAR0_OFFSET,ISAR1_OFFSET,ISAR2_OFFSET,ISAR3_OFFSET,ISAR4_OFFSET,STIR_OFFSET,PID4_OFFSET,PID5_OFFSET,PID6_OFFSET,PID7_OFFSET,PID0_OFFSET,
PID1_OFFSET,PID2_OFFSET,PID3_OFFSET,CID0_OFFSET,CID1_OFFSET,CID2_OFFSET,CID3_OFFSET,DHCSR_OFFSET,
#ifdef _DEBUG
GPIO_OFFSET,
#endif
};

unsigned long NVIC_reg_sizes[] =
{
CMD_SIZE,ICTR_SIZE,STCSR_SIZE,STRVR_SIZE,STCVR_SIZE,STCLVR_SIZE,IRQSETENR_SIZE,IRQCLRENR_SIZE,IRQSETPER_SIZE,IRQCLRPER_SIZE,IRQABR_SIZE,IRQPR_SIZE,CPUIDR_SIZE,
ICSR_SIZE,VTOR_SIZE,AIRCR_SIZE,SCR_SIZE,CCR_SIZE,SHPR_SIZE,SHCSR_SIZE,CFSR_SIZE,HFSR_SIZE,DFSR_SIZE,MMAR_SIZE,BFAR_SIZE,AFSR_SIZE,PFR0_SIZE,PFR1_SIZE,
DFR0_SIZE,AFR0_SIZE,MMFR0_SIZE,MMFR1_SIZE,MMFR2_SIZE,MMFR3_SIZE,ISAR0_SIZE,ISAR1_SIZE,ISAR2_SIZE,ISAR3_SIZE,ISAR4_SIZE,STIR_SIZE,PID4_SIZE,PID5_SIZE,
PID6_SIZE,PID7_SIZE,PID0_SIZE,PID1_SIZE,PID2_SIZE,PID3_SIZE,CID0_SIZE,CID1_SIZE,CID2_SIZE,CID3_SIZE,DHCSR_SIZE,
#ifdef _DEBUG
GPIO_SIZE,
#endif
};

simulCallbackFunctionPtr NVIC_reg_read_callbacks[] =
{
CMD_Read,ICTR_Read,STCSR_Read,STRVR_Read,STCVR_Read,STCLVR_Read,IRQSETENR_Read,IRQCLRENR_Read,IRQSETPER_Read,IRQCLRPER_Read,IRQABR_Read,IRQPR_Read,CPUIDR_Read,
ICSR_Read,VTOR_Read,AIRCR_Read,SCR_Read,CCR_Read,SHPR_Read,SHCSR_Read,CFSR_Read,HFSR_Read,DFSR_Read,MMAR_Read,BFAR_Read,AFSR_Read,/*PFR0_Read*/Feature_Read,/*PFR1_Read*/Feature_Read,
/*DFR0_Read*/Feature_Read,/*AFR0_Read*/Feature_Read,/*MMFR0_Read*/Feature_Read,/*MMFR1_Read*/Feature_Read,/*MMFR2_Read*/Feature_Read,/*MMFR3_Read*/Feature_Read,/*ISAR0_Read*/Feature_Read,/*ISAR1_Read*/Feature_Read,/*ISAR2_Read*/Feature_Read,/*ISAR3_Read*/Feature_Read,/*ISAR4_Read*/Feature_Read,/*STIR_Read*/NULL,/*PID4_Read*/ID_Read,/*PID5_Read*/ID_Read,
/*PID6_Read*/ID_Read,/*PID7_Read*/ID_Read,/*PID0_Read*/ID_Read,/*PID1_Read*/ID_Read,/*PID2_Read*/ID_Read,/*PID3_Read*/ID_Read,/*CID0_Read*/ID_Read,/*CID1_Read*/ID_Read,/*CID2_Read*/ID_Read,/*CID3_Read*/ID_Read,DHCSR_Read,
#ifdef _DEBUG
GPIO_Read,
#endif
};

simulCallbackFunctionPtr NVIC_reg_write_callbacks[] =
{
CMD_Write,/*ICTR_Write*/NULL,STCSR_Write,STRVR_Write,STCVR_Write,/*STCLVR_Write*/NULL,IRQSETENR_Write,IRQCLRENR_Write,IRQSETPER_Write,IRQCLRPER_Write,/*IRQABR_Write*/NULL,IRQPR_Write,/*CPUIDR_Write*/NULL,
ICSR_Write,VTOR_Write,AIRCR_Write,SCR_Write,CCR_Write,SHPR_Write,SHCSR_Write,CFSR_Write,HFSR_Write,DFSR_Write,MMAR_Write,BFAR_Write,AFSR_Write,/*PFR0_Write*/NULL,
/*PFR1_Write*/NULL,/*DFR0_Write*/NULL,/*AFR0_Write*/NULL,/*MMFR0_Write*/NULL,/*MMFR1_Write*/NULL,/*MMFR2_Write*/NULL,/*MMFR3_Write*/NULL,/*ISAR0_Write*/NULL,/*ISAR1_Write*/NULL,/*ISAR2_Write*/NULL,/*ISAR3_Write*/NULL,/*ISAR4_Write*/NULL,STIR_Write,
/*PID4_Write*/NULL,/*PID5_Write*/NULL,/*PID6_Write*/NULL,/*PID7_Write*/NULL,/*PID0_Write*/NULL,/*PID1_Write*/NULL,/*PID2_Write*/NULL,/*PID3_Write*/NULL,/*CID0_Write*/NULL,/*CID1_Write*/NULL,/*CID2_Write*/NULL,/*CID3_Write*/NULL,DHCSR_Write,
#ifdef _DEBUG
GPIO_Write,
#endif
};

int NVIC_SysTick(simulProcessor processor, simulCallbackStruct *cbs, simulPtr _private)
{
	IntController *IntCtrl = (IntController*) _private;

	if ((IntCtrl->regs.stcvr & 0xFFFFFF) > 1)
	{
		IntCtrl->regs.stcvr	= (IntCtrl->regs.stcvr & 0xFFFFFF) - 1;
	}
	else if ((IntCtrl->regs.stcvr & 0xFFFFFF) == 1)
	{
		IntCtrl->regs.stcsr |= REG_STCSR_COUNTFLAG;

		if (IntCtrl->regs.stcsr & REG_STCSR_TICKINT)
		{
			int wakeUpEvent = 0;
			ChangePending(IRQNUM_SYSTICK,1);
			Interrupt(processor,IntCtrl,wakeUpEvent);
		}

		IntCtrl->regs.stcvr = IntCtrl->regs.strvr;
	}
	else
	{
		//nothing happens here
	}

	return SIMUL_TIMER_OK;
}

void NVIC_Init(simulProcessor processor,IntController *IntCtrl)
{
    simulWord from, to;
	int i;

	for (i=0;i<sizeof(NVIC_reg_offsets)/sizeof(unsigned long);i++)
	{
		from = IntCtrl->baseaddress + NVIC_reg_offsets[i];
		to   = IntCtrl->baseaddress + NVIC_reg_offsets[i]+ NVIC_reg_sizes[i] - 1;
		if (NVIC_reg_read_callbacks[i]) SIMUL_RegisterBusReadCallback( processor, (simulCallbackFunctionPtr)NVIC_reg_read_callbacks[i], (simulPtr) IntCtrl, IntCtrl->bustype, &from, &to );
		if (NVIC_reg_write_callbacks[i]) SIMUL_RegisterBusWriteCallback( processor, (simulCallbackFunctionPtr)NVIC_reg_write_callbacks[i], (simulPtr) IntCtrl, IntCtrl->bustype, &from, &to );
	}
}

static int SIMULAPI NVIC_Reset(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
	IntController *IntCtrl = (IntController*) _private;

	memset(&IntCtrl->regs, 0x00, sizeof(IntCtrl->regs));

	IntCtrl->regs.ictr = 0;
	IntCtrl->regs.stclvr = 0xC0000000;
	IntCtrl->regs.stcsr = REG_STCSR_CLKSOURCE;
	IntCtrl->regs.cpuidr = 0x411FC231;
	IntCtrl->regs.pfr0 = 0x30;
	IntCtrl->regs.pfr1 = 0x200;
	IntCtrl->regs.dfr0 = 0x100000;
	IntCtrl->regs.mmfr0 = 0x30;
	IntCtrl->regs.isar0 = 0x01141110;
	IntCtrl->regs.isar1 = 0x02111000;
	IntCtrl->regs.isar2 = 0x21112231;
	IntCtrl->regs.isar3 = 0x01111110;
	IntCtrl->regs.isar4 = 0x01310102;
	IntCtrl->regs.pid4 = 0x04;
	IntCtrl->regs.pid1 = 0xB0;
	IntCtrl->regs.pid2 = 0x1B;
	IntCtrl->regs.cid0 = 0x0D;
	IntCtrl->regs.cid1 = 0xE0;
	IntCtrl->regs.cid2 = 0x05;
	IntCtrl->regs.cid3 = 0xB1;

	IntCtrl->regs.dhcsr = 0x0;

	IntCtrl->regs.ccr = 0x200;

	memset(&IntCtrl->IrqActive, 0x00, sizeof(IntCtrl->IrqActive));
	memset(&IntCtrl->IrqEnable, 0x00, sizeof(IntCtrl->IrqEnable));
	memset(&IntCtrl->IrqPending, 0x00, sizeof(IntCtrl->IrqPending));
	memset(&IntCtrl->IrqPriority, 0x00, sizeof(IntCtrl->IrqPriority));

	IntCtrl->IrqPriority[IRQNUM_RESET] = -3;
	IntCtrl->IrqPriority[IRQNUM_NMI] = -2;
	IntCtrl->IrqPriority[IRQNUM_HARDFAULT] = -1;

	IntCtrl->IrqEnable[IRQNUM_RESET]		= 1;
	IntCtrl->IrqEnable[IRQNUM_NMI]			= 1;
	IntCtrl->IrqEnable[IRQNUM_HARDFAULT]	= 1;
	IntCtrl->IrqEnable[IRQNUM_MEMFAULT]		= 0;
	IntCtrl->IrqEnable[IRQNUM_BUSFAULT]		= 0;
	IntCtrl->IrqEnable[IRQNUM_USAGEFAULT]	= 0;
	IntCtrl->IrqEnable[IRQNUM_SVCALL]		= 1;
	IntCtrl->IrqEnable[IRQNUM_DEBUG]		= 1;
	IntCtrl->IrqEnable[IRQNUM_PENDSV]		= 1;
	IntCtrl->IrqEnable[IRQNUM_SYSTICK]		= 1;

	IntCtrl->CurrentIrqNum = 0;

	if (IntCtrl->timerrun)
	{
		SIMUL_StopTimer(processor, IntCtrl->timer);
		IntCtrl->timerrun = 0;
	}

	return SIMUL_RESET_OK;
}

static int NVIC_PortChangeInternal(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 port_set, value = 0;
	int wakeUpEvent = 0;

	port_set = cbs->x.port.newdata & (~cbs->x.port.olddata);

	if (port_set & 0x0001)
	{/* RXEV */
		SIMUL_SetPort(processor, 0, 1, &value);
		WakeUpFromWFE(processor,IntCtrl);
		wakeUpEvent = 1;
	}

	if (port_set & 0x0004)
	{/* IRQNUM_NMI			2 */
		SIMUL_SetPort(processor, IRQNUM_NMI, 1, &value);
		IntCtrl->IrqPending[IRQNUM_NMI]=1;
	}

	if (port_set & 0x0008)
	{/* IRQNUM_HARDFAULT	3 */
		SIMUL_SetPort(processor, IRQNUM_HARDFAULT, 1, &value);
		IntCtrl->IrqPending[IRQNUM_HARDFAULT]=1;
	}

	if (port_set & 0x0010)
	{/* IRQNUM_MEMFAULT		4 */
		SIMUL_SetPort(processor, IRQNUM_MEMFAULT, 1, &value);
		IntCtrl->IrqPending[IRQNUM_MEMFAULT]=1;
	}

	if (port_set & 0x0020)
	{/* IRQNUM_BUSFAULT		5 */
		SIMUL_SetPort(processor, IRQNUM_BUSFAULT, 1, &value);
		IntCtrl->IrqPending[IRQNUM_BUSFAULT]=1;
	}

	if (port_set & 0x0040)
	{/* IRQNUM_USAGEFAULT	6 */
		SIMUL_SetPort(processor, IRQNUM_USAGEFAULT, 1, &value);
		IntCtrl->IrqPending[IRQNUM_USAGEFAULT]=1;
	}

	if (port_set & 0x0800)
	{/* IRQNUM_SVCALL		11 */
		SIMUL_SetPort(processor, IRQNUM_SVCALL, 1, &value);
		IntCtrl->IrqPending[IRQNUM_SVCALL]=1;
	}

	if (port_set & 0x1000)
	{/* IRQNUM_DEBUG		12 */
		SIMUL_SetPort(processor, IRQNUM_DEBUG, 1, &value);
		IntCtrl->IrqPending[IRQNUM_DEBUG]=1;
	}

	if (port_set & 0x4000)
	{/* IRQNUM_PENDSV		14 */
		SIMUL_SetPort(processor, IRQNUM_PENDSV, 1, &value);
		IntCtrl->IrqPending[IRQNUM_PENDSV]=1;
	}

	if (port_set & 0x8000)
	{/* IRQNUM_SYSTICK		15 */
		SIMUL_SetPort(processor, IRQNUM_SYSTICK, 1, &value);
		IntCtrl->IrqPending[IRQNUM_SYSTICK]=1;
	}

	Interrupt(processor,IntCtrl,wakeUpEvent);

	return SIMUL_PORT_OK;
}

static int NVIC_PortChangeFault(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 port_set, value = 0;

	port_set = cbs->x.port.newdata & (~cbs->x.port.olddata);

	if (port_set)
	{
		int wakeUpEvent = 0;

		if (port_set & REG_CFSR_MMFSR_MASK)
		{
			IntCtrl->regs.cfsr |= (port_set & REG_CFSR_MMFSR_MASK);
			ChangePending(IRQNUM_MEMFAULT,1);
			while (port_set & REG_CFSR_MMFSR_MASK)
			{
				SIMUL_SetPort(processor, 16 + setbit(port_set & REG_CFSR_MMFSR_MASK), 1, &value);
				port_set &= ~(1 << setbit(port_set));
			}
		}

		if (port_set & REG_CFSR_BFSR_MASK)
		{
			IntCtrl->regs.cfsr |= (port_set & REG_CFSR_BFSR_MASK);
			ChangePending(IRQNUM_BUSFAULT,1);
			while (port_set & REG_CFSR_BFSR_MASK)
			{
				SIMUL_SetPort(processor, 16 + setbit(port_set & REG_CFSR_BFSR_MASK), 1, &value);
				port_set &= ~(1 << setbit(port_set));
			}
		}

		if (port_set & REG_CFSR_UFSR_MASK)
		{
			IntCtrl->regs.cfsr |= (port_set & REG_CFSR_UFSR_MASK);
			ChangePending(IRQNUM_USAGEFAULT,1);
			while (port_set & REG_CFSR_UFSR_MASK)
			{
				SIMUL_SetPort(processor, 16 + setbit(port_set & REG_CFSR_UFSR_MASK), 1, &value);
				port_set &= ~(1 << setbit(port_set));
			}
		}

		Interrupt(processor,IntCtrl,wakeUpEvent);
	}

	return SIMUL_PORT_OK;
}

static int NVIC_PortChangeHardFault(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 port_set, value = 0;

	port_set = cbs->x.port.newdata & (~cbs->x.port.olddata);

	if (port_set)
	{
		int wakeUpEvent = 0;

		if (port_set & REG_HFSR_MASK)
		{
			IntCtrl->regs.hfsr |= (port_set & REG_HFSR_MASK);
			ChangePending(IRQNUM_DEBUG,1);
			while (port_set & REG_HFSR_MASK)
			{
				SIMUL_SetPort(processor, 48 + setbit(port_set & REG_HFSR_MASK), 1, &value);
				port_set &= ~(1 << setbit(port_set));
			}
		}

		Interrupt(processor,IntCtrl,wakeUpEvent);
	}

	return SIMUL_PORT_OK;
}

static int NVIC_PortChangeDebugFault(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 port_set, value = 0;

	port_set = cbs->x.port.newdata & (~cbs->x.port.olddata);

	if (port_set)
	{
		int wakeUpEvent = 0;

		if (port_set & REG_DFSR_MASK)
		{
			IntCtrl->regs.dfsr |= (port_set & REG_DFSR_MASK);
			ChangePending(IRQNUM_DEBUG,1);
			while (port_set & REG_DFSR_MASK)
			{
				SIMUL_SetPort(processor, 80 + setbit(port_set & REG_DFSR_MASK), 1, &value);
				port_set &= ~(1 << setbit(port_set));
			}
		}

		Interrupt(processor,IntCtrl,wakeUpEvent);
	}

	return SIMUL_PORT_OK;
}

static int NVIC_ExternalIrqChange(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private, int offset, int len)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 port_set, port_clr, value = 0;
	int wakeUpEvent = 0;
	int i;

	port_set = cbs->x.port.newdata & (~cbs->x.port.olddata);

	if (port_set)
	{
		for (i=0;i<len;i++)
		{
			if (port_set & (1 << i))
			{

				int _changePendingValue = !!(1);
				if (!IntCtrl->IrqPending[16 + offset + i] && (_changePendingValue))
				{
					if (IntCtrl->regs.scr & REG_SCR_SEVONPEND) WakeUpFromWFE(processor,IntCtrl);
				}

				IntCtrl->IrqPending[16 + offset + i] = !!(_changePendingValue);



				SIMUL_SetPort(processor, offset + i, 1, &value);
			}
		}
	}

	port_clr = ~cbs->x.port.newdata & cbs->x.port.olddata;

	if (port_clr)
	{
		for (i=0;i<len;i++)
		{
			if (port_clr & (1 << i))
			{
				ChangePending(16 + offset + i,0);
			}
		}
	}

	if (port_set || port_clr)
		Interrupt(processor,IntCtrl,wakeUpEvent);

	return SIMUL_PORT_OK;
}

static int NVIC_ExternalIrqChange_0_31(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
	return NVIC_ExternalIrqChange(processor,cbs,_private,0,32);
}

static int NVIC_ExternalIrqChange_32_63(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
	return NVIC_ExternalIrqChange(processor,cbs,_private,32,32);
}

static int NVIC_ExternalIrqChange_64_95(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
	return NVIC_ExternalIrqChange(processor,cbs,_private,64,32);
}

static int NVIC_ExternalIrqChange_96_127(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
	return NVIC_ExternalIrqChange(processor,cbs,_private,96,32);
}

static int NVIC_ExternalIrqChange_128_159(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
	return NVIC_ExternalIrqChange(processor,cbs,_private,128,32);
}

static int NVIC_ExternalIrqChange_160_191(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
	return NVIC_ExternalIrqChange(processor,cbs,_private,160,32);
}

static int NVIC_ExternalIrqChange_192_223(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
	return NVIC_ExternalIrqChange(processor,cbs,_private,192,32);
}

static int NVIC_ExternalIrqChange_224_240(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
	return NVIC_ExternalIrqChange(processor,cbs,_private,224,16);
}

void WakeUpFromWFE(simulProcessor processor, IntController * IntCtrl)
{
	if (IntCtrl->regs.dhcsr & REG_DHCSR_S_SLEEP)
	{
		simulWord port = 1;
		IntCtrl->regs.dhcsr &= ~REG_DHCSR_S_SLEEP;
		SIMUL_SetPort(processor, SIMUL_PORT_INTERRUPT, 1, &port);
	}
}

void RequestReset(simulProcessor processor)
{
	simulWord port = 1;
	SIMUL_SetPort(processor, SIMUL_PORT_RESET, 1, &port);
}

int SIMULAPI SIMUL_Init(simulProcessor processor, simulCallbackStruct *cbs)
{
	IntController *IntCtrl;
	int i;
	int (* ExternalIrqsCallbacks[8])(simulProcessor, simulCallbackStruct *, simulPtr);

	strcpy(cbs->x.init.modelname, __DATE__ " Cortex-M3 Interrupt Ctrl");
	IntCtrl = (IntController*) SIMUL_Alloc(processor, sizeof(IntController));
	IntCtrl->baseaddress = BASE_ADDRESS;

	if (cbs->x.init.argc == 1)
	{
		IntCtrl->bustype = cbs->x.init.argpbustype[0];
		IntCtrl->ExternalIrqPortsFirst = 112;
		IntCtrl->ExternalIrqPortsCount = 240;
	}
	else if ((cbs->x.init.argc == 2) && (strcmp(cbs->x.init.argp[1], "noport") == 0))
	{
		IntCtrl->bustype = cbs->x.init.argpbustype[0];
		IntCtrl->ExternalIrqPortsFirst = 0;
		IntCtrl->ExternalIrqPortsCount = 0;
	}
	else if (cbs->x.init.argc == 3)
	{
		IntCtrl->bustype = cbs->x.init.argpbustype[0];
		if (cbs->x.init.argpport[1] < 112)
		{
			IntCtrl->ExternalIrqPortsFirst = 112;
			SIMUL_Warning(processor, "First external interrupt port cannot be lower than 112");
			return SIMUL_INIT_FAIL;
		}
		else
		{
			IntCtrl->ExternalIrqPortsFirst = cbs->x.init.argpport[1];
		}

		IntCtrl->ExternalIrqPortsCount = cbs->x.init.argpport[2];
	}
	else
	{
		SIMUL_Warning(processor, "Usage parameters: [(<first int port> <number of int ports>) | noport]");
		return SIMUL_INIT_FAIL;
	}

	if (IntCtrl->ExternalIrqPortsCount < 0)
	{
		SIMUL_Warning(processor, "Number of external interrupts cannot be less than zero");
		return SIMUL_INIT_FAIL;
	}

	if (IntCtrl->ExternalIrqPortsCount > 240)
	{
		SIMUL_Warning(processor, "Maximum number of external interrupts is 240");
		return SIMUL_INIT_FAIL;
	}

	SIMUL_RegisterResetCallback(processor, NVIC_Reset, (simulPtr)IntCtrl);

	SIMUL_RegisterPortChangeCallback(processor, NVIC_PortChangeInternal, (simulPtr)IntCtrl, 0, 16);
	SIMUL_RegisterPortChangeCallback(processor, NVIC_PortChangeFault, (simulPtr)IntCtrl, 16, 32);
	SIMUL_RegisterPortChangeCallback(processor, NVIC_PortChangeHardFault, (simulPtr)IntCtrl, 48, 32);
	SIMUL_RegisterPortChangeCallback(processor, NVIC_PortChangeDebugFault, (simulPtr)IntCtrl, 80, 32);

	IntCtrl->timer = SIMUL_RegisterTimerCallback(processor, NVIC_SysTick, (simulPtr) IntCtrl);
	IntCtrl->timerrun = 0;

	ExternalIrqsCallbacks[0] = NVIC_ExternalIrqChange_0_31;
	ExternalIrqsCallbacks[1] = NVIC_ExternalIrqChange_32_63;
	ExternalIrqsCallbacks[2] = NVIC_ExternalIrqChange_64_95;
	ExternalIrqsCallbacks[3] = NVIC_ExternalIrqChange_96_127;
	ExternalIrqsCallbacks[4] = NVIC_ExternalIrqChange_128_159;
	ExternalIrqsCallbacks[5] = NVIC_ExternalIrqChange_160_191;
	ExternalIrqsCallbacks[6] = NVIC_ExternalIrqChange_192_223;
	ExternalIrqsCallbacks[7] = NVIC_ExternalIrqChange_224_240;

	for (i=0;i<IntCtrl->ExternalIrqPortsCount;i+=32)
	{
		int width = ((i + 31) < IntCtrl->ExternalIrqPortsCount)?(32):(IntCtrl->ExternalIrqPortsCount - i);
		SIMUL_RegisterPortChangeCallback(processor, ExternalIrqsCallbacks[i >> 5], (simulPtr)IntCtrl, i + IntCtrl->ExternalIrqPortsFirst, width);
	}

	NVIC_Init(processor,IntCtrl);
	NVIC_Reset(processor,cbs,IntCtrl);

	return SIMUL_INIT_OK;
}

/************************* Bus Read, Write ************************/
int BusRead(simulBusCallbackStruct * bus, simulWord32 * reg)
{
	switch (bus->width)
	{
		case 8:
			*reg = (*reg & ~(0xff << ((bus->address & 0x3) << 3))) | ((bus->data&0xFF) << ((bus->address & 0x3) << 3));
			break;
		case 16:
			if (bus->address & 0x1) return SIMUL_MEMORY_FAIL;
			*reg = (*reg & ~(0xffff << ((bus->address & 0x3) << 3))) | ((bus->data&0xFFFF) << ((bus->address & 0x3) << 3));
			break;
		case 32:
			if (bus->address & 0x3) return SIMUL_MEMORY_FAIL;
			*reg = bus->data;
			break;
		default:
			return SIMUL_MEMORY_FAIL;
	}
	return SIMUL_MEMORY_OK;
}

int BusWrite(simulBusCallbackStruct * bus, simulWord32 * reg)
{
	switch (bus->width)
	{
	case 8:
		bus->data = (*reg >> ((bus->address & 0x3) << 3)) & 0xff;
		break;
	case 16:
		if (bus->address & 0x1) return SIMUL_MEMORY_FAIL;
		bus->data = (*reg >> ((bus->address & 0x3) << 3)) & 0xffff;
		break;
	case 32:
		if (bus->address & 0x3) return SIMUL_MEMORY_FAIL;
		bus->data = *reg;
		break;
	default:
		return SIMUL_MEMORY_FAIL;
	}
	return SIMUL_MEMORY_OK;
}

void Interrupt(simulProcessor processor, simulPtr _private,int wakeUpEvent)
{
	IntController *IntCtrl = (IntController*) _private;
	int i;
	int pending = 0;
	simulWord port;

	if (wakeUpEvent) return;

	if (IntCtrl->IrqPending[IRQNUM_USAGEFAULT] && !IntCtrl->IrqEnable[IRQNUM_USAGEFAULT])
	{
		IntCtrl->IrqPending[IRQNUM_USAGEFAULT] = 0;
		IntCtrl->IrqPending[IRQNUM_HARDFAULT] = 1;
		IntCtrl->regs.hfsr |= REG_HFSR_FORCED;
	}

	if (IntCtrl->IrqPending[IRQNUM_MEMFAULT] && !IntCtrl->IrqEnable[IRQNUM_MEMFAULT])
	{
		IntCtrl->IrqPending[IRQNUM_MEMFAULT] = 0;
		IntCtrl->IrqPending[IRQNUM_HARDFAULT] = 1;
		IntCtrl->regs.hfsr |= REG_HFSR_FORCED;
	}

	if (IntCtrl->IrqPending[IRQNUM_BUSFAULT] && !IntCtrl->IrqEnable[IRQNUM_BUSFAULT])
	{
		IntCtrl->IrqPending[IRQNUM_BUSFAULT] = 0;
		IntCtrl->IrqPending[IRQNUM_HARDFAULT] = 1;
		IntCtrl->regs.hfsr |= REG_HFSR_FORCED;
	}

	for (i=0;i<256;i++)
	{
		if (IntCtrl->IrqPending[i] & IntCtrl->IrqEnable[i]) pending++;
	}

	port = pending > 0;

	SIMUL_SetPort(processor, SIMUL_PORT_INTERRUPT, 1, &port);
}

