//#pragma warning (disable : 4100)

#include "simul.h"

/* Definitions */
#define BASE_ADDRESS         0xE000E000 /* base address of controller */

#define CMD_OFFSET               0x0000 /* Commands from ARM Simulator       */
#define ICTR_OFFSET              0x0004 /* Interrupt Control Type Register       */
#define STCSR_OFFSET             0x0010 /* SysTick Control and Status Register   */
#define STRVR_OFFSET             0x0014 /* SysTick Reload Value Register         */
#define STCVR_OFFSET             0x0018 /* SysTick Current Value Register        */
#define STCLVR_OFFSET            0x001C /* SysTick Calibration Value Register    */
#define IRQSETENR_OFFSET		 0x0100 /* Irq x Set Enable Register       */
#define IRQCLRENR_OFFSET		 0x0180 /* Irq x Clear Enable Register     */
#define IRQSETPER_OFFSET		 0x0200 /* Irq x Set Pending Register      */
#define IRQCLRPER_OFFSET		 0x0280 /* Irq x Clear Pending Register    */
#define IRQABR_OFFSET			 0x0300 /* Irq x Active Bit Register       */
#define IRQPR_OFFSET			 0x0400 /* Irq x Priority Register          */
#define CPUIDR_OFFSET            0x0D00 /* CPUID Base Register              */
#define ICSR_OFFSET              0x0D04 /* Interrupt Control State Register */
#define VTOR_OFFSET              0x0D08 /* Vector Table Offset Register     */
#define AIRCR_OFFSET             0x0D0C /* Application Interrupt/Reset Control Register */
#define SCR_OFFSET               0x0D10 /* System Control Register                      */
#define CCR_OFFSET               0x0D14 /* Configuration Control Register               */
#define SHPR_OFFSET				 0x0D18 /* System Handlers x Priority Register        */
#define SHCSR_OFFSET             0x0D24 /* System Handler Control and State Register   */
#define CFSR_OFFSET              0x0D28 /* Configurable Fault Status Register           */
#define HFSR_OFFSET              0x0D2C /* Hard Fault Status Register      */
#define DFSR_OFFSET              0x0D30 /* Debug Fault Status Register     */
#define MMAR_OFFSET              0x0D34 /* Memory Manage Address Register  */
#define BFAR_OFFSET              0x0D38 /* Bus Fault Address Register      */
#define AFSR_OFFSET              0x0D3C /* Auxiliary Fault Status Register      */
#define PFR0_OFFSET              0x0D40 /* Processor Feature Register 0    */
#define PFR1_OFFSET              0x0D44 /* Processor Feature Register 1    */
#define DFR0_OFFSET              0x0D48 /* Debug Feature Register 0        */
#define AFR0_OFFSET              0x0D4C /* Auxiliary Feature Register 0    */
#define MMFR0_OFFSET             0x0D50 /* Memory Model Feature Register 0 */
#define MMFR1_OFFSET             0x0D54 /* Memory Model Feature Register 1 */
#define MMFR2_OFFSET             0x0D58 /* Memory Model Feature Register 2 */
#define MMFR3_OFFSET             0x0D5C /* Memory Model Feature Register 3 */
#define ISAR0_OFFSET             0x0D60 /* ISA Feature Register 0 */
#define ISAR1_OFFSET             0x0D64 /* ISA Feature Register 1 */
#define ISAR2_OFFSET             0x0D68 /* ISA Feature Register 2 */
#define ISAR3_OFFSET             0x0D6C /* ISA Feature Register 3 */
#define ISAR4_OFFSET             0x0D70 /* ISA Feature Register 4 */
#define STIR_OFFSET              0x0F00 /* Software Trigger Interrupt Register  */
#define PID4_OFFSET         	 0x0FD0 /* Peripheral Identification Register 4 */
#define PID5_OFFSET         	 0x0FD4 /* Peripheral Identification Register 5 */
#define PID6_OFFSET         	 0x0FD8 /* Peripheral Identification Register 6 */
#define PID7_OFFSET         	 0x0FDC /* Peripheral Identification Register 7 */
#define PID0_OFFSET         	 0x0FE0 /* Peripheral Identification Register 0 */
#define PID1_OFFSET         	 0x0FE4 /* Peripheral Identification Register 1 */
#define PID2_OFFSET         	 0x0FE8 /* Peripheral Identification Register 2 */
#define PID3_OFFSET         	 0x0FEC /* Peripheral Identification Register 3 */
#define CID0_OFFSET          	 0x0FF0 /* Component Identification Register 0  */
#define CID1_OFFSET          	 0x0FF4 /* Component Identification Register 1  */
#define CID2_OFFSET          	 0x0FF8 /* Component Identification Register 2  */
#define CID3_OFFSET          	 0x0FFC /* Component Identification Register 3  */
#define DHCSR_OFFSET          	 0x0DF0 /* Debug Halting Control and Status Register */
#ifdef _DEBUG
#define GPIO_OFFSET          	 0x6000340C
#endif


