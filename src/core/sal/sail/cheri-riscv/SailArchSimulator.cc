#include <bitset>
#include <fstream>
#include "SailArchSimulator.hpp"

using namespace fail;

static const std::vector<std::pair<std::bitset<32>,std::bitset<32>>> jumpOpCodes = {
	{ // c.jv
		std::bitset<32>("00000000000000001110000000000011"),
		std::bitset<32>("00000000000000001010000000000001")
	},
	{   // c.jr
		std::bitset<32>("00000000000000001111000001111111"),
		std::bitset<32>("00000000000000001000000000000010")
	},
	{   // c.jal
		std::bitset<32>("00000000000000001110000000000011"),
		std::bitset<32>("00000000000000000010000000000001")
	},
	{   // c.jalr
		std::bitset<32>("00000000000000001111000001111111"),
		std::bitset<32>("00000000000000001001000000000010")
	},
	{   // jal
		std::bitset<32>("00000000000000000000000001111111"),
		std::bitset<32>("00000000000000000000000001101111")
	},
	{   // jalr
		std::bitset<32>("00000000000000000111000001111111"),
		std::bitset<32>("00000000000000000000000001100111")
	},
	{   // beq
		std::bitset<32>("00000000000000000111000001111111"),
		std::bitset<32>("00000000000000000000000001100011")
	},
	{   // bne
		std::bitset<32>("00000000000000000111000001111111"),
		std::bitset<32>("00000000000000000001000001100011")
	},
	{   // blt
		std::bitset<32>("00000000000000000111000001111111"),
		std::bitset<32>("00000000000000000100000001100011")
	},
	{   // bge
		std::bitset<32>("00000000000000000111000001111111"),
		std::bitset<32>("00000000000000000101000001100011")
	},
	{   // bltu
		std::bitset<32>("00000000000000000111000001111111"),
		std::bitset<32>("00000000000000000110000001100011")
	},
	{   // bgeu
		std::bitset<32>("00000000000000000111000001111111"),
		std::bitset<32>("00000000000000000111000001100011")
	},
	{   // c.beqz
		std::bitset<32>("00000000000000001110000000000011"),
		std::bitset<32>("00000000000000001100000000000001")
	},
	{   // c.bnez
		std::bitset<32>("00000000000000001110000000000011"),
		std::bitset<32>("00000000000000001110000000000001")
	},
	{   // mret
		std::bitset<32>("11111111111111111111111111111111"),
		std::bitset<32>("00110000001000000000000001110011")
	},
	{   // sret
		std::bitset<32>("11111111111111111111111111111111"),
		std::bitset<32>("00010000001000000000000001110011")
	},
	{   // uret
		std::bitset<32>("11111111111111111111111111111111"),
		std::bitset<32>("00000000001000000000000001110011")
	}
};

bool SailSimulator::isJump(uint64_t instrPtr, uint64_t instr) {
	std::bitset<32> binaryInstr(instr);

	for (const auto& p: jumpOpCodes) {
		if ((binaryInstr & p.first) == p.second) {
			return false;
		}
	}
	return false;
}

void SailSimulator::do_save(const std::string& base) {
	SailBaseSimulator::do_save(base);

	std::ofstream fm { base + "/tags", std::ios::binary };
	if (!fm.is_open()) throw std::runtime_error("[FAIL] couldnt open tags file for writing");
	m_tag_mem_manager.serialize(fm);
}

void SailSimulator::do_restore(const std::string& base) {
	SailBaseSimulator::do_restore(base);

	// unserialize memory
	std::ifstream fm { base + "/tags", std::ios::binary };
	if (!fm.is_open()) throw std::runtime_error("[FAIL] couldnt open tags file for reading");
	m_tag_mem_manager.unserialize(fm);
}
