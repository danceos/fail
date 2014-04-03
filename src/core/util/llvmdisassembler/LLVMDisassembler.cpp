#include "LLVMDisassembler.hpp"

using namespace fail;
using namespace llvm;
using namespace llvm::object;


LLVMtoFailTranslator & LLVMDisassembler::getTranslator() {
	if ( ltofail == 0 ) {
		std::cout << "ArchType: " << llvm::Triple::getArchTypeName(	 llvm::Triple::ArchType(object->getArch())	) << std::endl;

		switch ( llvm::Triple::ArchType(object->getArch()) ) {
		case llvm::Triple::x86:
		case llvm::Triple::x86_64:
			ltofail = new LLVMtoFailBochs(this);
			break;
		case llvm::Triple::arm:
			ltofail = new LLVMtoFailGem5(this);
			break;
		default:
			std::cout << " not supported :(";
			exit(1);
		}
	}
	return *ltofail;
}

void LLVMDisassembler::disassemble()
{
	error_code ec;
	for (section_iterator i = object->begin_sections(),
			 e = object->end_sections();
		 i != e; i.increment(ec)) {
		if (error(ec)) break;
		bool text;
		if (error(i->isText(text))) break;
		if (!text) continue;

		uint64_t SectionAddr;
		if (error(i->getAddress(SectionAddr))) break;

		uint64_t SectionLength;
		if (error(i->getSize(SectionLength))) break;

		// Make a list of all the symbols in this section.
		std::vector<std::pair<uint64_t, StringRef> > Symbols;
		for (symbol_iterator si = object->begin_symbols(),
				 se = object->end_symbols();
			 si != se; si.increment(ec)) {
			bool contains;
			StringRef Name;

			if (!error(i->containsSymbol(*si, contains)) && contains) {
				uint64_t Address;
				if (error(si->getAddress(Address))) break;
				Address -= SectionAddr;

				if (error(si->getName(Name))) break;
				Symbols.push_back(std::make_pair(Address, Name));
			}
		}

		// Sort the symbols by address, just in case they didn't come in that way.
		array_pod_sort(Symbols.begin(), Symbols.end());

		StringRef name;
		if (error(i->getName(name))) break;

		// If the section has no symbols just insert a dummy one and disassemble
		// the whole section.
		if (Symbols.empty())
			Symbols.push_back(std::make_pair(0, name));

		StringRef Bytes;
		if (error(i->getContents(Bytes))) break;
		StringRefMemoryObject memoryObject(Bytes);
		uint64_t Size;
		uint64_t Index;
		uint64_t SectSize;
		if (error(i->getSize(SectSize))) break;

		// Disassemble symbol by symbol.
		for (unsigned si = 0, se = Symbols.size(); si != se; ++si) {
			uint64_t Start = Symbols[si].first;
			uint64_t End;
			// The end is either the size of the section or the beginning of the next
			// symbol.
			if (Start >= SectSize)
				// we are beyond the end of the section
				break;
			else if (si == se - 1)
				End = SectSize;
			// Make sure this symbol takes up space.
			else if (Symbols[si + 1].first != Start)
				End = Symbols[si + 1].first - 1;
			else
				// This symbol has the same address as the next symbol. Skip it.
				continue;

			for (Index = Start; Index <= End; Index += Size) {
				MCInst Inst;

				if (disas->getInstruction(Inst, Size, memoryObject, Index,
										  nulls(), nulls()) == MCDisassembler::Success) {
					const MCInstrDesc &desc = this->instr_info->get(Inst.getOpcode());
					//			Inst.dump();
					Instr instr_info;
					instr_info.opcode = Inst.getOpcode();
					instr_info.length = Size;
					instr_info.address = SectionAddr + Index;
					instr_info.conditional_branch = desc.isConditionalBranch();

					assert( Size > 0 && "zero size instruction disassembled" );

					unsigned int  pos = 0;
					for (MCInst::iterator it = Inst.begin(); it != Inst.end(); ++it) {

						if (it->isValid() && it->isReg()
							&& it->getReg() != 0 /* NOREG */) {
							// Distinguish between input and
							// output register operands
							if (pos < desc.getNumDefs())
								instr_info.reg_defs.push_back(it->getReg());
							else
								instr_info.reg_uses.push_back(it->getReg());
						}
						pos ++;
					}
					const uint16_t *ptr = desc.getImplicitUses();
					for (unsigned int i = 0; i < desc.getNumImplicitUses(); i++) {
						if (ptr[i] == 0) // NOREG
							continue;
						instr_info.reg_uses.push_back(ptr[i]);
					}
					ptr = desc.getImplicitDefs();
					for (unsigned int i = 0; i < desc.getNumImplicitDefs(); i++) {
						if (ptr[i] == 0) // NOREG
							continue;
						instr_info.reg_defs.push_back(ptr[i]);
					}

					// Insert the instruction info into the instr map
					(*instrs)[instr_info.address] = instr_info;
				} else {
					if (Size == 0)
						Size = 1; // skip illegible bytes
				}
			}
		}
	}

}

void LLVMDisassembler::StringRefMemoryObject::anchor() {}
