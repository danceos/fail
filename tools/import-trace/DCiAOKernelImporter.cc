#include <iostream>
#include <set>

#include "util/Logger.hpp"
static fail::Logger LOG("DCiAOKernelImporter");

using namespace fail;

#include "DCiAOKernelImporter.hpp"

bool DCiAOKernelImporter::inDynamicKernelMemory(fail::address_t addr) {
	const std::string &name = m_elf->getSymbol(addr).getDemangledName();
	bool dynamic = name.find("os::data::dynamic", 0) != std::string::npos;
	//	bool stack   = name.find("_stack") != std::string::npos;
	// return dynamic && !stack;
	return dynamic;
}

bool DCiAOKernelImporter::copy_to_database(fail::ProtoIStream &ps) {
	if (m_elf == 0) {
		LOG << "Please give an ELF Binary as a parameter" << std::endl;
		exit(-1);
	}

	if (getEnterKernelAddress() == 0 || getLeaveKernelAddress() == 0) {
		LOG << "Pleave give a valid CiAO Binary with kernel dependability options enabled" << std::endl;
		exit(-1);
	}

	unsigned row_count = 0;

	// instruction counter within trace
	unsigned instr = 0;
	unsigned instr_last_kernel_leave = 0;

	address_t enter_kernel_addr = getEnterKernelAddress();
	address_t leave_kernel_addr = getLeaveKernelAddress();


	Trace_Event ev;

	// Collect all memory addresses that
	std::set<address_t> already_written_addresses;

	bool in_kernel_space = false;

	while (ps.getNext(&ev)) {
		// instruction events just get counted
		if (!ev.has_memaddr()) {
			// new instruction
			instr++;

			if (ev.ip() == enter_kernel_addr) {
				in_kernel_space = true;
			}

			if (ev.ip() == leave_kernel_addr) {
				instr_last_kernel_leave = instr;
				in_kernel_space = false;
				already_written_addresses.clear();
			}
			continue;
		}

		if (in_kernel_space && inDynamicKernelMemory(ev.memaddr())) {
			if (ev.accesstype() == ev.WRITE) {
				/* If a address is written in the protected kernel
				   space, we ignore it for further injections */
				address_t from = ev.memaddr(), to = ev.memaddr() + ev.width();
				// Iterate over all accessed bytes
				for (address_t data_address = from; data_address < to; ++data_address) {
					already_written_addresses.insert(data_address);
				}
			} else {
				/* Read address was not written in this kernel section
				   -> Insert an trace event */
				address_t from = ev.memaddr(), to = ev.memaddr() + ev.width();
				// Iterate over all accessed bytes
				for (address_t data_address = from; data_address < to; ++data_address) {
					int instr1 = instr_last_kernel_leave;
					int instr2 = instr; // the current instruction

					/* Was the byte already written in this kernel
					   space */
					if (already_written_addresses.find(data_address)
							== already_written_addresses.end())
						continue;

					ev.set_memaddr(data_address);
					ev.set_width(1);

					// we now have an interval-terminating R/W event to the memaddr
					// we're currently looking at; the EC is defined by
					// data_address [last_kernel_leave, read_instr] (instr_absolute)
					if (!add_trace_event(instr1, instr2, ev)) {
						LOG << "add_trace_event failed" << std::endl;
						return false;
					}
					row_count ++;
					if (row_count % 1000 == 0) {
						LOG << "Imported " << row_count << " traces into the database" << std::endl;
					}
				}
			}
		}

	}

	LOG << "Inserted " << row_count << " traces into the database" << std::endl;

	return true;
}
