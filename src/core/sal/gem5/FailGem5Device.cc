#include "FailGem5Device.hpp"

#include "debug/Fail.hh"
#include "sim/system.hh"
#include "arch/arm/decoder.hh"
#include "cpu/simple/base.hh"

#include "../SALInst.hpp"
#include "Gem5PCEvents.hpp"

#include <bitset>

FailGem5Device::FailGem5Device(Params *p)
	: BasicPioDevice(p)
{
	pioSize = 0x60;
}

void FailGem5Device::startup()
{
	// TODO: Find a better way/position to start single stepping.
	// It's not possible to fetch the instruction at Adress 0x0
	m_BreakpointNotTaken = new Gem5InstructionEvent(this, &sys->pcEventQueue, 0x80000000);
	fail::simulator.startup();
}

void FailGem5Device::activateSingleStepMode()
{
	setNextBreakpoints(sys->getThreadContext(0));
}

void FailGem5Device::deactivateSingleStepMode()
{
	clearBreakpoints();
}

void FailGem5Device::setNextBreakpoints(ThreadContext *tc)
{
	clearBreakpoints();

	// Get the instruction at the current program counter
	MachInst inst;
	PCState pc = tc->pcState();
	tc->getVirtProxy().readBlob(pc.pc(), (uint8_t*)&inst, sizeof(MachInst));

	// Decode the instruction
	ArmISA::Decoder* decoder = tc->getDecoderPtr();
	BaseSimpleCPU* cpu = dynamic_cast<BaseSimpleCPU*>(tc->getCpuPtr());
	Addr fetchPC = (pc.pc() & cpu->PCMask) + cpu->fetchOffset;
	decoder->moreBytes(pc, fetchPC, inst);
	StaticInstPtr si = decoder->decode(pc);

	// Set breakpoints
	// First breakpoint is always the next pc
	// FIXME: Doesn't work properly in Thumb or Thumb2 mode
	m_BreakpointNotTaken = new Gem5InstructionEvent(this, &sys->pcEventQueue, pc.npc());

	// Second breakpoint will only be set if there is a possible change in control flow
	DPRINTF(Fail, "Instruction Name: %s\n", si->getName());

	if(si->getName().compare("bx") == 0)
	{
		DPRINTF(Fail, "BX instruction\n");
		Addr target = tc->readIntReg(si->machInst.rm);
		m_BreakpointTaken = new Gem5InstructionEvent(this, &sys->pcEventQueue, target);
		
		return;
	}

	// Check for a pop instruction with the pc as target
	if(si->getName().compare("ldmstm") == 0)
	{
		std::bitset<16> regs(si->machInst.regList);
		if(regs.test(15)) // Bit 15 = pc
		{
			DPRINTF(Fail, "LDMSTM with pc as target\n");
			Addr mem = tc->readIntReg(si->machInst.rn) + 4*(regs.count()-1);
			
			Addr target;
			tc->getVirtProxy().readBlob(mem, (uint8_t*)&target, sizeof(Addr));
			DPRINTF(Fail, "Value loaded into pc: %x\n", target);
			
			m_BreakpointTaken = new Gem5InstructionEvent(this, &sys->pcEventQueue, target);
		}
		return;
	}

	PCState bpc;
	if(si->hasBranchTarget(pc, tc, bpc))
	{
		DPRINTF(Fail, "Immediate branch\n");
	    if (bpc.pc() != pc.npc())
		{
	        m_BreakpointTaken = new Gem5InstructionEvent(this, &sys->pcEventQueue, bpc.pc());
		}
	}
}

void FailGem5Device::clearBreakpoints()
{
	if(m_BreakpointTaken)
	{
		delete m_BreakpointTaken;
		m_BreakpointTaken = 0;
	}
	if(m_BreakpointNotTaken)
	{
		delete m_BreakpointNotTaken;
		m_BreakpointNotTaken = 0;
	}
}


Tick FailGem5Device::read(PacketPtr pkt)
{
	return pioDelay;
}

Tick FailGem5Device::write(PacketPtr pkt)
{
	return pioDelay;
}

FailGem5Device* FailGem5DeviceParams::create()
{
	return new FailGem5Device(this);
}
