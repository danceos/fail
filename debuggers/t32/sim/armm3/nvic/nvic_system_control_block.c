#include "nvic.h"

/*************** Commands from ARM Simulator				***************/

int CMD_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord reg;

	cbs->x.bus.clocks = 1;

	if (cbs->x.bus.cycletype==SIMUL_MEMORY_HIDDEN)
	{
		return BusWrite(&cbs->x.bus, &IntCtrl->regs.cmd);
	}
	else
	{
		reg = 0;
		return BusWrite(&cbs->x.bus, &reg);
	}
}

int CMD_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;

	cbs->x.bus.clocks = 1;

	if (cbs->x.bus.cycletype==SIMUL_MEMORY_HIDDEN)
	{
		simulWord32 cmd;

		BusRead(&cbs->x.bus, &cmd);

		switch ((cmd>>24) & 0xFF)
		{
			case CMD_IS_PRESENT:
				{
					int i;

					IntCtrl->regs.cmd = 0;

					for (i=0;i<24;i++)
					{
						if ((1<<i) & cmd)
						{
							IntCtrl->regs.cmd |= 1 << (24-i);
						}
					}
				}
				break;

			case CMD_SET_INTERRUPT_ACTIVE:
				{
					/* command for setting interrupt to active state */
					int isrnum = cmd & 0x1FF;

					switch (isrnum)
					{
						case 0:
							Interrupt(processor,IntCtrl,0);
							break;

						case IRQNUM_RESET:
						case IRQNUM_NMI:
						case IRQNUM_HARDFAULT:
						case IRQNUM_MEMFAULT:
						case IRQNUM_BUSFAULT:
						case IRQNUM_USAGEFAULT:
						case IRQNUM_SVCALL:
						case IRQNUM_DEBUG:
						case IRQNUM_PENDSV:
						case IRQNUM_SYSTICK:
							IntCtrl->CurrentIrqNum = isrnum;
							ChangeActive(isrnum,1);
							ChangePending(isrnum,0);
							Interrupt(processor,IntCtrl,0);
							break;
						
						default:
							if (isrnum > 15 && isrnum < 256)
							{
								IntCtrl->CurrentIrqNum = isrnum;
								ChangeActive(isrnum,1);
								ChangePending(isrnum,0);
								Interrupt(processor,IntCtrl,0);
							}
							break;
					}
				}
				break;

			case CMD_SET_INTERRUPT_INACTIVE:
				{
					/* command for setting interrupt inactive */
					int isrnum = cmd & 0x1FF;

					switch (isrnum)
					{
						case IRQNUM_RESET:
						case IRQNUM_NMI:
						case IRQNUM_HARDFAULT:
						case IRQNUM_MEMFAULT:
						case IRQNUM_BUSFAULT:
						case IRQNUM_USAGEFAULT:
						case IRQNUM_SVCALL:
						case IRQNUM_DEBUG:
						case IRQNUM_PENDSV:
						case IRQNUM_SYSTICK:
							IntCtrl->CurrentIrqNum = 0;
							ChangeActive(isrnum,0);
							Interrupt(processor,IntCtrl,0);
							break;
						
						default:
							if (isrnum > 15 && isrnum < 256)
							{
								IntCtrl->CurrentIrqNum = 0;
								ChangeActive(isrnum,0);
								Interrupt(processor,IntCtrl,0);
							}
							break;
					}
				}
				break;

			case CMD_GET_EXCEPTION_PRIORITY:
				{
					/* command for getting priority of exception */
					int isrnum = cmd & 0x1FF;
					
					if (isrnum < 256)
						IntCtrl->regs.cmd = IntCtrl->IrqPriority[isrnum];
					else 
						IntCtrl->regs.cmd = 0;
				}
				break;

			case CMD_GET_HIGHEST_PRIORITY:
				{
					/* command for computing highest priority */
					int highestpri = 256;
					int groupshift = (IntCtrl->regs.aircr >> 8) & 7; /* Application Interrupt and Reset Control Register - PRIGROUP */
					int groupvalue = 2 << groupshift;
					int i;

					for (i=2;i<256;i++)
					{
						if (IntCtrl->IrqActive[i])
						{
							if (IntCtrl->IrqPriority[i] < highestpri)
							{
								int subgroupvalue;

								highestpri = IntCtrl->IrqPriority[i];

								subgroupvalue = highestpri % groupvalue;
								highestpri = highestpri - subgroupvalue;
							}
						}
					}

					IntCtrl->regs.cmd = highestpri;
				}
				break;

			case CMD_GET_ACTIVE_EXCEPTION_COUNT:
				{
					/* get active exception count */
					int i,count=0;

					for (i=0;i<256;i++)
					{
						if (IntCtrl->IrqActive[i]) count++;
					}

					IntCtrl->regs.cmd = count;
				}
				break;

			case CMD_IS_EXCEPTION_ACTIVE:
				{
					/* is exception active? */
					int isrnum = cmd & 0x1FF;

					if (isrnum < 256)
						IntCtrl->regs.cmd = IntCtrl->IrqActive[isrnum];
					else
						IntCtrl->regs.cmd = 0;
				}
				break;

			case CMD_ESCALATE_TO_HARD_FAULT:
				{
					/* escalate to HardFault */
					int isrnum = cmd & 0x1FF;
					if (isrnum < 256)
					{
						IntCtrl->IrqPending[isrnum] = 0;
						IntCtrl->IrqPending[IRQNUM_HARDFAULT] = 1;
						IntCtrl->regs.hfsr |= REG_HFSR_FORCED;
						Interrupt(processor,IntCtrl,0);
					}
				}
				break;

			case CMD_CHANGE_ACTIVE_EXCEPTION:
				{
					/* change active exception */
					int active = cmd & 0x1FF;

					if ((active > 2) && (active < 256))
					{
						IntCtrl->CurrentIrqNum = active;
						IntCtrl->IrqActive[active] = 1;
					}
				}
				break;
		}



	}

	return SIMUL_MEMORY_OK;
}