#define CMD_SIZE                 4 /* Commands Register       */
#define ICTR_SIZE                4 /* Interrupt Control Type Register       */
#define STCSR_SIZE               4 /* SysTick Control and Status Register   */
#define STRVR_SIZE               4 /* SysTick Reload Value Register         */
#define STCVR_SIZE               4 /* SysTick Current Value Register        */
#define STCLVR_SIZE              4 /* SysTick Calibration Value Register    */
#define IRQSETENR_SIZE           32 /* Irq x Set Enable Register       */
#define IRQCLRENR_SIZE           32 /* Irq x Clear Enable Register     */
#define IRQSETPER_SIZE           32 /* Irq x Set Pending Register      */
#define IRQCLRPER_SIZE           32 /* Irq x Clear Pending Register    */
#define IRQABR_SIZE              32 /* Irq x Active Bit Register       */
#define IRQPR_SIZE               240 /* Irq x Priority Register          */
#define CPUIDR_SIZE              4 /* CPUID Base Register              */
#define ICSR_SIZE                4 /* Interrupt Control State Register */
#define VTOR_SIZE                4 /* Vector Table Offset Register     */
#define AIRCR_SIZE               4 /* Application Interrupt/Reset Control Register */
#define SCR_SIZE                 4 /* System Control Register                      */
#define CCR_SIZE                 4 /* Configuration Control Register               */
#define SHPR_SIZE                12 /* System Handlers x Priority Register        */
#define SHCSR_SIZE               4 /* System Handler Control and State Register   */
#define CFSR_SIZE                4 /* Configurable Fault Status Register           */
#define HFSR_SIZE                4 /* Hard Fault Status Register      */
#define DFSR_SIZE                4 /* Debug Fault Status Register     */
#define MMAR_SIZE                4 /* Memory Manage Address Register  */
#define BFAR_SIZE                4 /* Bus Fault Address Register      */
#define AFSR_SIZE                4 /* Auxiliary Fault Status Register      */
#define PFR0_SIZE                4 /* Processor Feature Register 0    */
#define PFR1_SIZE                4 /* Processor Feature Register 1    */
#define DFR0_SIZE                4 /* Debug Feature Register 0        */
#define AFR0_SIZE                4 /* Auxiliary Feature Register 0    */
#define MMFR0_SIZE               4 /* Memory Model Feature Register 0 */
#define MMFR1_SIZE               4 /* Memory Model Feature Register 1 */
#define MMFR2_SIZE               4 /* Memory Model Feature Register 2 */
#define MMFR3_SIZE               4 /* Memory Model Feature Register 3 */
#define ISAR0_SIZE               4 /* ISA Feature Register 0 */
#define ISAR1_SIZE               4 /* ISA Feature Register 1 */
#define ISAR2_SIZE               4 /* ISA Feature Register 2 */
#define ISAR3_SIZE               4 /* ISA Feature Register 3 */
#define ISAR4_SIZE               4 /* ISA Feature Register 4 */
#define STIR_SIZE                4 /* Software Trigger Interrupt Register  */
#define PID4_SIZE                4 /* Peripheral Identification Register 4 */
#define PID5_SIZE                4 /* Peripheral Identification Register 5 */
#define PID6_SIZE                4 /* Peripheral Identification Register 6 */
#define PID7_SIZE                4 /* Peripheral Identification Register 7 */
#define PID0_SIZE                4 /* Peripheral Identification Register 0 */
#define PID1_SIZE                4 /* Peripheral Identification Register 1 */
#define PID2_SIZE                4 /* Peripheral Identification Register 2 */
#define PID3_SIZE                4 /* Peripheral Identification Register 3 */
#define CID0_SIZE                4 /* Component Identification Register 0  */
#define CID1_SIZE                4 /* Component Identification Register 1  */
#define CID2_SIZE                4 /* Component Identification Register 2  */
#define CID3_SIZE                4 /* Component Identification Register 3  */
#define DHCSR_SIZE          	 4 /* Debug Halting Control and Status Register */
#ifdef _DEBUG
#define GPIO_SIZE                12
#endif

