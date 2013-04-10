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
protected:
	int m_variant_id;
	fail::ElfReader *m_elf;
	fail::MemoryMap *m_mm;
	char m_faultspace_rightmargin;
	bool m_sanitychecks;
	fail::Database *db;

public:
	typedef unsigned instruction_count_t; //!< not big enough for some benchmarks
	struct margin_info_t { instruction_count_t dyninstr; fail::guest_address_t ip; fail::simtime_t time; };

	Importer() : m_sanitychecks(false) {}
	bool init(const std::string &variant, const std::string &benchmark, fail::Database *db);

	virtual bool create_database() = 0;
	virtual bool copy_to_database(fail::ProtoIStream &ps);
	virtual bool clear_database();
	virtual bool add_trace_event(margin_info_t &begin, margin_info_t &end,
								 const Trace_Event &event, bool is_fake = false) = 0;

	void set_elf_file(fail::ElfReader *elf) { m_elf = elf; }
	void set_memorymap(fail::MemoryMap *mm) { m_mm = mm; }
	void set_faultspace_rightmargin(char accesstype) { m_faultspace_rightmargin = accesstype; }
	void set_sanitychecks(bool enabled) { m_sanitychecks = enabled; }
protected:
private:

	typedef std::map<fail::address_t, margin_info_t> AddrLastaccessMap;
};

#endif