/*************** Interrupt Control State Register 				***************/

int ICSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int i;

	simulWord32 NMIPENDSET;
	simulWord32 PENDSVSET;
	simulWord32 PENDSTSET;
	simulWord32 ISRPENDING;
	simulWord32 VECTPENDING;
	simulWord32 RETTOBASE;
	simulWord32 VECTACTIVE;
	
	simulWord32 reg = 0;

	int highestPriorityPending = 256;
	int groupshift = (IntCtrl->regs.aircr >> 8) & 7; /* Application Interrupt and Reset Control Register - PRIGROUP */
	int groupvalue = 2 << groupshift;

	cbs->x.bus.clocks = 1;

	NMIPENDSET = !!IntCtrl->IrqPending[IRQNUM_NMI];
	PENDSVSET = !!IntCtrl->IrqPending[IRQNUM_PENDSV];
	PENDSTSET = !!IntCtrl->IrqPending[IRQNUM_SYSTICK];

	ISRPENDING = 0;
	for (i=16;i<256;i++)
	{
		if (IntCtrl->IrqPending[i])
		{
			ISRPENDING = 1;
			break;
		}
	}
	
	RETTOBASE = 1;
	for (i=0;i<256;i++)
	{
		if (RETTOBASE && IntCtrl->IrqActive[i])
		{
			if (i != IntCtrl->CurrentIrqNum)
			{
				RETTOBASE = 0;
				break;
			}
		}
	}

	VECTPENDING = 0;
	for (i=2;i<256;i++)
	{
		if (IntCtrl->IrqPending[i])
		{
			if (IntCtrl->IrqPriority[i] < highestPriorityPending)
			{
				int subgroupvalue;

				highestPriorityPending = IntCtrl->IrqPriority[i];

				subgroupvalue = highestPriorityPending % groupvalue;
				highestPriorityPending = highestPriorityPending - subgroupvalue;

				VECTPENDING = i;
			}
		}
	}

	VECTACTIVE = IntCtrl->CurrentIrqNum & 0x1FF;

	reg = (NMIPENDSET << 31) | (PENDSVSET << 28) | (PENDSTSET << 26) | (ISRPENDING << 22) | (VECTPENDING << 12) | (RETTOBASE << 11) | (VECTACTIVE << 0);

	return BusWrite(&cbs->x.bus, &reg);
}

int ICSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	int wakeUpEvent = 0;
	cbs->x.bus.clocks = 1;
	
	BusRead(&cbs->x.bus, &reg);
	
	if (reg & REG_ICSR_NMIPENDSET)
	{
		ChangePending(IRQNUM_NMI,1);
		Interrupt(processor,IntCtrl,wakeUpEvent);
	}

	if (reg & REG_ICSR_PENDSVSET)
	{
		ChangePending(IRQNUM_PENDSV,1);
		Interrupt(processor,IntCtrl,wakeUpEvent);
	}
	
	if (reg & REG_ICSR_PENDSVCLR)
	{
		ChangePending(IRQNUM_PENDSV,0);
		Interrupt(processor,IntCtrl,wakeUpEvent);
	}

	if (reg & REG_ICSR_PENDSTSET)
	{
		ChangePending(IRQNUM_SYSTICK,1);
		Interrupt(processor,IntCtrl,wakeUpEvent);
	}
	else if (reg & REG_ICSR_PENDSTCLR)
	{
		ChangePending(IRQNUM_SYSTICK,0);
		Interrupt(processor,IntCtrl,wakeUpEvent);
	}

	return SIMUL_MEMORY_OK;
}

/*************** Vector Table Offset Register     				***************/

int VTOR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	cbs->x.bus.clocks = 1;
	return BusWrite(&cbs->x.bus, &IntCtrl->regs.vtor);
}

int VTOR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;

	cbs->x.bus.clocks = 1;

	reg = IntCtrl->regs.vtor;

	BusRead(&cbs->x.bus, &reg);

	IntCtrl->regs.vtor = reg & 0x3FFFFF80;

	return SIMUL_MEMORY_OK;
}

/*************** Application Interrupt/Reset Control Register	***************/

int AIRCR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;
	
	reg = (IntCtrl->regs.aircr & REG_AIRCR_MASK) | 0xFA050000;

	return BusWrite(&cbs->x.bus, &reg);
}

int AIRCR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;

	reg = IntCtrl->regs.aircr;

	BusRead(&cbs->x.bus, &reg);

	if ((reg & 0xFFFF0000) == 0x05FA0000)
	{
		IntCtrl->regs.aircr = (IntCtrl->regs.aircr & ~REG_AIRCR_PRIGROUP) | (reg & REG_AIRCR_PRIGROUP);

		IntCtrl->regs.aircr = (IntCtrl->regs.aircr & ~REG_AIRCR_SYSRESETREQ) | (reg & REG_AIRCR_SYSRESETREQ);

		if (reg & REG_AIRCR_SYSRESETREQ)
		{
			RequestReset(processor);
		}

		if (reg & REG_AIRCR_VECTCLRACTIVE)
		{
			//not imlemented
			//ClearActiveState();
		}

		if (reg & REG_AIRCR_VECTRESET)
		{
			RequestReset(processor);
		}
	}

	return SIMUL_MEMORY_OK;
}

/*************** System Control Register						***************/

int SCR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;
	
	reg = (IntCtrl->regs.scr & 0x00000016);

	return BusWrite(&cbs->x.bus, &reg);
}

int SCR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = IntCtrl->regs.scr;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus, &reg);

	IntCtrl->regs.scr = reg & REG_SCR_MASK;

	return SIMUL_MEMORY_OK;
}

/*************** Configuration Control Register					***************/

int CCR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg;
	cbs->x.bus.clocks = 1;
	
	reg = (IntCtrl->regs.ccr & REG_CCR_MASK);

	return BusWrite(&cbs->x.bus, &reg);
}