int VH_Display_KKEY_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);


int CMD_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int ICTR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int STCSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int STRVR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int STCVR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int STCLVR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQSETENR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQCLRENR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQSETPER_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQCLRPER_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQABR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQPR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int CPUIDR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int ICSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int VTOR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int AIRCR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int SCR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int CCR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int SHPR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int SHCSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int CFSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int HFSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int DFSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int MMAR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int BFAR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int AFSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int Feature_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int ID_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int DHCSR_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
#ifdef _DEBUG
int GPIO_Read(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
#endif

int CMD_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int STCSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int STRVR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int STCVR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQSETENR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQCLRENR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQSETPER_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQCLRPER_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int IRQPR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int CPUIDR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int ICSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int VTOR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int AIRCR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int SCR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int CCR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int SHPR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int SHCSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int CFSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int HFSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int DFSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int MMAR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int BFAR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int AFSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int STIR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
int DHCSR_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
#ifdef _DEBUG
int GPIO_Write(simulProcessor processor, simulCallbackStruct * cbs, simulPtr dummy);
#endif

int BusRead(simulBusCallbackStruct * bus, simulWord32 * reg);
int BusWrite(simulBusCallbackStruct * bus, simulWord32 * reg);
void Interrupt(simulProcessor processor, simulPtr _private,int wakeUpEvent);
void RequestReset(simulProcessor processor);

#define ChangePending(isrnum,value)  {\
	int _changePendingValue = !!(value);\
	if (!IntCtrl->IrqPending[isrnum] && (_changePendingValue))\
	{\
		if (IntCtrl->regs.scr & REG_SCR_SEVONPEND) WakeUpFromWFE(processor,IntCtrl);\
	}\
\
	IntCtrl->IrqPending[isrnum] = !!(value);\
}

#define ChangeEnable(isrnum,value)  {\
	IntCtrl->IrqEnable[isrnum] = !!(value);\
}

#define ChangeActive(isrnum,value)  {\
	IntCtrl->IrqActive[isrnum] = !!(value);\
}

typedef struct {
	simulWord cmd;
	simulWord ictr,stcsr,strvr,stcvr,stclvr/*,irqsetenr[8],irqclrenr[8],irqsetper[8],irqclrper[8],irqabr[8],irqpr[8]*/;
	simulWord cpuidr,icsr,vtor,aircr,scr,ccr/*,shpr[3]*/,shcsr,cfsr,hfsr,dfsr,mmar,bfar,afsr,pfr0,pfr1,dfr0,afr0,mmfr0;
	simulWord mmfr1,mmfr2,mmfr3,isar0,isar1,isar2,isar3,isar4,stir,pid4,pid5,pid6,pid7,pid0,pid1,pid2,pid3,cid0,cid1,cid2,cid3,dhcsr;
	simulWord gpio;
} NVIC_Regs;

typedef struct {
	int IrqNum;
	int IrqPrio;
} NVIC_IrqStack;

typedef struct {
	int	bustype;
	simulWord baseaddress;
	NVIC_Regs regs;

	int IrqPriority[256];
	int IrqActive[256];
	int IrqEnable[256];
	int IrqPending[256];

	int CurrentIrqNum;

	int ExternalIrqPortsFirst;
	int ExternalIrqPortsCount;

	int timerrun;
	void *timer;

} IntController;

