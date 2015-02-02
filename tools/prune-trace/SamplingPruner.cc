#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include "SamplingPruner.hpp"
#include "util/Logger.hpp"
#include "util/CommandLine.hpp"
#include "util/SumTree.hpp"

static fail::Logger LOG("SamplingPruner");
using std::endl;

struct WeightedPilot {
	uint64_t duration;

	uint32_t id;
	uint32_t instr2;
	uint32_t instr2_absolute;
	uint32_t data_address;
	uint32_t weight;

	typedef uint64_t size_type;
	size_type size() const { return duration; }
};

bool SamplingPruner::commandline_init()
{
	fail::CommandLine &cmd = fail::CommandLine::Inst();
	SAMPLESIZE = cmd.addOption("", "samplesize", Arg::Required,
		"--samplesize N \tNumber of samples to take (per variant)");
	USE_KNOWN_RESULTS = cmd.addOption("", "use-known-results", Arg::None,
		"--use-known-results \tReuse known results from a campaign with the 'basic' pruner ");
	NO_WEIGHTING = cmd.addOption("", "no-weighting", Arg::None,
		"--no-weighting \tDisable weighted sampling (weight = 1 for all ECs) "
		"(don't do this unless you know what you're doing)");
	return true;
}

bool SamplingPruner::prune_all()
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
// TODO: deduplicate (copied from FESamplingPruner), put in a central place
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

