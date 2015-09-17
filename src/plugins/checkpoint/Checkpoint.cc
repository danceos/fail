#include "Checkpoint.hpp"
#include "sal/Listener.hpp"
#include "sal/Memory.hpp"
#include "sha1.h"
#include <cstring>
#include "sal/bochs/BochsCPU.hpp"
#include "sal/SALConfig.hpp"


using namespace std;
using namespace fail;

bool Checkpoint::run()
{
	assert(!m_checking && "FATAL: Checkpoint plugin must not be added to simulation in checking mode");

	// log information
	m_log << "Checkpoint Logger started." << std::endl;
	m_log << "Triggering on: " << m_symbol << std::endl;
	m_log << "Writing output to: " << m_file << std::endl;

	/*
	std::vector<address_range>::const_iterator it = m_check_ranges.begin();
	for( ; it != m_check_ranges.end(); ++it) {
		m_log << "Checksumming: " <<
			  << ((it->first.second) ? "*" : "")
			  << "0x" << std::hex << it->first.first
			  << " - " <<
			  << (it->second.second) ? "*" : ""
			  << "0x" << std::hex << it->second.first
			  << std::endl;
	}
	*/

	if (!m_ostream.is_open()) {
		m_log << "No output file."  << std::endl;
		return false;
	}

	// listen for memory writes and save checkpoints
	MemWriteListener ev_mem(m_symbol.getAddress());
	while (true) {
		simulator.addListenerAndResume(&ev_mem);

		save_checkpoint(ev_mem.getTriggerInstructionPointer());
	}

	return true;
}

address_t Checkpoint::resolve_address(const indirectable_address_t &addr) {
	if(addr.second) {
		const address_t paddr = addr.first;
		address_t value = 0;

		std::map<fail::address_t, fail::address_t>::iterator it = cached_stackpointer_variables.find(paddr);
		if (it != cached_stackpointer_variables.end()) {
			m_log << "Use Cached Value! " << std::hex << it->first << "=" << it->second << std::dec << endl;
			value = it->second;
		} else {
			MemoryManager& mm = simulator.getMemoryManager();

			if(mm.isMapped(paddr) && mm.isMapped(paddr+1) && mm.isMapped(paddr+2) && mm.isMapped(paddr+3)) {
				simulator.getMemoryManager().getBytes(paddr, 4, &value);
			}
		}

		// HACK/WORKAROUND for dOSEK, which uses bit 31 for parity!
		// This fixes checkpoint ranges for dOSEK, but breaks other systems *if*
		// addresses with bit 31 set are used as the limit for checkpoint regions
		return value & ~(1<<31);
	} else {
		return addr.first;
	}
}

bool Checkpoint::in_range(fail::address_t addr)
{
	// iterate memory regions
	std::vector<address_range>::const_iterator it = m_check_ranges.begin();
	for( ; it != m_check_ranges.end(); ++it) {
		fail::address_t start = resolve_address(it->first);
		fail::address_t end = resolve_address(it->second);
		if((start == 0) || (end == 0)) {
			m_log << std::hex << "invalid checksum range pointer" << std::endl;
			continue;
		}
		if (start <= addr && addr < end) {
			return true;
		}
	}
	return false;
}

void Checkpoint::checksum(uint8_t (&Message_Digest)[20])
{
	SHA1Context sha;
	int err;

	// prepare SHA1 hash
	err = SHA1Reset(&sha);
	assert(err == 0);

	MemoryManager& mm = simulator.getMemoryManager();

	// disable paging on x86
	#ifdef BUILD_X86
	const Register *reg_cr0 = simulator.getCPU(0).getRegister(RID_CR0);
	uint32_t cr0 = simulator.getCPU(0).getRegisterContent(reg_cr0);
	simulator.getCPU(0).setRegisterContent(reg_cr0, cr0 & ~(1<<31));
	#endif

	// iterate memory regions
	std::vector<address_range>::const_iterator it = m_check_ranges.begin();
	for( ; it != m_check_ranges.end(); ++it) {
		fail::address_t start = resolve_address(it->first);
		fail::address_t end = resolve_address(it->second);

		if((start == 0) || (end == 0)) {
			m_log << std::hex << "invalid checksum range pointer" << std::endl;
			continue;
		}

		// if (start != end)
		// m_log << std::hex << "checksumming 0x" << start << " - 0x" << end << std::endl;

		for(fail::address_t addr = start; addr < end; addr++) {
			if(mm.isMapped(addr)) {
				// read byte
				uint8_t data = mm.getByte(addr);
				// add to hash
				err = SHA1Input(&sha, &data, 1);
			} else {
				err = SHA1Input(&sha, (uint8_t*) &addr, 4);
			}

			assert(err == 0);
		}
	}

	// restore paging on x86
	#ifdef BUILD_X86
	simulator.getCPU(0).setRegisterContent(reg_cr0, cr0);
	#endif

	// complete hash
	err = SHA1Result(&sha, Message_Digest);
	assert(err == 0);
}

