#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include "FESamplingPruner.hpp"
#include "util/Logger.hpp"
#include "util/CommandLine.hpp"
#include "util/SumTree.hpp"

static fail::Logger LOG("FESamplingPruner");
using std::endl;

struct Pilot {
	uint64_t duration;

	uint32_t instr2;
	union {
	uint32_t instr2_absolute;
	uint32_t id;
	};
	uint32_t data_address;

	typedef uint64_t size_type;
	size_type size() const { return duration; }
};

bool FESamplingPruner::commandline_init()
{
	fail::CommandLine &cmd = fail::CommandLine::Inst();
	SAMPLESIZE = cmd.addOption("", "samplesize", Arg::Required,
		"--samplesize N \tNumber of samples to take (per variant)");
	USE_KNOWN_RESULTS = cmd.addOption("", "use-known-results", Arg::None,
		"--use-known-results \tReuse known results from a campaign with the 'basic' pruner "
		"(abuses the DB layout to a certain degree, use with caution)");
	NO_WEIGHTING = cmd.addOption("", "no-weighting", Arg::None,
		"--no-weighting \tDisable weighted sampling (weight = 1 for all ECs) "
		"(don't do this unless you know what you're doing)");
	return true;
}

bool FESamplingPruner::prune_all()
{
	fail::CommandLine &cmd = fail::CommandLine::Inst();
	if (!cmd[SAMPLESIZE]) {
		LOG << "parameter --samplesize required, aborting" << endl;
		return false;
	}
	m_samplesize = strtoul(cmd[SAMPLESIZE].first()->arg, 0, 10);

	if (cmd[USE_KNOWN_RESULTS]) {
		m_use_known_results = true;
	}

	if (cmd[NO_WEIGHTING]) {
		m_weighting = false;
	}

	// for each variant:
	for (std::vector<fail::Database::Variant>::const_iterator it = m_variants.begin();
		it != m_variants.end(); ++it) {
		if (!sampling_prune(*it)) {
			return false;
		}
	}

	return true;
}

// TODO: replace with a less syscall-intensive RNG
static std::ifstream dev_urandom("/dev/urandom", std::ifstream::binary);
static uint64_t my_rand(uint64_t limit)
{
	// find smallest bitpos that satisfies (1 << bitpos) > limit
	int bitpos = 0;
	while (limit >> bitpos) {
		bitpos++;
	}

	uint64_t retval;

	do {
		dev_urandom.read((char *) &retval, sizeof(retval));
		retval &= (1ULL << bitpos) - 1;
	} while (retval > limit);

	return retval;
}

