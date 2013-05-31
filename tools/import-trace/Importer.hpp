#ifndef __IMPORTER_H__
#define __IMPORTER_H__

#include <mysql/mysql.h>
#include <map>
#include "util/ProtoStream.hpp"
#include "util/ElfReader.hpp"
#include "sal/SALConfig.hpp"
#include "util/Database.hpp"
#include "util/MemoryMap.hpp"
#include "comm/TracePlugin.pb.h"

class Importer {
public:
	typedef unsigned instruction_count_t; //!< not big enough for some benchmarks
	struct margin_info_t { instruction_count_t dyninstr; fail::guest_address_t ip; fail::simtime_t time; };
	struct access_info_t { char access_type; fail::guest_address_t data_address; int data_width; };

protected:
	int m_variant_id;
	fail::ElfReader *m_elf;
	fail::MemoryMap *m_mm;
	char m_faultspace_rightmargin;
	bool m_sanitychecks;
	fail::Database *db;

	/* How many rows were inserted into the database */
	unsigned m_row_count;

	// time the trace started/ended
	fail::simtime_t m_time_trace_start;

	/* map for keeping one "open" EC for every address
	   (maps injection data address =>
	   dyn. instruction count / time information for equivalence class
	   left margin) */
	typedef std::map<fail::address_t, margin_info_t> AddrLastaccessMap;
	AddrLastaccessMap m_open_ecs;

	margin_info_t getOpenEC(fail::address_t data_address) {
		margin_info_t ec = m_open_ecs[data_address];
		// defaulting to 0 is not such a good idea, memory reads at the
		// beginning of the trace would get an unnaturally high weight:
		if (ec.time == 0) {
			ec.time = m_time_trace_start;
		}
		return ec;
	}

	void newOpenEC(fail::address_t data_address, fail::simtime_t time, instruction_count_t dyninstr,
				   fail::guest_address_t ip) {
		m_open_ecs[data_address].dyninstr = dyninstr;
		m_open_ecs[data_address].time     = time;
		// FIXME: This should be the next IP, not the current, or?
		m_open_ecs[data_address].ip       = ip;
	}


	fail::guest_address_t m_last_ip;
	instruction_count_t m_last_instr;
	fail::simtime_t m_last_time;

public:
	Importer() : m_sanitychecks(false), m_row_count(0), m_time_trace_start(0) {}
	bool init(const std::string &variant, const std::string &benchmark, fail::Database *db);

	virtual bool create_database();
	virtual bool copy_to_database(fail::ProtoIStream &ps);
	virtual bool clear_database();
	virtual bool add_trace_event(margin_info_t &begin, margin_info_t &end,
								 access_info_t &event, bool is_fake = false);
	virtual void open_unused_ec_intervals();
	virtual bool close_ec_intervals();

	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
								 const Trace_Event &ev) = 0;
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
								 const Trace_Event &ev) = 0;


	void set_elf(fail::ElfReader *elf) { m_elf = elf; }

	void set_memorymap(fail::MemoryMap *mm) { m_mm = mm; }
	void set_faultspace_rightmargin(char accesstype) { m_faultspace_rightmargin = accesstype; }
	void set_sanitychecks(bool enabled) { m_sanitychecks = enabled; }



};

#endif
