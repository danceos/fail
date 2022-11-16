#ifndef __LLVMDISASSEMBLER_HPP__
#define __LLVMDISASSEMBLER_HPP__

#include <iostream>
#include <vector>
#include <map>
#include <limits.h>
#include <memory> // unique_ptr

#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"


#include "LLVMtoFailTranslator.hpp"

namespace fail {

class ElfReader;

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
	llvm::object::OwningBinary<llvm::object::Binary> m_binary;
	llvm::object::ObjectFile *m_object;

	const llvm::Target *target;
	std::string triple;
	std::string MCPU;
	std::string FeaturesStr;
	std::unique_ptr<const llvm::MCSubtargetInfo> subtargetinfo;
	std::unique_ptr<const llvm::MCDisassembler> disas;
	std::unique_ptr<const llvm::MCInstrInfo> instr_info;
	std::unique_ptr<const llvm::MCRegisterInfo> register_info;
	std::unique_ptr<InstrMap> instrs;

	std::unique_ptr<fail::LLVMtoFailTranslator> ltofail;


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

	unsigned m_xlen;

public:
	LLVMDisassembler(fail::ElfReader *elf);

	InstrMap *getInstrMap() { return instrs.get(); };
	fail::LLVMtoFailTranslator *getTranslator() ;

	const std::string& GetTriple() const { return triple; };

	const llvm::MCRegisterInfo& getRegisterInfo() { return *register_info; }

	std::string getRegisterName(unsigned id);

	const std::string GetSubtargetFeatures() const;

	void disassemble();

	unsigned getWordWidth() { return m_xlen; }
};

}
#endif // __LLVMDISASSEMBLER_HPP__
