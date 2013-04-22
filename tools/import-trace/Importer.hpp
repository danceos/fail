#ifndef __IMPORTER_H__
#define __IMPORTER_H__

#include <mysql/mysql.h>
#include <map>
#include "util/ProtoStream.hpp"
#include "util/ElfReader.hpp"
#include "sal/SALConfig.hpp"
#include "util/Database.hpp"
#include "comm/TracePlugin.pb.h"


class Importer {
protected:
	int m_variant_id;
	fail::ElfReader *m_elf;
	fail::Database *db;

public:
	typedef unsigned instruction_count_t;

	bool init(const std::string &variant, const std::string &benchmark, fail::Database *db);

	virtual bool create_database() = 0;
	virtual bool copy_to_database(fail::ProtoIStream &ps);
	virtual bool clear_database();
	virtual bool add_trace_event(instruction_count_t begin, instruction_count_t end,
								 const Trace_Event &event, bool is_fake = false) = 0;

	void set_elf_file(fail::ElfReader *elf) { m_elf = elf; }
protected:
private:
	typedef std::map<fail::address_t, instruction_count_t> AddrLastaccessMap;
};

#endif
