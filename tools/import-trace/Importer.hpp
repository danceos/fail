#ifndef __IMPORTER_H__
#define __IMPORTER_H__

#include <mysql.h>
#include <map>
#include <sstream>
#include <vector>
#include "util/ProtoStream.hpp"
#include "util/ElfReader.hpp"
#include "sal/SALConfig.hpp"
#include "sal/Architecture.hpp"
#include "sal/FaultSpace.hpp"
#include "util/Database.hpp"
#include "util/MemoryMap.hpp"
#include "comm/TracePlugin.pb.h"
#include "util/AliasedRegisterable.hpp"

class Importer : public fail::AliasedRegisterable {
public:
	typedef unsigned instruction_count_t; //!< not big enough for some benchmarks
    enum class access_t {
        READ, WRITE
    };
	struct margin_info_t {
        instruction_count_t dyninstr;
        fail::guest_address_t ip;
        fail::simtime_t time;
        uint8_t mask;
        bool synthetic;
        margin_info_t(instruction_count_t dyninstr, fail::guest_address_t ip, fail::simtime_t time, uint8_t mask, bool synthetic = false):
            dyninstr(dyninstr), ip(ip), time(time), mask(mask), synthetic(synthetic) { }
        margin_info_t(const margin_info_t& other, uint8_t mask, bool synthetic = false):
            dyninstr(other.dyninstr), ip(other.ip), time(other.time), mask(mask), synthetic(other.synthetic) { }
    };

protected:
	int m_variant_id;
	fail::ElfReader *m_elf;
	fail::MemoryMap *m_mm;
    access_t m_faultspace_rightmargin;
	bool m_sanitychecks;
	bool m_import_write_ecs;
	bool m_extended_trace;
	fail::Database *db;
	fail::Architecture m_arch;
    fail::FaultSpace m_fsp;
	fail::UniformRegisterSet *m_extended_trace_regs;
    memory_type_t m_memtype;

	/* How many rows were inserted into the database */
	unsigned m_row_count;

	// time the trace started/ended
	fail::simtime_t m_time_trace_start;

	/* map for keeping one "open" EC for every address
	   (maps injection data address =>
	   dyn. instruction count / time information for equivalence class
	   left margin) */
    using element = fail::util::fsp::element;

	typedef std::map<element, std::vector<margin_info_t>> AddrLastaccessMap;
	AddrLastaccessMap m_open_ecs;

    std::vector<margin_info_t> getLeftMargins(const element& fsp_elem,
            fail::simtime_t time, instruction_count_t dyninstr,
            fail::guest_address_t ip, uint8_t mask);
	void addLeftMargin(const element& fsp_elem,
                   fail::simtime_t time, instruction_count_t dyninstr,
				   fail::guest_address_t ip,
                   uint8_t mask);


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
     * To be called from your importer implementation to add fault space elements to the database.
     * This function should be used over using the legacy add_trace_event functions directly.
     */
    virtual bool add_faultspace_elements(fail::simtime_t curtime, instruction_count_t instr,
                                          std::vector<std::unique_ptr<fail::util::fsp::element>> elements,
                                          uint8_t mask,
                                          access_t type,
                                          Trace_Event& origin
                                          );

    /**
     * Add a singular fault space element with the given margins to the database.
     * This is usually called from add_fault_space_elements, but can optionally be called
     * directly to avoid automatic margin generation.
     */
	virtual bool add_faultspace_event(margin_info_t &begin, margin_info_t &end,
								 std::unique_ptr<fail::util::fsp::element> elem,
                                 access_t type,
                                 Trace_Event& origin,
                                 bool known_outcome = false
                                 );
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
	Importer() : m_variant_id(0), m_elf(NULL), m_mm(NULL), m_faultspace_rightmargin(),
		m_sanitychecks(false), m_import_write_ecs(true), m_extended_trace(false), db(NULL),
		m_extended_trace_regs(NULL), m_memtype(fail::ANY_MEMORY), m_row_count(0), m_time_trace_start(0),
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
	void set_faultspace_rightmargin(access_t accesstype) { m_faultspace_rightmargin = accesstype; }
	void set_sanitychecks(bool enabled) { m_sanitychecks = enabled; }
	void set_import_write_ecs(bool enabled) { m_import_write_ecs = enabled; }
	void set_extended_trace(bool enabled) { m_extended_trace = enabled; }
    void set_memory_type(memory_type_t type) { m_memtype = type; }
};

#endif
