#ifndef __IMPORTER_H__
#define __IMPORTER_H__

#include <mysql.h>
#include <map>
#include <sstream>
#include "util/ProtoStream.hpp"
#include "util/ElfReader.hpp"
#include "sal/SALConfig.hpp"
#include "sal/Architecture.hpp"
#include "util/Database.hpp"
#include "util/MemoryMap.hpp"
#include "comm/TracePlugin.pb.h"
#include "util/AliasedRegisterable.hpp"

class Importer : public fail::AliasedRegisterable {
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
	bool m_import_write_ecs;
	bool m_extended_trace;
	fail::Database *db;
	fail::Architecture m_arch;
	fail::UniformRegisterSet *m_extended_trace_regs;

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

protected:
	/**
	 * Allows specialized importers to add more table columns instead of
	 * completely overriding create_database().  The returned SQL CREATE TABLE
	 * snippet should be terminated with a comma if non-empty.  Should call and
	 * pass through their parent's implementation.
	 */
	virtual std::string database_additional_columns() { return ""; }
	/**
	 * Similar to database_additional_columns(), this allows specialized
	 * importers to define which additional columns it wants to INSERT
	 * alongside what add_trace_event() adds by itself.  This may be identical
	 * to or a subset of what database_additional_columns() specifies.  The SQL
	 * snippet should *begin* with a comma if non-empty.
	 */
	virtual void database_insert_columns(std::string& sql, unsigned& num_columns) { num_columns = 0; }
	/**
	 * Will be called back from add_trace_event() to fill in data for the
	 * columns specified by database_insert_columns().
	 */
	virtual bool database_insert_data(Trace_Event &ev, std::stringstream& value_sql, unsigned num_columns, bool is_fake) { return true; }
	/**
	 * Use this variant if passing through the IP/MEM event does not make any
	 * sense for your Importer implementation.
	 */
	virtual bool add_trace_event(margin_info_t &begin, margin_info_t &end,
								 access_info_t &event, bool is_fake = false);
	/**
	 * Use this variant for passing through the IP/MEM event your Importer
	 * received.
	 */
	virtual bool add_trace_event(margin_info_t &begin, margin_info_t &end,
								 Trace_Event &event, bool is_fake = false);
	virtual void open_unused_ec_intervals();
	virtual bool close_ec_intervals();

	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
								 Trace_Event &ev) = 0;
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
								 Trace_Event &ev) = 0;
	/**
	 * May be overridden by importers that need to do stuff after the last
	 * event was consumed.
	 */
	virtual bool trace_end_reached() { return true; }

	/**
	 * Executes an SQL statement, assumes a sanity check failure if it returns
	 * any result rows, and provides some diagnostics.
	 */
	bool sanitycheck(std::string check_name, std::string fail_msg, std::string sql);
public:
	Importer() : m_variant_id(0), m_elf(NULL), m_mm(NULL), m_faultspace_rightmargin('W'),
		m_sanitychecks(false), m_import_write_ecs(true), m_extended_trace(false), db(NULL),
		m_extended_trace_regs(NULL), m_row_count(0), m_time_trace_start(0),
		m_last_ip(0), m_last_instr(0), m_last_time(0) {}
	bool init(const std::string &variant, const std::string &benchmark, fail::Database *db);

	/**
	 * Callback function that can be used to add command line options
	 * to the cmd interface
	 */
	virtual bool cb_commandline_init() { return true; }

	virtual bool create_database();
	virtual bool copy_to_database(fail::ProtoIStream &ps);
	virtual bool clear_database();

	void set_elf(fail::ElfReader *elf) { m_elf = elf; }

	void set_memorymap(fail::MemoryMap *mm) { m_mm = mm; }
	void set_faultspace_rightmargin(char accesstype) { m_faultspace_rightmargin = accesstype; }
	void set_sanitychecks(bool enabled) { m_sanitychecks = enabled; }
	void set_import_write_ecs(bool enabled) { m_import_write_ecs = enabled; }
	void set_extended_trace(bool enabled) { m_extended_trace = enabled; }
};

#endif