void WakeUpFromWFE(simulProcessor processor, IntController * IntCtrl);

#define REG_ICSR_NMIPENDSET 0x80000000
#define REG_ICSR_PENDSVSET	0x10000000
#define REG_ICSR_PENDSVCLR	0x08000000
#define REG_ICSR_PENDSTSET	0x04000000
#define REG_ICSR_PENDSTCLR	0x02000000

#define REG_HFSR_DEBUGEVT	0x80000000
#define REG_HFSR_FORCED		0x40000000
#define REG_HFSR_VECTTBL	0x00000002

#define REG_HFSR_MASK	(\
			REG_HFSR_DEBUGEVT |\
			REG_HFSR_FORCED |\
			REG_HFSR_VECTTBL\
		)

#define REG_SHCSR_MEMFAULTENA 0x00010000
#define REG_SHCSR_BUSFAULTENA 0x00020000
#define REG_SHCSR_USGFAULTENA 0x00040000

#define REG_SHCSR_SVCALLPENDED		0x8000
#define REG_SHCSR_BUSFAULTPENDED	0x4000
#define REG_SHCSR_MEMFAULTPENDED	0x2000
#define REG_SHCSR_USGFAULTPENDED	0x1000

#define REG_SHCSR_SYSTICKACT	0x800
#define REG_SHCSR_PENDSVACT		0x400
#define REG_SHCSR_MONITORACT	0x100
#define REG_SHCSR_SVCALLACT		0x080
#define REG_SHCSR_USGFAULTACT	0x008
#define REG_SHCSR_BUSFAULTACT	0x002
#define REG_SHCSR_MEMFAULTACT	0x001

#define REG_CFSR_MMFSR_MMARVALID	0x00000080
#define REG_CFSR_MMFSR_MLSPERR		0x00000020
#define REG_CFSR_MMFSR_MSTKERR		0x00000010
#define REG_CFSR_MMFSR_MUNSTKERR	0x00000008
#define REG_CFSR_MMFSR_DACCVIOL		0x00000002
#define REG_CFSR_MMFSR_IACCVIOL		0x00000001

#define REG_CFSR_MMFSR_MASK		(\
		REG_CFSR_MMFSR_MMARVALID |\
		REG_CFSR_MMFSR_MLSPERR |\
		REG_CFSR_MMFSR_MSTKERR |\
		REG_CFSR_MMFSR_MUNSTKERR |\
		REG_CFSR_MMFSR_DACCVIOL |\
		REG_CFSR_MMFSR_IACCVIOL\
		)

#define REG_CFSR_BFSR_BFARVALID		(0x00000080<<8)
#define REG_CFSR_BFSR_LSPERR		(0x00000020<<8)
#define REG_CFSR_BFSR_STKERR		(0x00000010<<8)
#define REG_CFSR_BFSR_UNSTKERR		(0x00000008<<8)
#define REG_CFSR_BFSR_IMPRECISERR	(0x00000004<<8)
#define REG_CFSR_BFSR_PRECISERR		(0x00000002<<8)
#define REG_CFSR_BFSR_IBUSERR		(0x00000001<<8)

#define REG_CFSR_BFSR_MASK		(\
		REG_CFSR_BFSR_BFARVALID |\
		REG_CFSR_BFSR_LSPERR |\
		REG_CFSR_BFSR_STKERR |\
		REG_CFSR_BFSR_UNSTKERR |\
		REG_CFSR_BFSR_IMPRECISERR |\
		REG_CFSR_BFSR_PRECISERR |\
		REG_CFSR_BFSR_IBUSERR \
		)

#define REG_CFSR_UFSR_DIVBYZERO		(0x00000200<<16)
#define REG_CFSR_UFSR_UNALIGNED		(0x00000100<<16)
#define REG_CFSR_UFSR_NOCP			(0x00000008<<16)
#define REG_CFSR_UFSR_INVPC			(0x00000004<<16)
#define REG_CFSR_UFSR_INVSTATE		(0x00000002<<16)
#define REG_CFSR_UFSR_UNDEFINSTR	(0x00000001<<16)

