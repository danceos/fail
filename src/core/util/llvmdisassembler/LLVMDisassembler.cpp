#include "LLVMDisassembler.hpp"
#include "util/Logger.hpp"

using namespace fail;
using namespace llvm;
using namespace llvm::object;

static Logger LOG("LLVMDisassembler");

LLVMtoFailTranslator *LLVMDisassembler::getTranslator() {
	if (!ltofail) {
		switch ( llvm::Triple::ArchType(m_object->getArch()) ) {
		case llvm::Triple::x86:
		case llvm::Triple::x86_64:
			ltofail.reset(new LLVMtoFailBochs(this));
			break;
		case llvm::Triple::arm:
			ltofail.reset(new LLVMtoFailGem5(this));
			break;
		default:
			LOG << "ArchType "
				<< llvm::Triple::getArchTypeName(llvm::Triple::ArchType(m_object->getArch())).str()
				<< " not supported\n";
			exit(1);
		}
	}
	return ltofail.get();
}

LLVMDisassembler::LLVMDisassembler(ElfReader *elf) {
	/* Disassemble the binary if necessary */
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllDisassemblers();

	std::string filename = elf->getFilename();

	Expected<OwningBinary<Binary>> BinaryOrErr = createBinary(filename);
	if (!BinaryOrErr) {
		std::string Buf;
		raw_string_ostream OS(Buf);
		logAllUnhandledErrors(std::move(BinaryOrErr.takeError()), OS, "");
		OS.flush();
		LOG << "Could not read ELF file:" << filename << "': " << Buf << ".\n";
		exit(1);
	}

	m_binary = std::move(BinaryOrErr.get());
	m_object = llvm::dyn_cast<ObjectFile>(m_binary.getBinary());


	this->triple = GetTriple(m_object);
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


void LLVMDisassembler::disassemble()
{
	std::error_code ec;
	for (section_iterator i = m_object->section_begin(),
			 e = m_object->section_end(); i != e; ++i) {
		bool text = i->isText();
		if (!text) continue;

		uint64_t SectionAddr = i->getAddress();
		// FIXME uint64_t SectionLength = i->getSize();

		// Make a list of all the symbols in this section.
		std::vector<std::pair<uint64_t, StringRef> > Symbols;
		for (const SymbolRef &symbol : m_object->symbols()) {
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

	LOG << "instructions disassembled: " << std::dec << instrs->size() << std::endl;
}