bool FESamplingPruner::sampling_prune(const fail::Database::Variant& variant)
{
	fail::SumTree<Pilot> pop; // sample population
	std::stringstream ss;
	MYSQL_RES *res;
	MYSQL_ROW row;

	uint64_t pilotcount = 0, samplerows;

	if (!m_use_known_results) {
		LOG << "loading trace entries for " << variant.variant << "/" << variant.benchmark << " ..." << endl;

		// load trace entries
		ss << "SELECT instr2, instr2_absolute, data_address, time2-time1+1 AS duration"
			<< " FROM trace"
			<< " WHERE variant_id = " << variant.id
			<< " AND accesstype = 'R'";
		res = db->query_stream(ss.str().c_str());
		ss.str("");
		if (!res) return false;
		while ((row = mysql_fetch_row(res))) {
			Pilot p;
			p.instr2 = strtoul(row[0], 0, 10);
			p.instr2_absolute = strtoul(row[1], 0, 10);
			p.data_address = strtoul(row[2], 0, 10);
			p.duration = m_weighting ? strtoull(row[3], 0, 10) : 1;
			pop.add(p);
			++pilotcount;
		}
		mysql_free_result(res);

		samplerows = std::min(pilotcount, m_samplesize);
	} else {
		LOG << "loading pilots for " << variant.variant << "/" << variant.benchmark << " ..." << endl;

		// load fsppilot entries
		ss << "SELECT p.id, p.instr2, p.data_address, t.time2 - t.time1 + 1 AS duration"
			<< " FROM fsppilot p"
			<< " JOIN trace t"
			<< " ON t.variant_id = p.variant_id AND t.data_address = p.data_address AND t.instr2 = p.instr2"
			<< " WHERE p.fspmethod_id = " << db->get_fspmethod_id("basic")
			<< " AND p.variant_id = " << variant.id
			<< " AND p.known_outcome = 0";
		res = db->query_stream(ss.str().c_str());
		ss.str("");
		if (!res) return false;
		while ((row = mysql_fetch_row(res))) {
			Pilot p;
			p.id = strtoul(row[0], 0, 10);
			p.instr2 = strtoul(row[1], 0, 10);
			p.data_address = strtoul(row[2], 0, 10);
			p.duration = m_weighting ? strtoull(row[3], 0, 10) : 1;
			pop.add(p);
			++pilotcount;
		}
		mysql_free_result(res);

		samplerows = std::min(pilotcount, m_samplesize);
	}

	LOG << "loaded " << pilotcount << " entries, sampling "
		<< samplerows << " entries with fault expansion ..." << endl;

	uint64_t num_fspgroup_entries = 0;
	uint32_t known_pilot_method_id = m_method_id;

	if (!m_use_known_results) {
		// FIXME: change strategy when trace entries have IDs, insert into fspgroup first
		ss << "INSERT INTO fsppilot (known_outcome, variant_id, instr2, injection_instr, "
			<< "injection_instr_absolute, data_address, data_width, fspmethod_id) VALUES ";
		std::string insert_sql(ss.str());
		ss.str("");

		for (uint64_t i = 0; i < samplerows; ++i) {
			uint64_t pos = my_rand(pop.get_size() - 1);
			Pilot p = pop.remove(pos);
			ss << "(0," << variant.id << "," << p.instr2 << "," << p.instr2
				<< "," << p.instr2_absolute << "," << p.data_address
				<< ",1," << m_method_id << ")";
			db->insert_multiple(insert_sql.c_str(), ss.str().c_str());
			ss.str("");
		}
		db->insert_multiple();

		uint64_t num_fsppilot_entries = samplerows;

		// single entry for known outcome (write access)
		ss << "INSERT INTO fsppilot (known_outcome, variant_id, instr2, injection_instr, injection_instr_absolute, data_address, data_width, fspmethod_id) "
			  "SELECT 1, variant_id, instr2, instr2, instr2_absolute, "
			  "  data_address, width, " << m_method_id << " "
			  "FROM trace "
			  "WHERE variant_id = " << variant.id << " AND accesstype = 'W' "
			  "ORDER BY instr2 ASC "
			  "LIMIT 1";
		if (!db->query(ss.str().c_str())) return false;
		ss.str("");
		num_fsppilot_entries += db->affected_rows();

		LOG << "created " << num_fsppilot_entries << " fsppilot entries" << std::endl;

		// fspgroup entries for sampled trace entries
		ss << "INSERT INTO fspgroup (variant_id, instr2, data_address, fspmethod_id, pilot_id) "
		   << "SELECT p.variant_id, p.instr2, p.data_address, p.fspmethod_id, p.id "
		   << "FROM fsppilot p "
		   << "WHERE known_outcome = 0 AND p.fspmethod_id = " << m_method_id << " "
		   << "AND p.variant_id = " << variant.id;

		if (!db->query(ss.str().c_str())) return false;
		ss.str("");
		num_fspgroup_entries = db->affected_rows();
	} else {
		ss << "INSERT INTO fspgroup (variant_id, instr2, data_address, fspmethod_id, pilot_id) VALUES ";
		std::string insert_sql(ss.str());
		ss.str("");

		for (uint64_t i = 0; i < samplerows; ++i) {
			uint64_t pos = my_rand(pop.get_size() - 1);
			Pilot p = pop.remove(pos);
			ss << "(" << variant.id << "," << p.instr2
				<< "," << p.data_address << "," << m_method_id
				<< "," << p.id << ")";
			db->insert_multiple(insert_sql.c_str(), ss.str().c_str());
			ss.str("");
		}
		db->insert_multiple();
		num_fspgroup_entries = samplerows;

		// the known_outcome=1 pilot has been determined with the "basic" method
		known_pilot_method_id = db->get_fspmethod_id("basic");
	}

#if 0 // do it like the basic pruner:
	// fspgroup entries for known (W) trace entries
	ss << "INSERT INTO fspgroup (variant_id, instr2, data_address, fspmethod_id, pilot_id) "
		"SELECT STRAIGHT_JOIN t.variant_id, t.instr2, t.data_address, p.fspmethod_id, p.id "
		"FROM fsppilot p "
		"JOIN trace t "
		"ON t.variant_id = p.variant_id AND p.fspmethod_id = " << m_method_id << " AND p.known_outcome = 1 "
		"WHERE t.variant_id = " << variant.id << " AND t.accesstype = 'W'";
#else
	// *one* fspgroup entry for known (W) trace entries (no need to create one
	// for each W); this needs to be accounted for at data analysis time,
	// though.
	ss << "INSERT INTO fspgroup (variant_id, instr2, data_address, fspmethod_id, pilot_id) "
		"SELECT variant_id, instr2, data_address, " << m_method_id << ", id "
		"FROM fsppilot "
		"WHERE variant_id = " << variant.id << " AND known_outcome = 1 AND fspmethod_id = " << known_pilot_method_id;
#endif
	if (!db->query(ss.str().c_str())) return false;
	ss.str("");
	num_fspgroup_entries += db->affected_rows();

	LOG << "created " << num_fspgroup_entries << " fspgroup entries" << std::endl;

	return true;
}