void Checkpoint::checkpoint(const fail::ElfSymbol symbol,
                            uint32_t &value,
                            fail::simtime_t &simtime,
                            std::string &digest_str)
{
	// increment checkpoint count
	m_count++;

	// timestamp
	simtime = simulator.getTimerTicks();

	// written value
	address_t addr = symbol.getAddress();
	MemoryManager& mm = simulator.getMemoryManager();
	if(mm.isMapped(addr) && mm.isMapped(addr+1) && mm.isMapped(addr+2) && mm.isMapped(addr+3)) {
		mm.getBytes(symbol.getAddress(), symbol.getSize(), &value);
	} else {
		value = 0xDEADBEEF; // TODO: invalid value?
	}

	// checksum
	uint8_t digest[20];
	checksum(digest);

	// checksum to string
	std::stringstream s;
	s.fill('0');
	for ( size_t i = 0 ; i < 20 ; ++i )
		s << std::setw(2) << std::hex <<(unsigned short)digest[i];
	digest_str = s.str();
}

void Checkpoint::save_checkpoint(fail::address_t ip)
{
	uint32_t value;
	fail::simtime_t simtime;
	std::string digest;

	// get checkpoint info
	checkpoint(m_symbol, value, simtime, digest);

	// log checkpoint
	m_log << std::dec << "Checkpoint " << m_count << " @ " << simtime << std::endl;

	// write checkpoint
	if (m_ostream.is_open()) {
		m_ostream << std::hex << ip << "\t" << std::dec << simtime << "\t" << value << "\t" << digest << std::endl;
	} else {
		m_log << "Output error" << std::endl;
	}
}

Checkpoint::check_result Checkpoint::check(const fail::ElfSymbol symbol, fail::address_t ip)
{
	uint32_t value;
	fail::simtime_t simtime;
	std::string digest;

	address_t golden_ip;
	uint32_t golden_timestamp;
	uint32_t golden_value;
	char golden_digest_hex[41];

	assert(m_checking && "FATAL: Checkpoint plugin cannot check in tracing mode");

	// get checkpoint info
	checkpoint(symbol, value, simtime, digest);

	// check with log
	if (!m_istream.is_open()) {
		m_log << "Input file not open!" << std::endl;
		return INVALID;
	}
	if (m_istream.eof()) {
		m_log << "Checkpoint after last golden checkpoint!" << std::endl;
		return INVALID;
	}

	// read golden values
	m_istream >> std::hex >> golden_ip;
	m_istream >> std::dec >> golden_timestamp;
	m_istream >> std::dec >> golden_value;
	m_istream.width(41);
	m_istream >> golden_digest_hex;
	std::string golden_digest = golden_digest_hex;


	//bool same_timestamp = simtime == golden_timestamp);
	bool same_ip = ip == golden_ip;
	bool same_value = value == golden_value;
	bool same_digest = digest == golden_digest;

	if (!same_ip || !same_value || !same_digest) {
		// log
		m_log << "GOLDEN:" << std::hex << golden_ip << "\t" << std::dec << golden_timestamp << "\t" << golden_value << "\t" << golden_digest << std::endl;
		m_log << "TEST:  " << std::hex << ip << "\t" <<  std::dec << simtime << "\t" << value << "\t" << digest << std::endl;
	}


	if(!same_value) return DIFFERENT_VALUE;
	if(!same_ip) return DIFFERENT_IP;
	if(!same_digest) return DIFFERENT_DIGEST;

	return IDENTICAL;
}