bool SamplingPruner::sampling_prune(const fail::Database::Variant& variant)
{
	typedef fail::SumTree<WeightedPilot> sumtree_type;
	sumtree_type pop; // sample population
	std::stringstream ss;
	MYSQL_RES *res;
	MYSQL_ROW row;

	uint64_t pilotcount = 0;

	if (!m_use_known_results) {
		LOG << "loading trace entries "
			<< (m_incremental ? "and existing pilots " : "")
			<< "for " << variant.variant << "/" << variant.benchmark << " ..." << endl;

		if (!m_incremental) {
			// load trace entries
			ss << "SELECT instr2, instr2_absolute, data_address, time2-time1+1 AS duration"
				" FROM trace"
				" WHERE variant_id = " << variant.id <<
				" AND accesstype = 'R'";
		} else {
			// load trace entries and existing pilots
			ss << "SELECT t.instr2, t.instr2_absolute, t.data_address, t.time2-t.time1+1 AS duration,"
				" IFNULL(g.pilot_id, 0), IFNULL(g.weight, 0)"
				" FROM trace t"
				" LEFT JOIN fspgroup g"
				" ON t.variant_id = g.variant_id AND t.data_address = g.data_address AND t.instr2 = g.instr2"
				" AND g.fspmethod_id = " << m_method_id <<
				" WHERE t.variant_id = " << variant.id <<
				" AND t.accesstype = 'R'";
		}
		res = db->query_stream(ss.str().c_str());
		ss.str("");
		if (!res) return false;
		while ((row = mysql_fetch_row(res))) {
			WeightedPilot p;
			p.instr2 = strtoul(row[0], 0, 10);
			p.instr2_absolute = strtoul(row[1], 0, 10);
			p.data_address = strtoul(row[2], 0, 10);
			p.duration = m_weighting ? strtoull(row[3], 0, 10) : 1;
			p.id = m_incremental ? strtoul(row[4], 0, 10) : 0;
			p.weight = m_incremental ? strtoul(row[5], 0, 10) : 0;
			pop.add(p);
			++pilotcount;
		}
		mysql_free_result(res);
	} else {
		LOG << "loading pilots for " << variant.variant << "/" << variant.benchmark << " ..." << endl;

		if (!m_incremental) {
			// load fsppilot entries
			ss << "SELECT p.id, p.instr2, p.data_address, t.time2 - t.time1 + 1 AS duration"
				" FROM fsppilot p"
				" JOIN trace t"
				" ON t.variant_id = p.variant_id AND t.data_address = p.data_address AND t.instr2 = p.instr2"
				" WHERE p.fspmethod_id = " << db->get_fspmethod_id("basic") <<
				" AND p.variant_id = " << variant.id <<
				" AND p.known_outcome = 0";
		} else {
			// load fsppilot entries and existing sampling pilots
			ss << "SELECT p.id, p.instr2, p.data_address, t.time2 - t.time1 + 1 AS duration, IFNULL(g.weight, 0)"
				" FROM fsppilot p"
				" JOIN trace t"
				" ON t.variant_id = p.variant_id AND t.data_address = p.data_address AND t.instr2 = p.instr2"
				" LEFT JOIN fspgroup g"
				" ON t.variant_id = g.variant_id AND t.data_address = g.data_address AND t.instr2 = g.instr2"
				" AND g.fspmethod_id = " << m_method_id <<
				" WHERE p.fspmethod_id = " << db->get_fspmethod_id("basic") <<
				" AND p.variant_id = " << variant.id <<
				" AND p.known_outcome = 0";
		}
		res = db->query_stream(ss.str().c_str());
		ss.str("");
		if (!res) return false;
		while ((row = mysql_fetch_row(res))) {
			WeightedPilot p;
			p.id = strtoul(row[0], 0, 10);
			p.instr2 = strtoul(row[1], 0, 10);
			p.data_address = strtoul(row[2], 0, 10);
			p.duration = m_weighting ? strtoull(row[3], 0, 10) : 1;
			p.weight = m_incremental ? strtoull(row[4], 0, 10) : 0;
			pop.add(p);
			++pilotcount;
		}
		mysql_free_result(res);
	}

	LOG << "loaded " << pilotcount << " entries, sampling "
		<< m_samplesize << " fault-space coordinates ..." << endl;

	ss << "INSERT INTO fsppilot (known_outcome, variant_id, instr2, injection_instr, "
		<< "injection_instr_absolute, data_address, data_width, fspmethod_id) VALUES ";
	std::string insert_sql(ss.str());
	ss.str("");

	uint64_t popsize = pop.get_size(); // stays constant
	uint64_t num_fsppilot_entries = 0;
	for (uint64_t i = 0; i < m_samplesize; ++i) {
		uint64_t pos = my_rand(popsize - 1);
		WeightedPilot& p = pop.get(pos);
		p.weight++;
		// first time we sample this pilot?
		if (!m_use_known_results && p.weight == 1) {
			// no need to special-case existing pilots (incremental mode), as
			// their initial weight is supposed to be at least 1
			ss << "(0," << variant.id << "," << p.instr2 << "," << p.instr2
				<< "," << p.instr2_absolute << "," << p.data_address
				<< ",1," << m_method_id << ")";
			db->insert_multiple(insert_sql.c_str(), ss.str().c_str());
			ss.str("");
			++num_fsppilot_entries;
		}
	}

	if (!m_use_known_results) {
		db->insert_multiple();
		LOG << "created " << num_fsppilot_entries << " fsppilot entries" << std::endl;
	}

	// fspgroup entries for sampled trace entries
	if (!m_use_known_results) {
		if (!m_incremental) {
			ss << "INSERT";
		} else {
			// this spares us to delete existing pilots before
			ss << "REPLACE";
		}
		ss << " INTO fspgroup (variant_id, instr2, data_address, fspmethod_id, pilot_id, weight) "
		   << "SELECT p.variant_id, p.instr2, p.data_address, " << m_method_id << ", p.id, 1 "
		   << "FROM fsppilot p "
		   << "WHERE known_outcome = 0 AND p.fspmethod_id = " << m_method_id << " "
		   << "AND p.variant_id = " << variant.id;

		if (!db->query(ss.str().c_str())) return false;
		ss.str("");
		uint64_t num_fspgroup_entries;
		if (!m_incremental) {
			num_fspgroup_entries = db->affected_rows();
		} else {
			// with REPLACE INTO, affected_rows does not yield the number of
			// new rows; take num_fsppilot_entries instead
			num_fspgroup_entries = num_fsppilot_entries;
		}
		LOG << "created " << num_fspgroup_entries << " fspgroup entries" << std::endl;

		// FIXME is this faster than manually INSERTing all fspgroup entries?
		num_fspgroup_entries = 0;
		LOG << "updating fspgroup entries with weight > 1 ..." << std::endl;
		for (sumtree_type::iterator it = pop.begin(); it != pop.end(); ++it) {
			if (it->weight <= 1) {
				continue;
			}
			++num_fspgroup_entries;
			ss << "UPDATE fspgroup SET weight = " << it->weight <<
				" WHERE variant_id = " << variant.id <<
				" AND instr2 = " << it->instr2 <<
				" AND data_address = " << it->data_address <<
				" AND fspmethod_id = " << m_method_id;
			// pilot_id is known but should be identical
			if (!db->query(ss.str().c_str())) return false;
			if (db->affected_rows() != 1) {
				LOG << "something is wrong, query affected unexpected ("
					<< db->affected_rows()
					<< " != 1) number of rows: "
					<< ss.str() << std::endl;
			}
			ss.str("");
		}

		if (!m_incremental) {
			LOG << "updated " << num_fspgroup_entries << " fspgroup entries" << std::endl;
		} else {
			// we don't know how many rows we really updated
			LOG << "updated fspgroup entries" << std::endl;
		}
	} else {
		uint64_t num_fspgroup_entries = 0;

		LOG << "creating fspgroup entries ..." << std::endl;

		if (!m_incremental) {
			ss << "INSERT";
		} else {
			// this spares us to delete existing pilots before
			ss << "REPLACE";
		}
		ss << " INTO fspgroup (variant_id, instr2, data_address, fspmethod_id, pilot_id, weight) VALUES ";
		insert_sql = ss.str();
		ss.str("");

		for (sumtree_type::iterator it = pop.begin(); it != pop.end(); ++it) {
			if (it->weight == 0) {
				continue;
			}
			++num_fspgroup_entries;
			ss << "(" << variant.id << "," << it->instr2 << "," << it->data_address
				<< "," << m_method_id << "," << it->id << "," << it->weight << ")";
			db->insert_multiple(insert_sql.c_str(), ss.str().c_str());
			ss.str("");
		}
		db->insert_multiple();
		LOG << "created " << num_fspgroup_entries << " fspgroup entries" << std::endl;
	}

	return true;
}
