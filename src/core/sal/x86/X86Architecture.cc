#include "X86Architecture.hpp"
#include "../Register.hpp"
#include <sstream>

namespace fail {

X86Architecture::X86Architecture()
{
	// -------------------------------------
	// Add the general purpose register:
#ifdef SIM_SUPPORT_64
	// -- 64 bit register --
	const  std::string names[] = { "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI", "R8",
		"R9", "R10", "R11", "R12", "R13", "R14", "R15" };
	for (unsigned short i = 0; i < 16; i++) {
		Register* pReg = new Register(i, 64);
		pReg->setName(names[i]);
		m_addRegister(pReg, RT_GP);
	}
#else
	// -- 32 bit register --
	const std::string names[] = { "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI" };
	for (unsigned short i = 0; i < 8; i++) {
		Register* pReg = new Register(i, 32);
		pReg->setName(names[i]);
		m_addRegister(pReg, RT_GP);
	}
#endif // SIM_SUPPORT_64
	// -------------------------------------
	// Add the program counter (PC) register:
#ifdef SIM_SUPPORT_64
	Register* pPCReg = new Register(RID_PC, 64);
	pPCReg->setName("RIP");
#else
	Register* pPCReg = new Register(RID_PC, 32);
	pPCReg->setName("EIP");
#endif // SIM_SUPPORT_64
	m_addRegister(pPCReg, RT_IP);
	// -------------------------------------
	// Add the status register (EFLAGS):
	Register* pFlagReg = new Register(RID_FLAGS, 32);
	pFlagReg->setName("EFLAGS");
	m_addRegister(pFlagReg, RT_ST);

	// Add the control registers
	Register* pCR0Reg = new Register(RID_CR0, 32);
	pCR0Reg->setName("CR0");
	m_addRegister(pCR0Reg, RT_CONTROL);

	Register* pCR2Reg = new Register(RID_CR2, 32);
	pCR2Reg->setName("CR2");
	m_addRegister(pCR2Reg, RT_CONTROL);

	Register* pCR3Reg = new Register(RID_CR3, 32);
	pCR3Reg->setName("CR3");
	m_addRegister(pCR3Reg, RT_CONTROL);

	Register* pCR4Reg = new Register(RID_CR4, 32);
	pCR4Reg->setName("CR4");
	m_addRegister(pCR4Reg, RT_CONTROL);

	// Add the segment selector registers
	Register* pCSReg = new Register(RID_CS, 16);
	pCSReg->setName("CS");
	m_addRegister(pCSReg, RT_SEGMENT);

	Register* pDSReg = new Register(RID_DS, 16);
	pDSReg->setName("DS");
	m_addRegister(pDSReg, RT_SEGMENT);

	Register* pESReg = new Register(RID_ES, 16);
	pESReg->setName("ES");
	m_addRegister(pESReg, RT_SEGMENT);

	Register* pFSReg = new Register(RID_FS, 16);
	pFSReg->setName("FS");
	m_addRegister(pFSReg, RT_SEGMENT);

	Register* pGSReg = new Register(RID_GS, 16);
	pGSReg->setName("GS");
	m_addRegister(pGSReg, RT_SEGMENT);

	Register* pSSReg = new Register(RID_SS, 16);
	pSSReg->setName("SS");
	m_addRegister(pSSReg, RT_SEGMENT);

	// FPU registers
	Register *fpureg;
	fpureg = new Register(RID_FSW, 16);
	fpureg->setName("FSW");
	m_addRegister(fpureg, RT_FPU);
	fpureg = new Register(RID_FCW, 16);
	fpureg->setName("FCW");
	m_addRegister(fpureg, RT_FPU);
	fpureg = new Register(RID_FTW, 16);
	fpureg->setName("FTW");
	m_addRegister(fpureg, RT_FPU);
	std::stringstream ss;
	for (int i = 0; i < 8; ++i) {
		fpureg = new Register(RID_FPR0_LO + i * 2, 64);
		ss.str("");
		ss << "FPR" << i << "_LO";
		fpureg->setName(ss.str());
		m_addRegister(fpureg, RT_FPU);

		fpureg = new Register(RID_FPR0_HI + i * 2, 16);
		ss.str("");
		ss << "FPR" << i << "_HI";
		fpureg->setName(ss.str());
		m_addRegister(fpureg, RT_FPU);
	}

	// XMM registers (SSE)
	fpureg = new Register(RID_MXCSR, 16); // in fact MXCSR has 32 bits, but only 0-15 are defined as of SSE3
	fpureg->setName("MXCSR");
	m_addRegister(fpureg, RT_VECTOR);
#ifdef SIM_SUPPORT_64
	for (int i = 0; i < 16; ++i) {
#else
	for (int i = 0; i < 8; ++i) {
#endif
		fpureg = new Register(RID_XMM0_LO + i * 2, 64);
		ss.str("");
		ss << "XMM" << i << "_LO";
		fpureg->setName(ss.str());
		m_addRegister(fpureg, RT_VECTOR);

		fpureg = new Register(RID_XMM0_HI + i * 2, 64);
		ss.str("");
		ss << "XMM" << i << "_HI";
		fpureg->setName(ss.str());
		m_addRegister(fpureg, RT_VECTOR);
	}

	// Registers used for extended tracing:
	size_t ids[] = {RID_CAX, RID_CBX, RID_CCX, RID_CDX, RID_CSI, RID_CDI, RID_CSP, RID_CBP, RID_FLAGS};
	for (size_t i = 0; i < sizeof(ids)/sizeof(*ids); ++i) {
		m_addRegister(getRegister(ids[i]), RT_TRACE);
	}
}

X86Architecture::~X86Architecture()
{
	for (std::vector<Register*>::iterator it = m_Registers.begin(); it != m_Registers.end(); it++)
		delete *it;
	m_Registers.clear();
}

} // end-of-namespace: fail
