#ifndef _OVPCpu_HPP_
#define _OVPCpu_HPP_

#include "icm/icmCpuManager.hpp"
#include "SAL/Register.hpp"
#include "OVPPlatform.hpp"

extern OVPPlatform ovpplatform;


using namespace icmCpuManager;

/**
 * \class OVPCpu
 * Abstract class which contains methods OVP platforms must implement.
 * As there is the possibility to create different platforms this class
 * is an interface for the OVPPlatform.
 */
class OVPCpu {
	protected:
		icmPlatform *platform;
		icmProcessorObject *cpu;
		icmAttrListObject *attrList;
		icmProcessorP processorP;

		// for MMC
		icmMmcObject *mmcInstr;
		icmMmcObject *mmcData;
		icmBusObject *instrBus;
		icmBusObject *dataBus;
		icmBusObject *mainBus;
		icmMemoryObject *icmMem0;
		icmMemoryObject *icmMem1;
		icmMemoryObject *icmMem2;
	
		icmRegInfoP rid_sp;

	public:
		unsigned char *mem;
		Int16 *textmem;
		size_t offset;
		size_t memSize;
		size_t textOffset;
		size_t textMemSize;

		/**
		 * Initialize platform.
		 * @param gdb set if GDB functionality should be enabled
		 */
		virtual void init(bool) = 0;

		/**
		 * Simulate the platform
		 * @param app Name/Path to the application to run 
		 */
		virtual int startSimulation(const char*) = 0;

		/** 
		 * Create a full MMC to have memory control.
		 * @param vlnRoot Path to OVP model. Default: 0 -> default OVP path will be taken
		 */
		virtual void createFullMMC(const char *) = 0;

		/**
		 * Set general purpose register in SAL 
		 */
		virtual void makeGPRegister() = 0;

		/**
		 * Set status register in SAL
		 */
		virtual void makeSTRegister() = 0;

		/**
		 * Set program counter register in SAL
		 */
		virtual void makePCRegister() = 0;

		/**
		 * Create local mirrored memory to handle memory manipulations
		 * during the memory callback
		 * @param sizeText Size of the text segment
		 * @param offText Address the text segment starts
		 * @param sizeMem Size of the other segments
		 * @param offMem Address the other segments start
		 */
		virtual void makeCallbackMemory(size_t sizeText, size_t offText, size_t sizeMem, size_t offMem) = 0;

		/**
		 * Returns the private icmProcessorObject pointer needed for some OVP action
		 * @return icmProcessorObject
		 */
		icmProcessorObject *getProcessor() {
			return cpu;
		}

		/**
		 * Returns the private ProcessorP struct needed for some OVP action
		 * @return ProcessorP
		 */
		icmProcessorP getProcessorP() {
			return processorP;
		}

		/**
		 * Set the pointer to the stack pointer register
		 * @param reg icmRegInfoP of the stack pointer register
		 */
		void setSPReg(icmRegInfoP reg) {
			rid_sp = reg;
		}
		
		/**
		 * Returns the private stack pointer register pointer
		 * @return stack pointer register
		 */
		icmRegInfoP getSPReg() {
			return rid_sp;
		}
		
		/**
		 * Fills the callback memory with program data. This function
		 * must be called after the application is loaded to local memory
		 * and before the start of the simulation.
		 */
		void fillCallbackMemory() {
			mainBus->read(offset, mem, memSize, cpu);
			mainBus->read(textOffset, textmem, textMemSize, cpu);
		}
};

#endif