int CCR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = IntCtrl->regs.ccr;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus, &reg);

	IntCtrl->regs.ccr = reg & REG_CCR_MASK;

	if (IntCtrl->regs.ccr & REG_CCR_USERSETMPEND)
	{
		//This bit functionality cannot be implemented - NVIC model doesn't have access to CONTROL register
	}

	return SIMUL_MEMORY_OK;
}

/*************** System Handlers x Priority Register			***************/

int SHPR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = (cbs->x.bus.address - IntCtrl->baseaddress - SHPR_OFFSET) >> 2;
	simulWord32 reg = 0;
	
	cbs->x.bus.clocks = 1;

	switch (idx)
	{
		case 0: /* PRI_4 -- PRI_7 */
			reg = ((IntCtrl->IrqPriority[IRQNUM_MEMFAULT] & 0xFF) << 0) | ((IntCtrl->IrqPriority[IRQNUM_BUSFAULT] & 0xFF) << 8) | ((IntCtrl->IrqPriority[IRQNUM_USAGEFAULT] & 0xFF) << 16);
			break;

		case 1: /* PRI_8 -- PRI_11 */
			reg = ((IntCtrl->IrqPriority[IRQNUM_SVCALL] & 0xFF) << 24);
			break;

		case 2: /* PRI_12 -- PRI_15 */
			reg = ((IntCtrl->IrqPriority[IRQNUM_DEBUG] & 0xFF) << 0) | ((IntCtrl->IrqPriority[IRQNUM_PENDSV] & 0xFF) << 16) | ((IntCtrl->IrqPriority[IRQNUM_SYSTICK] & 0xFF) << 24);
			break;

		default:
			reg = 0;
			break;
	}

	return BusWrite(&cbs->x.bus, &reg);
}

int SHPR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int idx = (cbs->x.bus.address - IntCtrl->baseaddress - SHPR_OFFSET) >> 2;
	simulWord32 reg = 0;
//	simulWord32 regBackup;

	cbs->x.bus.clocks = 1;

	switch (idx)
	{
		case 0: /* PRI_4 -- PRI_7 */
			reg = ((IntCtrl->IrqPriority[IRQNUM_MEMFAULT] & 0xFF) << 0) | ((IntCtrl->IrqPriority[IRQNUM_BUSFAULT] & 0xFF) << 8) | ((IntCtrl->IrqPriority[IRQNUM_USAGEFAULT] & 0xFF) << 16);
			break;

		case 1: /* PRI_8 -- PRI_11 */
			reg = ((IntCtrl->IrqPriority[IRQNUM_SVCALL] & 0xFF) << 24);
			break;

		case 2: /* PRI_12 -- PRI_15 */
			reg = ((IntCtrl->IrqPriority[IRQNUM_DEBUG] & 0xFF) << 0) | ((IntCtrl->IrqPriority[IRQNUM_PENDSV] & 0xFF) << 16) | ((IntCtrl->IrqPriority[IRQNUM_SYSTICK] & 0xFF) << 24);
			break;

		default:
			reg = 0;
			break;
	}

//	regBackup = reg;
	BusRead(&cbs->x.bus, &reg);

	switch (idx)
	{
		case 0: /* PRI_4 -- PRI_7 */
			IntCtrl->IrqPriority[IRQNUM_MEMFAULT] = (reg >> 0) & 0xFF;
			IntCtrl->IrqPriority[IRQNUM_BUSFAULT] = (reg >> 8) & 0xFF;
			IntCtrl->IrqPriority[IRQNUM_USAGEFAULT] = (reg >> 16) & 0xFF;
			break;

		case 1: /* PRI_8 -- PRI_11 */
			IntCtrl->IrqPriority[IRQNUM_SVCALL] = (reg >> 24) & 0xFF;
			break;

		case 2: /* PRI_12 -- PRI_15 */
			IntCtrl->IrqPriority[IRQNUM_DEBUG] = (reg >> 0) & 0xFF;
			IntCtrl->IrqPriority[IRQNUM_PENDSV] = (reg >> 16) & 0xFF;
			IntCtrl->IrqPriority[IRQNUM_SYSTICK] = (reg >> 24) & 0xFF;
			break;

		default:
			return SIMUL_MEMORY_OK;
	}

	return SIMUL_MEMORY_OK;
}

