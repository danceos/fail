#ifndef __LLVMDISASSEMBLER_HPP__
#define __LLVMDISASSEMBLER_HPP__

#include <iostream>
#include <vector>
#include <map>
#include <limits.h>

#include "llvm/Object/ObjectFile.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/MemoryObject.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ADT/StringExtras.h"

#include "llvm/Support/Casting.h"

#include "llvm/ADT/OwningPtr.h"

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
	llvm::OwningPtr<llvm::MCSubtargetInfo> subtargetinfo;
	llvm::OwningPtr<const llvm::MCDisassembler> disas;
	llvm::OwningPtr<const llvm::MCInstrInfo> instr_info;
	llvm::OwningPtr<const llvm::MCRegisterInfo> register_info;
	llvm::OwningPtr<InstrMap> instrs;

	fail::LLVMtoFailTranslator * ltofail;


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

	static bool error(llvm::error_code ec) {
		if (!ec) return false;

		std::cerr << "DIS" << ": error reading file: " << ec.message() << ".\n";
		return true;
	}

	class StringRefMemoryObject : public llvm::MemoryObject {
		virtual void anchor();
		llvm::StringRef Bytes;
	public:
		StringRefMemoryObject(llvm::StringRef bytes) : Bytes(bytes) {}

		uint64_t getBase() const { return 0; }
		uint64_t getExtent() const { return Bytes.size(); }

		int readByte(uint64_t Addr, uint8_t *Byte) const {
			if (Addr >= getExtent())
				return -1;
			*Byte = Bytes[Addr];
			return 0;
		}
	};



public:
	LLVMDisassembler(const llvm::object::ObjectFile *object) : ltofail(0)  {
		this->object = object;
		this->triple = GetTriple(object);
		this->target = GetTarget(triple);
		this->subtargetinfo.reset(target->createMCSubtargetInfo(triple, MCPU, FeaturesStr));
		this->disas.reset(target->createMCDisassembler(*subtargetinfo));
		this->instr_info.reset(target->createMCInstrInfo());
		this->register_info.reset(target->createMCRegInfo(triple));

		this->instrs.reset(new InstrMap());
	}

	~LLVMDisassembler() { delete ltofail; };

	InstrMap &getInstrMap() { return *instrs; };
	const llvm::MCRegisterInfo &getRegisterInfo() { return *register_info;}
	fail::LLVMtoFailTranslator & getTranslator() ;

	const std::string & GetTriple() const { return triple; };

	void disassemble();
};


}
#endif // __LLVMDISASSEMBLER_HPP__
