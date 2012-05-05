#include "OVPCpu.hpp"
#include "OVPStatusRegister.hpp"
#include "OVPPlatform.hpp"
#include "SAL/Register.hpp"
#include "SAL/SALInst.hpp"


OVPPlatform ovpplatform;

// current CPU
OVPCpu *platform;
icmProcessorObject *processor;
icmProcessorP cpuP;

void OVPPlatform::setCpu(void *ovpcpu) {
	platform = (OVPCpu *)ovpcpu;
	processor = platform->getProcessor();
	cpuP = processor->getProcessorP();
}

void OVPPlatform::setRegisterData(void * link, unsigned int val) {
	icmWriteRegInfoValue(cpuP, (icmRegInfoP)link, (void *) &val);	
}

unsigned int OVPPlatform::getRegisterData(void *link) {
	unsigned int res;

	icmReadRegInfoValue(cpuP, (icmRegInfoP)link, (void *)&res);
	return res;
}

uint32_t OVPPlatform::getPC() {
	return (uint32_t) processor->getPC();
}

void OVPPlatform::setPC(uint32_t val) {
	processor->setPC(val);
}


uint32_t OVPPlatform::getSP() {
	uint32_t res;
	void *buf = &res;

	icmReadRegInfoValue(cpuP, platform->getSPReg(), buf);

	return res;

}

void OVPPlatform::save(const string& path) {
	platform->save(path);
}

void OVPPlatform::restore(const string& path) {
	platform->restore(path);
}