/*************** System Handler Control and State Register		***************/

int SHCSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;

	simulWord32 USGFAULTENA, BUSFAULTENA, MEMFAULTENA;
	simulWord32 SVCALLPENDED, BUSFAULTPENDED, MEMFAULTPENDED, USGFAULTPENDED;
	simulWord32 SYSTICKACT, PENDSVACT, MONITORACT, SVCALLACT, USGFAULTACT, BUSFAULTACT, MEMFAULTACT;

	simulWord32 reg = 0;

	cbs->x.bus.clocks = 1;

	USGFAULTENA = IntCtrl->IrqEnable[IRQNUM_USAGEFAULT];
	BUSFAULTENA = IntCtrl->IrqEnable[IRQNUM_BUSFAULT];
	MEMFAULTENA = IntCtrl->IrqEnable[IRQNUM_MEMFAULT];
	
	SVCALLPENDED = IntCtrl->IrqPending[IRQNUM_SVCALL];
	BUSFAULTPENDED = IntCtrl->IrqPending[IRQNUM_BUSFAULT];
	MEMFAULTPENDED = IntCtrl->IrqPending[IRQNUM_MEMFAULT];
	USGFAULTPENDED = IntCtrl->IrqPending[IRQNUM_USAGEFAULT];

	SYSTICKACT = IntCtrl->IrqActive[IRQNUM_SYSTICK];
	PENDSVACT = IntCtrl->IrqActive[IRQNUM_PENDSV];
	MONITORACT = IntCtrl->IrqActive[IRQNUM_DEBUG];
	SVCALLACT = IntCtrl->IrqActive[IRQNUM_SVCALL];
	USGFAULTACT = IntCtrl->IrqActive[IRQNUM_USAGEFAULT];
	BUSFAULTACT = IntCtrl->IrqActive[IRQNUM_BUSFAULT];
	MEMFAULTACT = IntCtrl->IrqActive[IRQNUM_MEMFAULT];

	reg = 
		((USGFAULTENA)?(REG_SHCSR_USGFAULTENA):(0)) |
		((BUSFAULTENA)?(REG_SHCSR_BUSFAULTENA):(0)) |
		((MEMFAULTENA)?(REG_SHCSR_MEMFAULTENA):(0)) |

		((SVCALLPENDED)?(REG_SHCSR_SVCALLPENDED):(0)) |
		((BUSFAULTPENDED)?(REG_SHCSR_BUSFAULTPENDED):(0)) |
		((MEMFAULTPENDED)?(REG_SHCSR_MEMFAULTPENDED):(0)) |
		((USGFAULTPENDED)?(REG_SHCSR_USGFAULTPENDED):(0)) |

		((SYSTICKACT)?(REG_SHCSR_SYSTICKACT):(0)) |
		((PENDSVACT)?(REG_SHCSR_PENDSVACT):(0)) |
		((MONITORACT)?(REG_SHCSR_MONITORACT):(0)) |
		((SVCALLACT)?(REG_SHCSR_SVCALLACT):(0)) |
		((USGFAULTACT)?(REG_SHCSR_USGFAULTACT):(0)) |
		((BUSFAULTACT)?(REG_SHCSR_BUSFAULTACT):(0)) |
		((MEMFAULTACT)?(REG_SHCSR_MEMFAULTACT):(0));
		;

	return BusWrite(&cbs->x.bus, &reg);
}

int SHCSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	int wakeUpEvent = 0;

	simulWord32 USGFAULTENA, BUSFAULTENA, MEMFAULTENA;
	simulWord32 SVCALLPENDED, BUSFAULTPENDED, MEMFAULTPENDED, USGFAULTPENDED;
	simulWord32 SYSTICKACT, PENDSVACT, MONITORACT, SVCALLACT, USGFAULTACT, BUSFAULTACT, MEMFAULTACT;

	simulWord32 reg = 0;

	cbs->x.bus.clocks = 1;

	USGFAULTENA = IntCtrl->IrqEnable[IRQNUM_USAGEFAULT];
	BUSFAULTENA = IntCtrl->IrqEnable[IRQNUM_BUSFAULT];
	MEMFAULTENA = IntCtrl->IrqEnable[IRQNUM_MEMFAULT];
	
	SVCALLPENDED = IntCtrl->IrqPending[IRQNUM_SVCALL];
	BUSFAULTPENDED = IntCtrl->IrqPending[IRQNUM_BUSFAULT];
	MEMFAULTPENDED = IntCtrl->IrqPending[IRQNUM_MEMFAULT];
	USGFAULTPENDED = IntCtrl->IrqPending[IRQNUM_USAGEFAULT];

	SYSTICKACT = IntCtrl->IrqActive[IRQNUM_SYSTICK];
	PENDSVACT = IntCtrl->IrqActive[IRQNUM_PENDSV];
	MONITORACT = IntCtrl->IrqActive[IRQNUM_DEBUG];
	SVCALLACT = IntCtrl->IrqActive[IRQNUM_SVCALL];
	USGFAULTACT = IntCtrl->IrqActive[IRQNUM_USAGEFAULT];
	BUSFAULTACT = IntCtrl->IrqActive[IRQNUM_BUSFAULT];
	MEMFAULTACT = IntCtrl->IrqActive[IRQNUM_MEMFAULT];

	reg = 
		((USGFAULTENA)?(REG_SHCSR_USGFAULTENA):(0)) |
		((BUSFAULTENA)?(REG_SHCSR_BUSFAULTENA):(0)) |
		((MEMFAULTENA)?(REG_SHCSR_MEMFAULTENA):(0)) |

		((SVCALLPENDED)?(REG_SHCSR_SVCALLPENDED):(0)) |
		((BUSFAULTPENDED)?(REG_SHCSR_BUSFAULTPENDED):(0)) |
		((MEMFAULTPENDED)?(REG_SHCSR_MEMFAULTPENDED):(0)) |
		((USGFAULTPENDED)?(REG_SHCSR_USGFAULTPENDED):(0)) |

		((SYSTICKACT)?(REG_SHCSR_SYSTICKACT):(0)) |
		((PENDSVACT)?(REG_SHCSR_PENDSVACT):(0)) |
		((MONITORACT)?(REG_SHCSR_MONITORACT):(0)) |
		((SVCALLACT)?(REG_SHCSR_SVCALLACT):(0)) |
		((USGFAULTACT)?(REG_SHCSR_USGFAULTACT):(0)) |
		((BUSFAULTACT)?(REG_SHCSR_BUSFAULTACT):(0)) |
		((MEMFAULTACT)?(REG_SHCSR_MEMFAULTACT):(0));
		;

	BusRead(&cbs->x.bus, &reg);

	ChangeEnable(IRQNUM_USAGEFAULT,reg & REG_SHCSR_USGFAULTENA);
	ChangeEnable(IRQNUM_BUSFAULT,reg & REG_SHCSR_BUSFAULTENA);
	ChangeEnable(IRQNUM_MEMFAULT,reg & REG_SHCSR_MEMFAULTENA);

	ChangePending(IRQNUM_SVCALL,	reg & REG_SHCSR_SVCALLPENDED);
	ChangePending(IRQNUM_BUSFAULT,	reg & REG_SHCSR_BUSFAULTPENDED);
	ChangePending(IRQNUM_MEMFAULT,	reg & REG_SHCSR_MEMFAULTPENDED);
	ChangePending(IRQNUM_USAGEFAULT,reg & REG_SHCSR_USGFAULTPENDED);

	ChangeActive(IRQNUM_SYSTICK,reg & REG_SHCSR_SYSTICKACT);
	ChangeActive(IRQNUM_PENDSV,reg & REG_SHCSR_PENDSVACT);
	ChangeActive(IRQNUM_DEBUG,reg & REG_SHCSR_MONITORACT);
	ChangeActive(IRQNUM_SVCALL,reg & REG_SHCSR_SVCALLACT);
	ChangeActive(IRQNUM_USAGEFAULT,reg & REG_SHCSR_USGFAULTACT);
	ChangeActive(IRQNUM_BUSFAULT,reg & REG_SHCSR_BUSFAULTACT);
	ChangeActive(IRQNUM_MEMFAULT,reg & REG_SHCSR_MEMFAULTACT);

	Interrupt(processor,IntCtrl,wakeUpEvent);

	return SIMUL_MEMORY_OK;
}

