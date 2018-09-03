#include "LLVMDisassembler.hpp"

using namespace fail;
using namespace llvm;
using namespace llvm::object;

// In LLVM 3.9, llvm::Triple::getArchTypeName() returns const char*, since LLVM
// 4.0 it returns StringRef.  This overload catches the latter case.
__attribute__((unused))
static std::ostream& operator<<(std::ostream& stream, const llvm::StringRef& s)
{
	stream << s.str();
	return stream;
}

LLVMtoFailTranslator *LLVMDisassembler::getTranslator() {
	if (ltofail == 0) {
		switch ( llvm::Triple::ArchType(object->getArch()) ) {
		case llvm::Triple::x86:
		case llvm::Triple::x86_64:
			ltofail = new LLVMtoFailBochs(this);
			break;
		case llvm::Triple::arm:
			ltofail = new LLVMtoFailGem5(this);
			break;
		default:
			std::cerr << "ArchType "
				<< llvm::Triple::getArchTypeName(llvm::Triple::ArchType(object->getArch()))
				<< " not supported\n";
			exit(1);
		}
	}
	return ltofail;
}

void LLVMDisassembler::disassemble()
{
	std::error_code ec;
	for (section_iterator i = object->section_begin(),
			 e = object->section_end(); i != e; ++i) {
		bool text = i->isText();
		if (!text) continue;

		uint64_t SectionAddr = i->getAddress();
		// FIXME uint64_t SectionLength = i->getSize();

		// Make a list of all the symbols in this section.
		std::vector<std::pair<uint64_t, StringRef> > Symbols;
		for (const SymbolRef &symbol : object->symbols()) {
			StringRef Name;

			if (!i->containsSymbol(symbol)) {
				continue;
			}

			uint64_t Address;
			Expected<uint64_t> AddrOrErr = symbol.getAddress();
			if (!AddrOrErr) {
				// FIXME fail
				continue;
			}
			Address = *AddrOrErr - SectionAddr;

			Expected<StringRef> NameOrErr = symbol.getName();
			if (!NameOrErr) {
				// FIXME fail
				continue;
			}

			Symbols.push_back(std::make_pair(Address, *NameOrErr));
		}

		// Sort the symbols by address, just in case they didn't come in that way.
		array_pod_sort(Symbols.begin(), Symbols.end());

		StringRef name;
		if (error(i->getName(name))) break;

		// If the section has no symbols just insert a dummy one and disassemble
		// the whole section.
		if (Symbols.empty())
			Symbols.push_back(std::make_pair(0, name));

		StringRef BytesStr;
		if (error(i->getContents(BytesStr))) break;
		ArrayRef<uint8_t> Bytes(reinterpret_cast<const uint8_t *>(BytesStr.data()),
			BytesStr.size());

		uint64_t Size;
		uint64_t Index;
		uint64_t SectSize = i->getSize();

		// Disassemble symbol by symbol.
		for (unsigned si = 0, se = Symbols.size(); si != se; ++si) {
			uint64_t Start = Symbols[si].first;
			uint64_t End; // exclusive
			// The end is either the size of the section or the beginning of the next
			// symbol.
			if (Start >= SectSize)
				// we are beyond the end of the section
				break;
			else if (si == se - 1)
				End = SectSize;
			// Make sure this symbol takes up space.
			else if (Symbols[si + 1].first != Start)
				End = Symbols[si + 1].first;
			else
				// This symbol has the same address as the next symbol. Skip it.
				continue;

			for (Index = Start; Index < End; Index += Size) {
				MCInst Inst;

				if (disas->getInstruction(Inst, Size, Bytes.slice(Index), Index,
										  nulls(), nulls()) == MCDisassembler::Success) {
					const MCInstrDesc &desc = this->instr_info->get(Inst.getOpcode());

					Instr instr;
					instr.opcode = Inst.getOpcode();
					instr.length = Size;
					instr.address = SectionAddr + Index;
					instr.conditional_branch = desc.isConditionalBranch();

					assert( Size > 0 && "zero size instruction disassembled" );

					unsigned int pos = 0;
					for (MCInst::iterator it = Inst.begin(); it != Inst.end(); ++it) {

						if (it->isValid() && it->isReg()
							&& it->getReg() != 0 /* NOREG */) {
							// Distinguish between input and
							// output register operands
							if (pos < desc.getNumDefs())
								instr.reg_defs.push_back(it->getReg());
							else
								instr.reg_uses.push_back(it->getReg());
						}
						pos ++;
					}
					const uint16_t *ptr = desc.getImplicitUses();
					for (unsigned int i = 0; i < desc.getNumImplicitUses(); i++) {
						if (ptr[i] == 0) // NOREG
							continue;
						instr.reg_uses.push_back(ptr[i]);
					}
					ptr = desc.getImplicitDefs();
					for (unsigned i = 0; i < desc.getNumImplicitDefs(); i++) {
						if (ptr[i] == 0) // NOREG
							continue;
						instr.reg_defs.push_back(ptr[i]);
					}

					// Insert the instruction info into the instr map
					(*instrs)[instr.address] = instr;
				} else {
					if (Size == 0)
						Size = 1; // skip illegible bytes
				}
			}
		}
	}
}