#define REG_CFSR_UFSR_MASK	(\
		REG_CFSR_UFSR_DIVBYZERO |\
		REG_CFSR_UFSR_UNALIGNED |\
		REG_CFSR_UFSR_NOCP |\
		REG_CFSR_UFSR_INVPC |\
		REG_CFSR_UFSR_INVSTATE |\
		REG_CFSR_UFSR_UNDEFINSTR \
		)

#define REG_CFSR_MASK		(\
		REG_CFSR_MMFSR_MASK |\
		REG_CFSR_BFSR_MASK |\
		REG_CFSR_UFSR_MASK \
		)

#define REG_DFSR_EXTERNAL		0x00000010
#define REG_DFSR_VCATCH			0x00000008
#define REG_DFSR_DWTTRAP		0x00000004
#define REG_DFSR_BKPT			0x00000002
#define REG_DFSR_HALTED			0x00000001

#define REG_DFSR_MASK	(\
		REG_DFSR_BKPT |\
		REG_DFSR_DWTTRAP |\
		REG_DFSR_EXTERNAL |\
		REG_DFSR_HALTED |\
		REG_DFSR_VCATCH\
		)

#define REG_STCSR_COUNTFLAG			0x00010000
#define REG_STCSR_CLKSOURCE			0x00000004
#define REG_STCSR_TICKINT			0x00000002
#define REG_STCSR_ENABLE			0x00000001

#define REG_STCLVR_NOREF		0x80000000
#define REG_STCLVR_SKEW			0x40000000
#define REG_STCLVR_TENMS		0x00FFFFFF

#define REG_AFSR_MASK			0xFFFFFFFF

#define REG_SCR_SEVONPEND		0x00000010
#define REG_SCR_SLEEPONEXIT		0x00000002

#define REG_SCR_MASK	(\
		REG_SCR_SEVONPEND |\
		REG_SCR_SLEEPONEXIT\
		)

#define REG_DHCSR_S_SLEEP		0x00040000

#define REG_CCR_STKALIGN		0x00000200
#define REG_CCR_BFHFNMIGN		0x00000100
#define REG_CCR_DIV_0_TRP		0x00000010
#define REG_CCR_UNALIGN_TRP		0x00000008
#define REG_CCR_USERSETMPEND	0x00000002
#define REG_CCR_NONBASETHRDENA	0x00000001

#define REG_CCR_MASK 0x0000031B

#define REG_AIRCR_ENDIANNESS	0x00008000
#define REG_AIRCR_PRIGROUP		0x00000700
#define REG_AIRCR_SYSRESETREQ	0x00000004
#define REG_AIRCR_VECTCLRACTIVE	0x00000002
#define REG_AIRCR_VECTRESET		0x00000001

#define REG_AIRCR_MASK (\
	REG_AIRCR_ENDIANNESS |\
	REG_AIRCR_PRIGROUP |\
	REG_AIRCR_SYSRESETREQ |\
	REG_AIRCR_VECTCLRACTIVE |\
	REG_AIRCR_VECTRESET\
)

#define IRQNUM_RESET		1
#define IRQNUM_NMI			2
#define IRQNUM_HARDFAULT	3
#define IRQNUM_MEMFAULT		4
#define IRQNUM_BUSFAULT		5
#define IRQNUM_USAGEFAULT	6
#define IRQNUM_SVCALL		11
#define IRQNUM_DEBUG		12
#define IRQNUM_PENDSV		14
#define IRQNUM_SYSTICK		15

#define CMD_IS_PRESENT					0x00
#define CMD_SET_INTERRUPT_ACTIVE		0x01
#define CMD_SET_INTERRUPT_INACTIVE		0x02
#define CMD_GET_EXCEPTION_PRIORITY		0x03
#define CMD_GET_HIGHEST_PRIORITY		0x04
#define CMD_GET_ACTIVE_EXCEPTION_COUNT	0x05
#define CMD_IS_EXCEPTION_ACTIVE			0x06
#define CMD_ESCALATE_TO_HARD_FAULT		0x07
#define CMD_CHANGE_ACTIVE_EXCEPTION		0x08