/*************** Configurable Fault Status Register				***************/

int CFSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0;

	cbs->x.bus.clocks = 1;

	reg = IntCtrl->regs.cfsr & REG_CFSR_MASK;

	return BusWrite(&cbs->x.bus, &reg);
}

int CFSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus, &reg);

	IntCtrl->regs.cfsr &= ~(reg & REG_CFSR_MASK);

	return SIMUL_MEMORY_OK;
}

/*************** Hard Fault Status Register						***************/

int HFSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0;

	cbs->x.bus.clocks = 1;

	reg = IntCtrl->regs.hfsr & REG_HFSR_MASK;

	return BusWrite(&cbs->x.bus, &reg);
}

int HFSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus, &reg);

	IntCtrl->regs.hfsr &= ~(reg & REG_HFSR_MASK);

	return SIMUL_MEMORY_OK;
}

/*************** Debug Fault Status Register     				***************/

int DFSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0;

	cbs->x.bus.clocks = 1;

	reg = IntCtrl->regs.dfsr & REG_DFSR_MASK;

	return BusWrite(&cbs->x.bus, &reg);
}

int DFSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus, &reg);

	IntCtrl->regs.dfsr &= ~(reg & REG_DFSR_MASK);

	return SIMUL_MEMORY_OK;
}

/*************** Memory Manage Address Register  				***************/

int MMAR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	cbs->x.bus.clocks = 1;
	return BusWrite(&cbs->x.bus, &IntCtrl->regs.mmar);
}

int MMAR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = IntCtrl->regs.mmar;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus, &reg);

	IntCtrl->regs.mmar = reg;

	return SIMUL_MEMORY_OK;
}

/*************** Bus Fault Address Register      				***************/

int BFAR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	cbs->x.bus.clocks = 1;
	return BusWrite(&cbs->x.bus, &IntCtrl->regs.bfar);
}

int BFAR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = IntCtrl->regs.bfar;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus, &reg);

	IntCtrl->regs.bfar = reg;

	return SIMUL_MEMORY_OK;
}

/*************** Auxiliary Fault Status Register 				***************/

int AFSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = IntCtrl->regs.afsr;

	cbs->x.bus.clocks = 1;

	return BusWrite(&cbs->x.bus, &reg);
}

int AFSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus, &reg);

	IntCtrl->regs.afsr &= ~(reg & REG_AFSR_MASK);

	return SIMUL_MEMORY_OK;
}

/*************** Debug Halting Control and Status Register 				***************/

int DHCSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = IntCtrl->regs.dhcsr;

	cbs->x.bus.clocks = 1;

	return BusWrite(&cbs->x.bus, &reg);
}

int DHCSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr _private)
{
    IntController *IntCtrl = (IntController*) _private;
	simulWord32 reg = 0;

	cbs->x.bus.clocks = 1;

	BusRead(&cbs->x.bus, &reg);

	IntCtrl->regs.dhcsr = reg;

	return SIMUL_MEMORY_OK;
}
