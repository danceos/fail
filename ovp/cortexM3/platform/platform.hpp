#ifndef _armCortexM3_HPP_
#define _armCortexM3_HPP_


#include "../../OVPCpu.hpp"

using namespace icmCpuManager;

/**
 * \class ARM_Cortex_M3
 * Implements the OVP platform for ARM Cortex M3 with callback memory 
 */
class ARM_Cortex_M3 : public OVPCpu {
	private:
	
	public:
		ARM_Cortex_M3();	
		~ARM_Cortex_M3();	
		void init(bool);
		int startSimulation(const char*);
		void createFullMMC(const char *);
		void mapMemToCallback();

		void makeGPRegister();
		void makeSTRegister();
		void makePCRegister();

		void makeCallbackMemory(size_t, size_t, size_t, size_t);
};


#endif
