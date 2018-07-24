#ifndef __LLVMDISASSEMBLER_HPP__
#define __LLVMDISASSEMBLER_HPP__

#include <iostream>
#include <vector>
#include <map>
#include <limits.h>
#include <memory> // unique_ptr

#include "llvm/Object/ObjectFile.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ADT/StringExtras.h"

#include "llvm/Support/Casting.h"

#include "LLVMtoFailTranslator.hpp"
#include "LLVMtoFailBochs.hpp"
#include "LLVMtoFailGem5.hpp"

namespace fail {

class LLVMDisassembler {

public:
	typedef uint16_t register_t;
	typedef unsigned int address_t;
	struct Instr {
		unsigned int opcode;
		unsigned int address;
		unsigned char length;
		bool conditional_branch;
		std::vector<register_t> reg_uses;
		std::vector<register_t> reg_defs;
	};

	typedef std::map<address_t, Instr> InstrMap;

private:
	const llvm::object::ObjectFile *object;
	const llvm::Target *target;
	std::string triple;
	std::string MCPU;
	std::string FeaturesStr;
	std::unique_ptr<const llvm::MCSubtargetInfo> subtargetinfo;
	std::unique_ptr<const llvm::MCDisassembler> disas;
	std::unique_ptr<const llvm::MCInstrInfo> instr_info;
	std::unique_ptr<const llvm::MCRegisterInfo> register_info;
	std::unique_ptr<InstrMap> instrs;

	fail::LLVMtoFailTranslator *ltofail;


	static std::string GetTriple(const llvm::object::ObjectFile *Obj) {
		llvm::Triple TT("unknown-unknown-unknown");
		TT.setArch(llvm::Triple::ArchType(Obj->getArch()));
		std::string TripleName = TT.str();
		return TripleName;
	}


	static const llvm::Target *GetTarget(const std::string &TripleName) {
		// Get the target specific parser.
		std::string Error;
		const llvm::Target *TheTarget = llvm::TargetRegistry::lookupTarget(TripleName, Error);
		if (TheTarget)
			return TheTarget;

		std::cerr << "error: unable to get target for '" << TripleName
				  << std::endl;
		return 0;
	}

	static bool error(std::error_code ec) {
		if (!ec) return false;

		std::cerr << "DIS error: " << ec.message() << ".\n";
		return true;
	}

public:
	LLVMDisassembler(const llvm::object::ObjectFile *object) : ltofail(0)  {
		this->object = object;
		this->triple = GetTriple(object);
		this->target = GetTarget(triple);

		std::unique_ptr<const llvm::MCRegisterInfo> MRI(target->createMCRegInfo(triple));
		if (!MRI) {
			std::cerr << "DIS error: no register info for target " << triple << "\n";
			return;
		}                                             

		std::unique_ptr<const llvm::MCAsmInfo> MAI(target->createMCAsmInfo(*MRI, triple));
		if (!MAI) {
			std::cerr << "DIS error: no assembly info for target " << triple << "\n";
			return;
		}

		std::unique_ptr<const llvm::MCSubtargetInfo> STI(
				target->createMCSubtargetInfo(triple, MCPU, FeaturesStr));
		if (!STI) {
			std::cerr << "DIS error: no subtarget info for target " << triple << "\n";
			return;
		}
		std::unique_ptr<const llvm::MCInstrInfo> MII(target->createMCInstrInfo());
		if (!MII) {
			std::cerr << "DIS error: no instruction info for target " << triple << "\n";
			return;
		}
		std::unique_ptr<const llvm::MCObjectFileInfo> MOFI(new llvm::MCObjectFileInfo);
		// Set up the MCContext for creating symbols and MCExpr's.
		llvm::MCContext Ctx(MAI.get(), MRI.get(), MOFI.get());

		this->subtargetinfo = std::move(STI);
		std::unique_ptr<llvm::MCDisassembler> DisAsm(
				target->createMCDisassembler(*subtargetinfo, Ctx));
		if (!DisAsm) {
			std::cerr << "DIS error: no disassembler for target " << triple << "\n";
			return;
		}
		this->disas = std::move(DisAsm);
		this->instr_info = std::move(MII);
		this->register_info = std::move(MRI);

		this->instrs.reset(new InstrMap());
	}

	~LLVMDisassembler() { delete ltofail; };

	InstrMap &getInstrMap() { return *instrs; };
	const llvm::MCRegisterInfo& getRegisterInfo() { return *register_info; }
	fail::LLVMtoFailTranslator *getTranslator() ;

	const std::string& GetTriple() const { return triple; };

	void disassemble();
};

}
#endif // __LLVMDISASSEMBLER_HPP__
