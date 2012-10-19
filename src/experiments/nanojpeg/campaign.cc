#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "util/ProtoStream.hpp"

#include "UDIS86.hpp"

#include "../plugins/tracing/TracingPlugin.hpp"

using namespace std;
using namespace fail;

bool get_file_contents(const char *filename, string& contents)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (!in) {
		return false;
	}
	in.seekg(0, std::ios::end);
	contents.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();
	return true;
}

bool NanoJPEGCampaign::run()
{
	// read already existing results
	bool file_exists = false;
/*
	set<uint64_t> existing_results;
	ifstream oldresults(results_filename, ios::in);
	if (oldresults.is_open()) {
		char buf[16*1024];
		uint64_t addr;
		int count = 0;
		m_log << "scanning existing results ..." << endl;
		file_exists = true;
		while (oldresults.getline(buf, sizeof(buf)).good()) {
			stringstream ss;
			ss << buf;
			ss >> addr;
			if (ss.fail()) {
				continue;
			}
			++count;
			if (!existing_results.insert(addr).second) {
				m_log << "duplicate: " << addr << endl;
			}
		}
		m_log << "found " << dec << count << " existing results" << endl;
		oldresults.close();
	}
*/

	// non-destructive: due to the CSV header we can always manually recover
	// from an accident (append mode)
	ofstream results(NANOJPEG_RESULTS, ios::out | ios::app);
	if (!results.is_open()) {
		m_log << "failed to open " << NANOJPEG_RESULTS << endl;
		return false;
	}
	// only write CSV header if file didn't exist before
	if (!file_exists) {
		results << "instr_offset\tinstr_address\tregister_id\ttimeout\tinjection_ip\tbitnr\tresulttype\tlatest_ip\tpsnr\tdetails" << endl;
	}

	// load binary image (objcopy'ed system.elf = system.bin)
	string binimage;
	if (!get_file_contents(NANOJPEG_BIN, binimage)) {
		m_log << "couldn't open " << NANOJPEG_BIN << endl;
		return false;
	}
	Udis86 udis(0);

	// load trace
	ifstream tracef(NANOJPEG_TRACE);
	if (tracef.fail()) {
		m_log << "couldn't open " << NANOJPEG_TRACE << endl;
		return false;
	}
	ProtoIStream ps(&tracef);

	// experiment count
	int count = 0;

	// instruction counter within trace
	int instr = 0;

	Trace_Event ev;
	// for every event in the trace ...
	while (ps.getNext(&ev) && instr < NANOJPEG_INSTR_LIMIT) {
		// sanity check: skip memory access entries
		if (ev.has_memaddr()) {
			continue;
		}
		instr++;

		// disassemble instruction
		if (ev.ip() < NANOJPEG_BIN_OFFSET || ev.ip() - NANOJPEG_BIN_OFFSET > binimage.size()) {
			m_log << "traced IP 0x" << hex << ev.ip() << " outside system image" << endl;
			continue;
		}
		udis.setIP(ev.ip());
		size_t ip_offset = ev.ip() - NANOJPEG_BIN_OFFSET;
		udis.setInputBuffer((unsigned char *) &binimage[ip_offset], min((size_t) 20, binimage.size() - ip_offset));

		if (!udis.fetchNextInstruction()) {
			m_log << "fatal: cannot disassemble instruction at 0x" << hex << ev.ip() << endl;
			return false;
		}

		ud_t ud = udis.getCurrentState();

		// for now: debug output
		m_log << "0x" << hex << ev.ip() << " " << ::ud_insn_asm(&ud) << endl;
		for (int i = 0; i < 3; ++i) {
			switch (ud.operand[i].type) {
			case UD_NONE: cout << "-"; break;
			case UD_OP_MEM: cout << "M"; break;
			case UD_OP_PTR: cout << "P"; break;
			case UD_OP_IMM: cout << "I"; break;
			case UD_OP_JIMM: cout << "J"; break;
			case UD_OP_CONST: cout << "C"; break;
			case UD_OP_REG: cout << "R"; break;
			default: m_log << "WAT" << endl;
			}
			std::cout << " ";
		}
		cout << endl;
	}

	return true;
}
